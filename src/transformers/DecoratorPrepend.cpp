#include "DecoratorPrepend.h"

REGISTER_DEF_TYPE(DecoratorPrepend, prepend);

DecoratorPrepend::DecoratorPrepend(std::string column, std::unique_ptr<AbstractTransformer> transformer) : Decorator(column, std::move(transformer))
{
}

void DecoratorPrepend::Operation(CSVRow &row) const
{
    Decorator::Operation(row);

    if (!row[m_column].empty() && !row[m_from_column].empty())
    {
        row[m_column] = row[m_from_column] + row[m_column];
    }
}

DecoratorPrepend::~DecoratorPrepend(){};