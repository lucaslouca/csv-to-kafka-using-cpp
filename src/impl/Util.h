#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <avro/Generic.hh>
#include <exception>
#include <stdexcept>

namespace Util
{

    bool str_ends_with(const char *str, const char *suffix);
    avro::GenericDatum create_datum_for_type(const std::string &value, const std::string &type);
    std::string what(const std::exception_ptr &eptr);
};

#endif
