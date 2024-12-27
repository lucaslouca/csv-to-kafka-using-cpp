#ifndef CSVITERATOR_H
#define CSVITERATOR_H
#include "CSVRow.h"
#include <iostream>

class CSVIterator
{
public:
    typedef std::input_iterator_tag iterator_category;
    typedef CSVRow value_type;
    typedef std::size_t difference_type;
    typedef CSVRow *pointer;
    typedef CSVRow &reference;

    CSVIterator(std::istream &stream, bool has_header);
    CSVIterator();

    // Pre Increment
    CSVIterator &operator++();

    // Post Increment
    CSVIterator operator++(int);

    CSVRow &operator*();
    CSVRow *operator->();

    bool operator==(CSVIterator const &rhs) const;
    bool operator!=(CSVIterator const &rhs) const;

private:
    bool m_has_header;
    std::istream *m_stream;
    CSVRow m_row;
    void set_columns(std::istream &stream);
};

#endif