#include "CSVRange.h"

CSVRange::CSVRange(std::istream &str, bool has_header) : m_stream(str), m_has_header(has_header)
{
}

CSVIterator CSVRange::begin() const
{
    return CSVIterator{m_stream, m_has_header};
}

CSVIterator CSVRange::end() const
{
    return CSVIterator{};
}
