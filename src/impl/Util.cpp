#include "Util.h"
#include <cstring>

bool Util::str_ends_with(const char *str, const char *suffix)
{
    if (str == NULL || suffix == NULL)
    {
        return false;
    }

    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);

    if (suffix_len > str_len)
    {
        return false;
    }

    return 0 == strncmp(str + str_len - suffix_len, suffix, suffix_len);
}

avro::GenericDatum Util::create_datum_for_type(const std::string &value, const std::string &type)
{
    if (!type.compare("string"))
    {
        return avro::GenericDatum(value);
    }
    else if (!type.compare("float"))
    {
        return avro::GenericDatum(std::stof(value));
    }
    else if (!type.compare("double"))
    {
        return avro::GenericDatum(std::stod(value));
    }
    else if (!type.compare("int"))
    {
        return avro::GenericDatum(std::stoi(value));
    }
    else if (!type.compare("long"))
    {
#ifdef __linux__
        return avro::GenericDatum(std::stol(value));
#elif __APPLE__
        return avro::GenericDatum(strtoll(value.c_str(), NULL, 10));
#endif
    }
    else
    {
        throw std::logic_error("Unknown datum type");
    }
}

std::string Util::what(const std::exception_ptr &eptr = std::current_exception())
{
    if (!eptr)
    {
        throw std::bad_exception();
    }

    try
    {
        std::rethrow_exception(eptr);
    }
    catch (const std::exception &e)
    {
        return e.what();
    }
    catch (const std::string &e)
    {
        return e;
    }
    catch (const char *e)
    {
        return e;
    }
    catch (...)
    {
        return "who knows";
    }
}
