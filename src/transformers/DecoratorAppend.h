/**
 * Append the value row[from_column] onto row[column].
 *
 * @author Lucas Louca
 **/

#ifndef DECORATOR_APPEND_H
#define DECORATOR_APPEND_H

#include "Decorator.h"
#include "Factory.h"
#include "config/ConfigParser.h"
#include <string>

class DecoratorAppend : public Decorator
{
private:
    REGISTER_DEC_TYPE(DecoratorAppend);
    std::string from_column;

public:
    DecoratorAppend(std::string column, std::unique_ptr<AbstractTransformer> transformer);
    virtual ~DecoratorAppend() override;
    virtual void Operation(CSVRow &row) const override;
    friend class ConfigParser;
};

#endif