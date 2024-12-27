#include "CSVIterator.h"
#include <sstream>

CSVIterator::CSVIterator(std::istream &stream, bool has_header) : m_stream(stream.good() ? &stream : nullptr), m_has_header(has_header)
{
    ++(*this);
}

CSVIterator::CSVIterator() : m_stream(nullptr) {}

void CSVIterator::set_columns(std::istream &stream)
{
    std::vector<std::string> columns;
    std::string header;
    std::getline(stream, header);
    std::stringstream ss(header);
    while (ss.good())
    {
        std::string column;
        getline(ss, column, ',');
        column.erase(std::remove(column.begin(), column.end(), '\n'), column.end());
        column.erase(std::remove(column.begin(), column.end(), '\r'), column.end());
        columns.emplace_back(column);
    }

    m_row.set_columns(std::move(columns));
}

// Pre Increment
CSVIterator &CSVIterator::operator++()
{
    if (m_stream)
    {
        if (m_has_header)
        {
            set_columns(*m_stream);
            m_has_header = false;
        }

        if (!((*m_stream) >> m_row))
        {
            m_stream = nullptr;
        }
    }
    return *this;
}

// Post Increment
CSVIterator CSVIterator::operator++(int)
{
    CSVIterator tmp(*this);
    ++(*this);
    return tmp;
}

CSVRow &CSVIterator::operator*()
{
    return m_row;
}
CSVRow *CSVIterator::operator->()
{
    return &m_row;
}

bool CSVIterator::operator==(CSVIterator const &rhs) const
{
    return ((this == &rhs) || ((this->m_stream == nullptr) && (rhs.m_stream == nullptr)));
}

bool CSVIterator::operator!=(CSVIterator const &rhs) const
{
    return !((*this) == rhs);
}
