#ifndef RESULT_CONTAINER_H
#define RESULT_CONTAINER_H

#include <map>
#include <string>
#include <vector>
#include <list>

template <typename T>
struct WrapVector
{
    typedef std::vector<T> type;
};
template <typename T>
struct WrapList
{
    typedef std::list<T> type;
};

template <typename T, template <typename> class C>
class ResultContainer
{
public:
    // (default) Constructor
    ResultContainer(std::string reference) : m_reference(reference) {}

    // Copy Constructor
    ResultContainer(const ResultContainer &other)
    {
        m_reference = other.m_reference;
        m_data = other.m_data;
    }

    // Move Constructor
    ResultContainer(ResultContainer &&other) : ResultContainer()
    {
        swap(*this, other);
    }

    // Copy Assignment
    // Take argument by-value: let the compiler do the copying for us :-)
    // Upon the end of the parameter's scope its destructor is called.
    ResultContainer &operator=(ResultContainer other)
    {
        // Use friend function to swap
        swap(*this, other);
        return *this;
    }

    // Move Assignment
    ResultContainer &operator=(ResultContainer &&other)
    {
        // Use friend function to swap
        swap(*this, other);
        return *this;
    }

    ~ResultContainer() {}

    /**
     * @brief For copy-and-swap idiom.
     *
     * Conceptually, it works by using the copy-constructor's
     * functionality to create a local copy of the data, then
     * takes the copied data with a swap function, swapping the
     * old data with the new data. The temporary copy then destructs,
     * taking the old data with it. We are left with a copy of the new data.
     *
     * @param first
     * @param second
     */
    friend void swap(ResultContainer &first, ResultContainer &second) // nothrow
    {
        // Enable ADL
        using std::swap;

        // By swapping the members of two objects,
        // the two objects are effectively swapped
        swap(first.m_reference, second.m_reference);
        swap(first.m_data, second.m_data);
    }

private:
    std::string m_reference;
    typename C<T>::type m_data; // trick to use a proxy
};

#endif
