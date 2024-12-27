#include "BaseTransformer.h"
#include <iostream>

REGISTER_DEF_TYPE(BaseTransformer, void);

BaseTransformer::BaseTransformer(std::string column) : AbstractTransformer(column)
{
}

BaseTransformer::~BaseTransformer(){};

void BaseTransformer::Operation(CSVRow &row) const
{
    if (!row[m_column].empty())
    {
        row[m_column] = "BASE " + row[m_column];
    }
}