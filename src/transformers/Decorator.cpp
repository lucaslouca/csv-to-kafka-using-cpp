
#include "Decorator.h"

Decorator::Decorator(std::string column, std::unique_ptr<AbstractTransformer> transformer) : m_transformer(std::move(transformer)), AbstractTransformer(column)
{
}

void Decorator::Operation(CSVRow &row) const
{
    if (this->m_transformer)
    {
        // Delegate work to wrapped transformer
        this->m_transformer->Operation(row);
    }
}

Decorator::~Decorator(){};