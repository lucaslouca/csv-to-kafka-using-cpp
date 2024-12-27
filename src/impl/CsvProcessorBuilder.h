#ifndef CSV_PROCESSOR_BUILDER_H
#define CSV_PROCESSOR_BUILDER_H

#include "CsvProcessor.h"

class CsvProcessorBuilder
{
private:
    std::string m_name;
    std::vector<std::unique_ptr<AbstractTransformer>> *m_transformers = nullptr;
    std::atomic<size_t> *m_active_processors = nullptr;
    std::condition_variable *m_log_cv = nullptr;
    std::mutex *m_log_cv_mutex = nullptr;
    RdKafka::Producer *m_kafka_producer;
    std::string m_kafka_topic;
    std::shared_ptr<SignalChannel> m_sig_channel;
    std::map<std::string, SchemaConfig> *m_schemas;
    std::pair<std::string, int> *m_max_age;

public:
    CsvProcessorBuilder(std::string name);
    CsvProcessorBuilder &with_transformers(std::vector<std::unique_ptr<AbstractTransformer>> *t);
    CsvProcessorBuilder &with_active_processors_counter(std::atomic<size_t> *c);
    CsvProcessorBuilder &with_logging_cv(std::condition_variable *cv);
    CsvProcessorBuilder &with_logging_mutex(std::mutex *m);
    CsvProcessorBuilder &with_kafka_producer(RdKafka::Producer *kp);
    CsvProcessorBuilder &with_schemas(std::map<std::string, SchemaConfig> *s);
    CsvProcessorBuilder &with_sig_channel(std::shared_ptr<SignalChannel> sc);
    CsvProcessorBuilder &with_drop_max_age(std::pair<std::string, int> *p);
    std::unique_ptr<CsvProcessor> build() const;
};

#endif