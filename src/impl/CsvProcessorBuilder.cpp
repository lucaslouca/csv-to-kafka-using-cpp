#include "CsvProcessorBuilder.h"
#include "CsvProcessor.h"

CsvProcessorBuilder::CsvProcessorBuilder(std::string name) : m_name(name)
{
}

CsvProcessorBuilder &CsvProcessorBuilder::with_transformers(std::vector<std::unique_ptr<AbstractTransformer>> *t)
{
    m_transformers = t;
    return *this;
}

CsvProcessorBuilder &CsvProcessorBuilder::with_active_processors_counter(std::atomic<size_t> *c)
{
    m_active_processors = c;
    return *this;
}

CsvProcessorBuilder &CsvProcessorBuilder::with_logging_cv(std::condition_variable *cv)
{
    m_log_cv = cv;
    return *this;
}

CsvProcessorBuilder &CsvProcessorBuilder::with_logging_mutex(std::mutex *m)
{
    m_log_cv_mutex = m;
    return *this;
}

CsvProcessorBuilder &CsvProcessorBuilder::with_kafka_producer(RdKafka::Producer *kp)
{
    m_kafka_producer = kp;
    return *this;
}

CsvProcessorBuilder &CsvProcessorBuilder::with_schemas(std::map<std::string, SchemaConfig> *s)
{
    m_schemas = s;
    return *this;
}

CsvProcessorBuilder &CsvProcessorBuilder::with_sig_channel(std::shared_ptr<SignalChannel> sc)
{
    m_sig_channel = sc;
    return *this;
}

CsvProcessorBuilder &CsvProcessorBuilder::with_drop_max_age(std::pair<std::string, int> *p)
{
    m_max_age = p;
    return *this;
}

std::unique_ptr<CsvProcessor> CsvProcessorBuilder::build() const
{
    if (!m_transformers)
    {
        throw std::runtime_error("No transformers provided");
    }

    if (!m_active_processors)
    {
        throw std::runtime_error("No active processors counter provided");
    }

    if (!m_log_cv)
    {
        throw std::runtime_error("No log cv provided");
    }

    if (!m_log_cv_mutex)
    {
        throw std::runtime_error("No log cv mutex provided");
    }

    if (!m_kafka_producer)
    {
        throw std::runtime_error("No kafka producer provided");
    }

    if (m_schemas->empty())
    {
        throw std::runtime_error("No schemas provided");
    }

    if (!m_sig_channel)
    {
        throw std::runtime_error("No signal channel provided");
    }

    std::unique_ptr<CsvProcessor> processor = std::make_unique<CsvProcessor>(m_name, m_sig_channel);
    processor->m_transformers = m_transformers;
    processor->m_active_processors = m_active_processors;
    processor->m_log_cv = m_log_cv;
    processor->m_log_cv_mutex = m_log_cv_mutex;
    processor->m_kafka_producer = m_kafka_producer;
    processor->m_schemas = m_schemas;

    if (m_max_age)
    {
        processor->m_max_age_config = m_max_age;
    }

    return processor;
}