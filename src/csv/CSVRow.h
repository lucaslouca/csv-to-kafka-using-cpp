#ifndef CSVROW_H
#define CSVROW_H
#include <string_view>
#include <cstddef>
#include <istream>
#include <string>
#include <vector>
#include <map>

enum class CSVState
{
    UnquotedField,
    QuotedField,
    QuotedQuote
};

class CSVRow
{
public:
    std::string_view operator[](std::size_t index);
    std::string &operator[](const std::string column);
    std::size_t size() const;
    void next(std::istream &str);
    void set_columns(std::vector<std::string> &&columns);
    operator std::string();

private:
    std::string m_line;
    std::vector<std::string> m_fields;
    std::vector<int> m_data;
    std::vector<std::string> m_columns;
    std::map<std::string, std::string> m_map_data;

    // The non-member function operator>> will have access to CSVRow's private members
    friend std::istream &operator>>(std::istream &str, CSVRow &data);

    // to string
    friend std::ostream &operator<<(std::ostream &str, CSVRow &row);
};

#endif