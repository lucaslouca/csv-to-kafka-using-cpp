#include "CsvProcessor.h"
#include "logging/Logging.h"
#include "CsvProcessorBuilder.h"
#include "csv/CSVRange.h"
#include "impl/SchemaRegistry.h"
#include "Util.h"
#include <thread>
#include <iostream>
#include <fstream>
#include <stdio.h>     // for rename()
#include <arpa/inet.h> // for htonl()
#include <avro/Schema.hh>
#include <avro/Specific.hh>
#include <avro/Encoder.hh>
#include <avro/Decoder.hh>
#include <avro/Compiler.hh>
#include <signal.h>
#include <exception>
#include <stdexcept>
#include <typeinfo>
#include <time.h>

CsvProcessor::CsvProcessor(std::string name, std::shared_ptr<SignalChannel> sig_channel) : AbstractProcessor(name, sig_channel)
{
}

ssize_t CsvProcessor::serialize(avro::ValidSchema schema, const int32_t schema_id, const avro::GenericDatum datum, std::vector<char> &out, std::string &errstr)
{
  auto output_stream = avro::memoryOutputStream();
  auto encoder = avro::validatingEncoder(schema, avro::binaryEncoder());

  try
  {
    // Encode Avro datum to Avro binary format
    encoder->init(*output_stream.get());
    avro::encode(*encoder, datum);
    encoder->flush();
  }
  catch (const avro::Exception &e)
  {
    errstr = e.what();
    return -1;
  }

  // Extract written bytes
  auto encoded = avro::snapshot(*output_stream.get());

  // Write framing [<magic byte> <schema id> <bytesavro>]
  ssize_t framing_size = 5; // 1 for magic byte + 4 for schema id

  int pos = out.size();
  out.resize(out.size() + framing_size);

  char *payload = &out[pos];

  // Magic byte
  payload[0] = 0;

  // Schema ID
  int32_t id = htonl(schema_id);
  memcpy(payload + 1, &id, 4);

  // Append binary encoded Avro to output vector
  out.insert(out.end(), encoded->cbegin(), encoded->cend());

  return out.size();
}

void CsvProcessor::publish(CSVRow &row, size_t &old_count)
{
  // Create Avro record
  std::vector<char> out_data;
  for (auto &[topic, schema_config] : *m_schemas)
  {
    const avro::ValidSchema &schema = schema_config.schema;
    avro::GenericDatum datum(schema);
    avro::GenericRecord &record = datum.value<avro::GenericRecord>();
    for (const auto &field : schema_config.columns)
    {
      std::string field_name = field;

      auto name_it = schema_config.column_map.find(field);
      if (name_it != schema_config.column_map.end())
      {
        field_name.assign(name_it->second);
      }

      /* Check max age now that we got the new field name */
      if (m_max_age_config)
      {
        if (!field_name.compare(m_max_age_config->first))
        {
          long event_timestamp = stol(row[field]);
          time_t now = time(NULL);
          int days_since_event = (now - event_timestamp) / (60 * 60 * 24);

          if (days_since_event > m_max_age_config->second)
          {
            ++old_count;
            return;
          }
        }
      }

      std::string type = "string";
      auto type_it = schema_config.column_type_transforms.find(field);
      if (type_it != schema_config.column_type_transforms.end())
      {
        type.assign(type_it->second);
      }

      record.setFieldAt(record.fieldIndex(field_name), Util::create_datum_for_type(row[field], type));
    }

    // Register schema
    if (schema_config.schema_id == -1)
    {
      schema_config.schema_id = SchemaRegistry::instance().register_value_schema(topic, schema_config.schema.toJson());
    }

    std::string errstr;
    if (serialize(schema, schema_config.schema_id, datum, out_data, errstr) == -1)
    {
      Logging::ERROR("Avro serialization failed: " + errstr, m_name);
      kill(getpid(), SIGINT);
    }
    else
    {
    retry:
      RdKafka::ErrorCode err = m_kafka_producer->produce(topic,
                                                         RdKafka::Topic::PARTITION_UA,
                                                         RdKafka::Producer::RK_MSG_COPY,
                                                         /* Value */
                                                         out_data.data(),
                                                         out_data.size(),
                                                         /* Key */
                                                         row[schema_config.key_column].c_str(),
                                                         /* Key len */
                                                         row[schema_config.key_column].length(),
                                                         /* Timestamp (defaults to current time) */
                                                         0,
                                                         /* Message headers, if any */
                                                         NULL,
                                                         /* Per-message opaque value passed to
                                                          * delivery report */
                                                         NULL);
      if (err != RdKafka::ERR_NO_ERROR)
      {
        Logging::ERROR("Failed to produce to topic '" + topic + "': " + RdKafka::err2str(err), m_name);

        if (err == RdKafka::ERR__QUEUE_FULL)
        {
          /* If the internal queue is full, wait for
           * messages to be delivered and then retry.
           * The internal queue represents both
           * messages to be sent and messages that have
           * been sent or failed, awaiting their
           * delivery report callback to be called.
           *
           * The internal queue is limited by the
           * configuration property
           * queue.buffering.max.messages and queue.buffering.max.kbytes */
          m_kafka_producer->poll(1000 /*block for max 1000ms*/);
          goto retry;
        }
      }
      else
      {
        Logging::DEBUG("Enqueued message (" + std::to_string(out_data.size()) + " bytes) for topic '" + topic + "'", m_name);
      }

      /* Wait for final messages to be delivered or fail.
       * flush() is an abstraction over poll() which
       * waits for all messages to be delivered. */
      Logging::DEBUG("Flushing final messages...", m_name);
      m_kafka_producer->flush(10 * 1000 /* wait for max 10 seconds */);

      if (m_kafka_producer->outq_len() > 0)
      {
        Logging::ERROR(std::to_string(m_kafka_producer->outq_len()) + " message(s) were not delivered", m_name);
      }
    }
  }
}

