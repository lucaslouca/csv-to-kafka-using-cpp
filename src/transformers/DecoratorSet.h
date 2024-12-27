/**
 * Set the value row[from_column] to some value.
 *
 * @author Lucas Louca
 **/

#ifndef DECORATOR_SET_H
#define DECORATOR_SET_H

#include "Decorator.h"
#include "Factory.h"
#include "config/ConfigParser.h"
#include <string>

class DecoratorSet : public Decorator
{
private:
    REGISTER_DEC_TYPE(DecoratorSet);
    std::string m_value;

public:
    DecoratorSet(std::string column, std::unique_ptr<AbstractTransformer> transformer);
    virtual ~DecoratorSet() override;
    virtual void Operation(CSVRow &row) const override;
    friend class ConfigParser;
};

#endif