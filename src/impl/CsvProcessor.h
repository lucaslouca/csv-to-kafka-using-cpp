#ifndef CSV_PROCESSOR_H
#define CSV_PROCESSOR_H

#include "AbstractProcessor.h"
#include "impl/PollResult.h"
#include "config/SchemaConfig.h"
#include <librdkafka/rdkafkacpp.h>

#include <avro/ValidSchema.hh>
#include <avro/Generic.hh>

class CsvProcessorBuilder;

class CsvProcessor : public AbstractProcessor
{
private:
  void handle(PollResult d) override;
  void clean() override;
  void publish(CSVRow &row, size_t &old_count);
  RdKafka::Producer *m_kafka_producer;
  std::map<std::string, SchemaConfig> *m_schemas;
  ssize_t serialize(avro::ValidSchema schema, const int32_t schema_id, const avro::GenericDatum datum, std::vector<char> &out, std::string &errstr);
  std::pair<std::string, int> *m_max_age_config;

public:
  CsvProcessor(std::string name_, std::shared_ptr<SignalChannel> sig_channel_);
  ~CsvProcessor() override;
  AbstractProcessor *clone() const override;
  friend class CsvProcessorBuilder;
  static CsvProcessorBuilder builder(std::string name);

  /**
   * Emit to stdout
   **/
  template <class... Args>
  void emit(Args &&...args)
  {
    using expand = int[];

    // instantiate array
    void(expand{
        0,                          // in case no args were provided
        ((std::cout << args), 0)... // stream args to std::cout, discard result and place 0 into array
    });                             // wrapped around void to discard int array

    void();
  }
};

#endif
