#ifndef SCHEMA_REGISTRY_H
#define SCHEMA_REGISTRY_H

#include <string>
#include <atomic>
#include <mutex>
#include <libserdes/serdescpp.h>
#include <libserdes/serdescpp-avro.h>

class SchemaRegistry
{
private:
    std::atomic<bool> m_uninitialized;
    Serdes::Avro *m_serdes;
    SchemaRegistry(const std::string *h);
    static SchemaRegistry &instance_impl(const std::string *h);

public:
    SchemaRegistry(const SchemaRegistry &) = delete;
    void operator=(const SchemaRegistry &) = delete;

    static void init(const std::string h);
    static SchemaRegistry &instance();
    int fetch_value_schema_id(const std::string &schema_name);
    int register_value_schema(const std::string &schema_name, const std::string &schema_def);
};

#endif