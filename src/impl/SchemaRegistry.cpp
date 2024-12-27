#include "SchemaRegistry.h"
#include "logging/Logging.h"
#include <thread>
#include <iostream>
#include <signal.h>
#include <sstream>

static const std::string name = "SchemaRegistry";

SchemaRegistry::SchemaRegistry(const std::string *h)
{
    if (h)
    {
        Serdes::Conf *m_sconf = Serdes::Conf::create();

        std::string errstr;
        if (m_sconf->set("schema.registry.url", *h, errstr))
        {
            Logging::ERROR("Configuration failed: " + errstr, name);
            kill(getpid(), SIGINT);
        }

        /* Default framing CP1 */
        if (m_sconf->set("serializer.framing", "cp1", errstr))
        {
            Logging::ERROR("Configuration failed: " + errstr, name);
            kill(getpid(), SIGINT);
        }

        m_serdes = Serdes::Avro::create(m_sconf, errstr);
        if (!m_serdes)
        {
            Logging::ERROR("Failed to create Serdes handle: " + errstr, name);
            kill(getpid(), SIGINT);
        }

        m_uninitialized.store(false);
    }
}

SchemaRegistry &SchemaRegistry::instance_impl(const std::string *h = nullptr)
{
    static SchemaRegistry i{h};
    return i;
}

void SchemaRegistry::init(const std::string h)
{
    instance_impl(&h);
}

SchemaRegistry &SchemaRegistry::instance()
{
    SchemaRegistry &i = instance_impl();

    if (i.m_uninitialized.load())
    {
        throw std::logic_error("SchemaRegistry was not initialized");
    }

    return i;
}

int SchemaRegistry::fetch_value_schema_id(const std::string &schema_name)
{
    if (!schema_name.empty())
    {
        std::string errstr;
        Serdes::Schema *schema = Serdes::Schema::get(m_serdes, schema_name + "-value", errstr);
        if (schema)
        {
            std::stringstream ss;
            ss << "Fetched schema: '"
               << schema->name() << "'"
               << ", id: "
               << schema->id();

            Logging::INFO(ss.str(), name);
            return schema->id();
        }
        else
        {
            std::stringstream ss;
            ss << "No schema with name '"
               << schema_name << "'"
               << " found";
            Logging::INFO(ss.str(), name);
        }
    }

    return -1;
}

int SchemaRegistry::register_value_schema(const std::string &schema_name, const std::string &schema_def)
{

    std::string errstr;
    Serdes::Schema *schema = Serdes::Schema::add(m_serdes, schema_name + "-value", schema_def, errstr);

    if (schema)
    {
        std::stringstream ss;
        ss << "Registered schema: '"
           << schema->name() << "'"
           << ", id: "
           << schema->id();
        Logging::INFO(ss.str(), name);
        return schema->id();
    }
    else
    {
        std::stringstream ss;
        ss << "Failed to register new schema: '"
           << schema_name << "'"
           << ", id: "
           << schema->id()
           << ", definition: "
           << schema_def;
        Logging::ERROR(ss.str(), name);
    }
    return -1;
}