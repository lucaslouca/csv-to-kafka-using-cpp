#ifndef BASE_TRANSFORMER_H
#define BASE_TRANSFORMER_H

#include "AbstractTransformer.h"
#include "Factory.h"

class BaseTransformer : public AbstractTransformer
{
public:
    BaseTransformer(std::string column);
    virtual ~BaseTransformer() override;
    virtual void Operation(CSVRow &row) const override;

private:
    REGISTER_DEC_TYPE(BaseTransformer);
};

#endif