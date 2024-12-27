#include "DecoratorUnuuid.h"
#include <iostream>
#include <algorithm>

REGISTER_DEF_TYPE(DecoratorUnuuid, unuuid);

DecoratorUnuuid::DecoratorUnuuid(std::string column, std::unique_ptr<AbstractTransformer> transformer) : Decorator(column, std::move(transformer))
{
}

void DecoratorUnuuid::Operation(CSVRow &row) const
{
    Decorator::Operation(row);

    if (!row[m_column].empty())
    {
        row[m_column].erase(std::remove(row[m_column].begin(), row[m_column].end(), '-'), row[m_column].end());
        std::transform(row[m_column].begin(), row[m_column].end(), row[m_column].begin(), [](unsigned char c)
                       { return std::tolower(c); });
    }
}

DecoratorUnuuid::~DecoratorUnuuid(){};