void CsvProcessor::handle(PollResult d)
{
  /*
  Atomic since we are modifying it from multiple processors and we want it to be be threadsafe.

  Although atomic and threadsafe when multiple CsvProcessors are modifying it is better to lock in case:
  1. A LogProcessor was waken up (maybe by a different CsvProcessor) and is ready to check the condition to see if it is allowed to log.
  It expects the condition to be true. At this point the LogProcessor is not in a waiting state.
  2. Just before it manages to reacquire the mutex and check, another CsvProcessor is scheduled and wants to parse a CSV file.
  Because we want the CsvProcessors to have higher priority than logging, we quickly lock the mutex to prevent the
  LogProcessor from doing its work. Even if it is just checking the condition.

  But then again... if we are locking anyways we don't really need atomic.
  */
  {
    std::unique_lock lock(*m_log_cv_mutex);
    m_active_processors->fetch_add(1);
  }

  size_t old_count = 0;

  std::stringstream ss;
  ss << "Processing '" << d.get() << "'";
  Logging::INFO(ss.str(), m_name);

  std::string tmp_file_path = d.get() + "_inprogress";
  if (rename(d.get().c_str(), tmp_file_path.c_str()) == 0)
  {
    std::ifstream file(tmp_file_path);

    short exc_count = 0;
    try
    {
      for (auto &row : CSVRange(file, true))
      {

        // Proceed with transformations
        try
        {
          for (const auto &transformer_ptr : *m_transformers)
          {
            transformer_ptr->Operation(row);
          }
          publish(row, old_count);
        }
        catch (...)
        {
          std::exception_ptr e = std::current_exception();
          ss.str("Error in row: ");
          ss << row;
          if (exc_count == 0)
          {
            Logging::ERROR(ss.str(), m_name);
          }
          Logging::ERROR(e ? Util::what(e) : "null", m_name);
          if (++exc_count > 13)
          {
            Logging::ERROR("To many exceptions in file '" + d.get() + "'", m_name);
            break;
          }
        }
      }
    }
    catch (...)
    {
      Logging::ERROR("Unable to load file '" + d.get() + "'", m_name);
    }

    rename(tmp_file_path.c_str(), std::string(d.get() + "_done").c_str());
  }

  ss.str("");
  ss << "Done with '"
     << d.get()
     << "'";

  if (old_count > 0)
  {
    ss << ". Ignored " << old_count << " events because they were older than " << m_max_age_config->second << " days";
  }
  Logging::INFO(ss.str(), m_name);

  /*
  Why do we protect writes to shared var even if it is atomic?
  There could be problems if write to shared variable happens between checking it in predicate and waiting on condition. Consider the following:
  1. Waiting thread (e.g. LogProcessor) wakes up spuriously, aquires mutex, checks predicate and evaluates it to false, so it must wait in cv again.
  2. Controlling thread (e.g. CsvProcessor) set the shared variable to true (or 0 in case of our count).
  3. Controlling thread sends notification, which is not received by anybody, because there is no thread waiting on condition variable
  4. Waiting thread waits on condition variable. Since notification was already sent, it would wait until the next spurious wakeup, or next time
  when controlling thread sends notification. Potentially waiting indefinetly.
  */
  {
    std::unique_lock lock(*m_log_cv_mutex);
    m_active_processors->fetch_add(-1);
  }
  m_log_cv->notify_all();
};

AbstractProcessor *CsvProcessor::clone() const
{
  return new CsvProcessor(*this);
};

CsvProcessorBuilder CsvProcessor::builder(std::string name)
{
  return CsvProcessorBuilder(name);
}

void CsvProcessor::clean()
{
}

CsvProcessor::~CsvProcessor()
{
}
