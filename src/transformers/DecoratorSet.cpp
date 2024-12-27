#include "DecoratorSet.h"

REGISTER_DEF_TYPE(DecoratorSet, set);

DecoratorSet::DecoratorSet(std::string column, std::unique_ptr<AbstractTransformer> transformer) : Decorator(column, std::move(transformer))
{
}

void DecoratorSet::Operation(CSVRow &row) const
{
    Decorator::Operation(row);
    row[m_column] = m_value;
}

DecoratorSet::~DecoratorSet(){};