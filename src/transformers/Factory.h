#ifndef FACTORY_H
#define FACTORY_H

#include <map>
#include <memory>
#include <string>

#include "AbstractTransformer.h"
#include "Decorator.h"

#define REGISTER_DEC_TYPE(TYPE_NAME) \
    static DecoratorRegister<TYPE_NAME> reg

#define REGISTER_DEF_TYPE(TYPE_NAME, CONFIG_NAME) \
    DecoratorRegister<TYPE_NAME> TYPE_NAME::reg(#CONFIG_NAME)

/*
If Decorator is a base of T then std::is_base_of_v<A, T> is true and
std::enable_if_t becomes bool and we give it the value of true.

If Decorator is not a base of T then the condition is false and std::enable_if_t
results to nothing and the template is discarded as a viable candidate and
a compiler error will be generated.
*/
template <typename T, std::enable_if_t<std::is_base_of_v<Decorator, T>, bool> = true>
std::unique_ptr<AbstractTransformer> create_T(std::string column, std::unique_ptr<AbstractTransformer> wrapped)
{
    return std::make_unique<T>(column, std::move(wrapped));
}

template <typename T, std::enable_if_t<!std::is_base_of_v<Decorator, T>, bool> = true>
std::unique_ptr<AbstractTransformer> create_T(std::string column, std::unique_ptr<AbstractTransformer> wrapped = nullptr)
{
    return std::make_unique<T>(column);
}

struct Factory
{
    template <typename T>
    using MFP = std::unique_ptr<T> (*)(std::string, std::unique_ptr<T>);
    using map_type = std::map<std::string, MFP<AbstractTransformer>>;

private:
    static map_type *map;

protected:
    static map_type *get_map()
    {
        // Never deleted. Exists until program termination.
        if (!map)
        {
            map = new map_type;
        }
        return map;
    }

public:
    static std::unique_ptr<AbstractTransformer> get_instance(const std::string &s, const std::string column, std::unique_ptr<AbstractTransformer> wrapped = nullptr)
    {
        map_type::iterator it = get_map()->find(s);
        if (it == get_map()->end())
        {
            return nullptr;
        }
        else
        {
            return it->second(column, std::move(wrapped));
        }
    }
};

template <typename T>
struct DecoratorRegister : Factory
{
    DecoratorRegister(const std::string &config_name)
    {
        get_map()->insert(std::make_pair<std::string, MFP<AbstractTransformer>>(static_cast<std::string>(config_name), &create_T<T>));
    }
};

#endif