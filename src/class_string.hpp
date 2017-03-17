#pragma once

#include <stddef.h>
#include <stdint.h>

#include "cstring.hpp"
#include "memory.hpp"

class string
{
public:
    string(void);
    string(const char* const str);
    void dispose(void);

    // Capacity
    size_t size(void);
    void resize(const size_t newsize);
    void clear(void);
    bool empty(void);

    // Element access
    char& at(const size_t idx);
    char& front(void);
    char& back(void);

    // Modifiers
    void append(const char c);
    void append(const char* const str);
    void append(const string& str);
    void push_back(const char c);
    void push_back(const char* const str);
    void push_back(const string& str);
    void pop_back(void);

    // String operations
    char* c_str(void);
    string substr(const size_t pos, const size_t len = 0);
    bool compare(const string& str);
    string tolower(void);
    string toupper(void);

    // Operator overloads
    bool operator==(const string& str);
    char& operator[](const size_t idx);
    string& operator+=(const string& str);
    string operator+(const string& str);

private:
    void* m_ptr;
    char* m_ptrC;
    size_t m_size;

    void updatePtr(void* ptr);
    void fixend(void);
};
