/**
 * Remove dashes/hyphens from row[column] values and make them lowercase.
 *
 * @author Lucas Louca
 **/

#ifndef DECORATOR_UNUUID_H
#define DECORATOR_UNUUID_H

#include "Decorator.h"
#include "Factory.h"

class DecoratorUnuuid : public Decorator
{
public:
    DecoratorUnuuid(std::string column, std::unique_ptr<AbstractTransformer> transformer);
    virtual ~DecoratorUnuuid() override;
    virtual void Operation(CSVRow &row) const override;

private:
    /*
    static: Dynamic initialization of an object with static storage duration is guaranteed to
    happen before execution of any function defined in the same translation unit. If there are
    no such functions, or your program never calls them, then there's no guarantee it will ever
    be initialised.

    static DecoratorRegister<DecoratorUnuuid> reg;
    */
    REGISTER_DEC_TYPE(DecoratorUnuuid);
};
#endif