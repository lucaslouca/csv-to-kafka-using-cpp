#ifndef ABSTRACT_TRANSFORMER_H
#define ABSTRACT_TRANSFORMER_H

#include "csv/CSVRow.h"
#include "logging/Logging.h"

class AbstractTransformer
{
protected:
    const std::string m_column;

public:
    AbstractTransformer();
    AbstractTransformer(std::string column);
    virtual ~AbstractTransformer();
    virtual void Operation(CSVRow &row) const = 0;
};

#endif