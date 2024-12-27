#include "DecoratorAppend.h"

REGISTER_DEF_TYPE(DecoratorAppend, append);

DecoratorAppend::DecoratorAppend(std::string column_, std::unique_ptr<AbstractTransformer> transformer_) : Decorator(column_, std::move(transformer_))
{
}

void DecoratorAppend::Operation(CSVRow &row) const
{
    Decorator::Operation(row);

    if (!row[m_column].empty() && !row[from_column].empty())
    {
        row[m_column] += row[from_column];
    }
}

DecoratorAppend::~DecoratorAppend(){};