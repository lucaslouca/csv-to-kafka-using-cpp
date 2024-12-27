/**
 * Prepend the value row[from_column] onto row[column].
 *
 * @author Lucas Louca
 **/

#ifndef DECORATOR_PREPEND_H
#define DECORATOR_PREPEND_H

#include "Decorator.h"
#include "Factory.h"
#include "config/ConfigParser.h"
#include <string>

class DecoratorPrepend : public Decorator
{
private:
    REGISTER_DEC_TYPE(DecoratorPrepend);
    std::string m_from_column;

public:
    DecoratorPrepend(std::string column, std::unique_ptr<AbstractTransformer> transformer);
    virtual ~DecoratorPrepend() override;
    virtual void Operation(CSVRow &row) const override;
    friend class ConfigParser;
};

#endif