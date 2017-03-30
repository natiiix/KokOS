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
    inline size_t size(void)
    { return m_size; }
    //
    inline void resize(const size_t newsize)
    { updatePtr(realloc(m_ptr, (m_size = newsize) * m_sizeofT)); }
    //
    inline void clear(void)
    { updatePtr(realloc(m_ptr, m_size = 0)); }
    //
    inline bool empty(void)
    { return !m_size; }

    // Element access
    inline T& operator[](const size_t idx)
    { return at(idx); }
    //
    inline T& at(const size_t idx)
    { return m_ptrT[idx]; }
    //
    inline T& front(void)
    { return m_ptrT[0]; }
    //
    inline T& back(void)
    { return m_ptrT[m_size - 1]; }

    // Modifiers
    inline void push_back(T element)
    {
        updatePtr(realloc(m_ptr, ++m_size * m_sizeofT));
        m_ptrT[m_size - 1] = element;
    }
    //
    inline void pop_back(void)
    { updatePtr(realloc(m_ptr, --m_size * m_sizeofT)); }

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
};

// Frees the memory used by a vector of string including the vector itself
class string; template <> void vector<string>::dispose(void);
