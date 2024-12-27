/**
 * Lookup values in the column and map them to a different value.
 *
 * @author Lucas Louca
 **/

#ifndef DECORATOR_MAP_H
#define DECORATOR_MAP_H

#include "Decorator.h"
#include "Factory.h"
#include "config/ConfigParser.h"
#include <map>

class DecoratorMap : public Decorator
{
public:
    DecoratorMap(std::string column, std::unique_ptr<AbstractTransformer> transformer);
    virtual ~DecoratorMap() override;
    virtual void Operation(CSVRow &row) const override;
    friend class ConfigParser;

private:
    REGISTER_DEC_TYPE(DecoratorMap);
    std::map<std::string, std::string> lookup;
};

#endif