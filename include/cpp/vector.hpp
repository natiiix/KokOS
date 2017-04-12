#pragma once

#include <stddef.h>
#include <stdint.h>

#include <c/stdlib.h>

template<class T>
class vector
{
public:
    inline vector(void) :
        m_sizeofT(sizeof(T)),
        m_ptr(malloc(0)),
        m_ptrT((T*)m_ptr),
        m_size(0) { }

    inline void dispose(void)
    { delete m_ptrT; }

    inline void* getPtr(void)
    { return m_ptr; }

    inline T* getPtrT(void)
    { return m_ptrT; }

    // Capacity
    inline size_t size(void) const
    { return m_size; }
    //
    inline void resize(const size_t newsize)
    { updatePtr(realloc(m_ptr, (m_size = newsize) * m_sizeofT)); }
    //
    inline void clear(void)
    { resize(0); }
    //
    inline bool empty(void) const
    { return !m_size; }

    // Element access
    inline T& operator[](const size_t idx)
    { return m_ptrT[idx]; }
    //
    inline T at(const size_t idx) const
    { return m_ptrT[idx]; }
    //
    inline T& front(void)
    { return m_ptrT[0]; }
    //
    inline T& back(void)
    { return m_ptrT[m_size - 1]; }

    // Modifiers
    inline void push_back(const T& element)
    {
        resize(m_size + 1);
        m_ptrT[m_size - 1] = element;
    }
    //
    inline void pop_back(void)
    {
        if (m_size > 0)
        {
            resize(m_size - 1);
        }
    }
    
	// Removes an element / multiple elements from a specified position in the vector
	inline void remove(const size_t pos, const size_t len = 1)
    {
        shiftElementsLeft(pos, len);
    }
	// Inserts an element at a specified position in the vector
	inline void insert(const T& element, const size_t pos)
    {
        shiftElementsRight(pos, 1);

        m_ptrT[pos] = element;
    }
	// Inserts a vector at a specified position in the vector
	inline void insert(const vector<T>& vec, const size_t pos)
    {
        size_t vecsize = vec.size();
        shiftElementsRight(pos, vecsize);

        for (size_t i = 0; i < vecsize; i++)
        {
            m_ptrT[pos + i] = vec[i];
        }
    }

private:
    // Size of a single element
    size_t m_sizeofT;
    // Pointer to the memory segment
    void* m_ptr;
    // Pointer to the first element of the vector, used for easier element access
    T* m_ptrT;
    // Amount of elements stored in the vector
    size_t m_size;

    inline void updatePtr(void* ptr)
    {
        m_ptr = ptr;
        m_ptrT = (T*)ptr;
    }

    // Makes space for elements to insert
    inline void shiftElementsRight(const size_t pos, const size_t offset)
    {
        if (pos >= m_size)
        {
            return;
        }

        size_t oldsize = m_size;
        resize(m_size + offset);

        size_t shiftedElements = oldsize - pos;

        for (size_t i = 0; i < shiftedElements; i++)
        {
            m_ptrT[m_size - 1 - i] = m_ptrT[oldsize - 1 - i];
        }
    }

    // Gets rid of removed elements
    inline void shiftElementsLeft(const size_t pos, const size_t offset)
    {
        if (pos >= m_size || m_size - pos < offset)
        {
            return;
        }
        
        for (size_t i = pos; i < m_size; i++)
        {
            m_ptrT[i] = m_ptrT[i + offset];
        }

        resize(m_size - offset);
    }
};

// Frees the memory used by a vector of string including the vector itself
#include <cpp/string.hpp>

template <>
inline void vector<string>::dispose(void)
{
    for (size_t i = 0; i < m_size; i++)
	{
		m_ptrT[i].dispose();
	}

    delete m_ptrT;
}

template <>
inline void vector<string>::pop_back(void)
{
    if (m_size > 0)
    {
        m_ptrT[m_size - 1].dispose();
        resize(m_size - 1);
    }
}

template <>
inline void vector<string>::clear(void)
{
    for (size_t i = 0; i < m_size; i++)
    {
        m_ptrT[i].dispose();
    }

    resize(0);
}

template <>
inline void vector<string>::remove(const size_t pos, const size_t len)
{
    // Dispose those vector elements that are going to be removed from the vector
    for (size_t i = 0; i < len; i++)
    {
        m_ptrT[pos + i].dispose();
    }

    shiftElementsLeft(pos, len);
}
