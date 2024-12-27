#include "DecoratorMap.h"

REGISTER_DEF_TYPE(DecoratorMap, map);

DecoratorMap::DecoratorMap(std::string column, std::unique_ptr<AbstractTransformer> transformer) : Decorator(column, std::move(transformer))
{
}

void DecoratorMap::Operation(CSVRow &row) const
{
    Decorator::Operation(row);

    if (!row[m_column].empty())
    {
        std::map<std::string, std::string>::const_iterator it = lookup.find(row[m_column]);
        if (it != lookup.cend())
        {
            row[m_column] = it->second;
        }
    }
}

DecoratorMap::~DecoratorMap(){};