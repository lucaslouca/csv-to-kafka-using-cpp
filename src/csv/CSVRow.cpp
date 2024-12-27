#include "CSVRow.h"
#include <istream>
#include <sstream>
#include <iostream>
// std::string_view CSVRow::operator[](std::size_t index)
// {
//     return std::string_view(&m_line[m_data[index] + 1], m_data[index + 1] - (m_data[index] + 1));
// }

std::string_view CSVRow::operator[](std::size_t index)
{
    return m_fields[index];
}

std::string &CSVRow::operator[](const std::string column)
{
    return m_map_data[column];
}

std::size_t CSVRow::size() const
{
    return m_data.size() - 1;
}

// void CSVRow::next(std::istream &stream)
// {

//     std::getline(stream, m_line);
//     m_data.clear();
//     m_data.emplace_back(-1);
//     std::string::size_type pos = 0;
//     std::size_t col = 0;
//     while ((pos = m_line.find(',', pos)) != std::string::npos)
//     {
//         m_data.emplace_back(pos);
//         if (col < m_columns.size()) // need this check in case CSV didn't have a header
//         {
//             m_map_data[m_columns[col]] = (*this)[col];
//             ++col;
//         }
//         ++pos;
//     }

//     pos = m_line.size();
//     m_data.emplace_back(pos);
//     m_map_data[m_columns[col]] = (*this)[col];
// }

void CSVRow::next(std::istream &stream)
{
    CSVState state = CSVState::UnquotedField;

    std::getline(stream, m_line);
    m_line.erase(std::remove(m_line.begin(), m_line.end(), '\n'), m_line.end());
    m_line.erase(std::remove(m_line.begin(), m_line.end(), '\r'), m_line.end());

    m_fields.clear();
    m_fields.push_back("");
    size_t i = 0; // index of the current field
    for (char c : m_line)
    {
        switch (state)
        {
        case CSVState::UnquotedField:
            switch (c)
            {
            case ',': // end of field
                m_map_data[m_columns[i]] = m_fields[i];
                m_fields.push_back("");
                i++;
                break;
            case '"':
                state = CSVState::QuotedField;
                break;
            default:
                m_fields[i].push_back(c);
                break;
            }
            break;
        case CSVState::QuotedField:
            switch (c)
            {
            case '"':
                state = CSVState::QuotedQuote;
                break;
            default:
                m_fields[i].push_back(c);
                break;
            }
            break;
        case CSVState::QuotedQuote:
            switch (c)
            {
            case ',': // , after closing quote
                m_map_data[m_columns[i]] = m_fields[i];
                m_fields.push_back("");
                i++;
                state = CSVState::UnquotedField;
                break;
            case '"': // "" -> "
                m_fields[i].push_back('"');
                state = CSVState::QuotedField;
                break;
            default: // end of quote
                state = CSVState::UnquotedField;
                break;
            }
            break;
        }
    }

    m_map_data[m_columns[i]] = m_fields[i];
}

void CSVRow::set_columns(std::vector<std::string> &&columns)
{
    m_columns = columns;
}

std::istream &operator>>(std::istream &stream, CSVRow &data)
{
    data.next(stream);
    return stream;
}

CSVRow::operator std::string()
{
    std::stringstream ss;
    std::string sep = "";
    for (const std::string &col : m_columns)
    {
        ss << sep << (*this)[col];
        sep.assign(",");
    }
    return ss.str();
}

std::ostream &operator<<(std::ostream &str, CSVRow &row)
{
    str << (std::string)row;
}