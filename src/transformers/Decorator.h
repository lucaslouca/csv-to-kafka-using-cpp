#ifndef DECORATOR_H
#define DECORATOR_H

#include <memory>
#include "AbstractTransformer.h"

class Decorator : public AbstractTransformer
{
protected:
    std::unique_ptr<AbstractTransformer> m_transformer;

public:
    Decorator(std::string column, std::unique_ptr<AbstractTransformer> transformer);
    virtual ~Decorator() override;
    virtual void Operation(CSVRow &row) const override;
};

#endif