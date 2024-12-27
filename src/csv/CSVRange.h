#ifndef CSVRANGE_H
#define CSVRANGE_H
#include "CSVIterator.h"
#include <istream>

class CSVRange
{
public:
    CSVRange(std::istream &str, bool has_header);
    CSVIterator begin() const;
    CSVIterator end() const;

private:
    std::istream &m_stream;
    bool m_has_header;
};

#endif