#pragma once

#include <stddef.h>
#include <stdint.h>

template<class T>
class vector;

class string
{
public:
	// ---- GENERIC ----
	// Constructs an empty string object
    string(void);
	// Constructs a string object from cstring
    string(const char* const str);
	// Frees the memory used by this string
    void dispose(void);

    // ---- CAPACITY ----
	// Returns the length of the string
    size_t size(void);
	// Resizes the string and puts '\0' at the end of it
    void resize(const size_t newsize);
	// Clears the string by resizing it to 0
    void clear(void);
	// Returns true if the string is empty (if it's length is 0)
    bool empty(void);

    // ---- ELEMENT ACCESS ----
	// Returns the character at index idx
    char& at(const size_t idx);
	// Returns the first character of the string
    char& front(void);
	// Returns the last character of the string (before the final '\0')
    char& back(void);

    // ---- MODIFIERS ----
	// Synonymous to push_back(const char c)
    void append(const char c);
	// Synonymous to push_back(const char* const str)
    void append(const char* const str);
	// Synonymous to push_back(const string& str)
    void append(const string& str);
	// Appends the character c to the end of the string
    void push_back(const char c);
	// Appends the cstring str to the end of the string
    void push_back(const char* const str);
	// Appends the string str to the end of the string
    void push_back(const string& str);
	// Removes the last character from the string
    void pop_back(void);
	// Removes the last popcount number of character
	void pop_back(const size_t popcount);

    // ---- STRING OPERATIONS ----
	// Returns pointer to the cstring storage
    char* c_str(void);
	// Returns part of a string starting at pos with the length len
    string substr(const size_t pos, const size_t len = 0);
	// Compares two string, returns true if they're the same, false if they're different
    bool compare(const string& str);
	// Compares the string to a cstring str, returns true if they're the same
	bool compare(const char* const str);
	// Converts a string to lowercase and returns it
    string tolower(void);
	// Converts a string to uppercase and returns it
    string toupper(void);
	// Splits the string, uses char as delimiter
	vector<string> split(const char cDelimiter, const bool removeEmpty = false);
	// Splits the string, uses string as delimiter
	vector<string> split(const char* const strDelimiter, const bool removeEmpty = false);

    // ---- OPERATOR OVERLOADS ----
	// Synonymous to compare(const string& str)
    bool operator==(const string& str);
	// Synonymous to compare(const char* const str)
	bool operator==(const char* const str);
	// Synonymous to at(const size_t idx)
    char& operator[](const size_t idx);
	// Synonymous to push_back(const string& str)
    string& operator+=(const string& str);
	// Joins two strings and returns the result
    string operator+(const string& str);

private:
    void* m_ptr;
    char* m_ptrC;
    size_t m_size;

	// Updates m_ptr and m_ptrC
    void updatePtr(void* ptr);
	// Puts '\0' at the end of the string
    void fixend(void);
	// Adds part of the string into the string vector (ignores empty string parts if removeEmpty is set to true)
	void splitVectorAdd(vector<string>& vectsplit, const size_t start, const size_t end, const bool removeEmpty);
};
