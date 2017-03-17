#pragma once

#include <stddef.h>
#include <stdint.h>

#include "memory.hpp"

template<class T>
class vector
{
public:
    vector(void) :
        m_sizeofT(sizeof(T)),
        m_ptr(mem::dynalloc(0)),
        m_ptrT((T*)m_ptr),
        m_size(0) { }

    // delete m_ptr throws a compiler warning [-Wdelete-incomplete]
    // therefore it's better to pass the element pointer to the delete
    void dispose(void)
    { delete m_ptrT; }

    void* getPtr(void)
    { return m_ptr; }

    T* getPtrT(void)
    { return m_ptrT; }

    // Capacity
    size_t size(void)
    { return m_size; }
    //
    void resize(const size_t newsize)
    { updatePtr(mem::dynresize(m_ptr, (m_size = newsize) * m_sizeofT)); }
    //
    void clear(void)
    { updatePtr(mem::dynresize(m_ptr, m_size = 0)); }
    //
    bool empty(void)
    { return !m_size; }

    // Element access
    T& operator[](const size_t idx)
    { return at(idx); }
    //
    T& at(const size_t idx)
    { return m_ptrT[idx]; }
    //
    T& front(void)
    { return m_ptrT[0]; }
    //
    T& back(void)
    { return m_ptrT[m_size - 1]; }

    // Modifiers
    void push_back(T element)
    {
        updatePtr(mem::dynresize(m_ptr, ++m_size * m_sizeofT));
        m_ptrT[m_size - 1] = element;
    }
    //
    void pop_back(void)
    { updatePtr(mem::dynresize(m_ptr, --m_size * m_sizeofT)); }

private:
    // Size of a single element
    size_t m_sizeofT;
    // Pointer to the memory segment
    void* m_ptr;
    // Pointer to the first element of the vector, used for easier element access
    T* m_ptrT;
    // Amount of elements stored in the vector
    size_t m_size;

    void updatePtr(void* ptr)
    {
        m_ptr = ptr;
        m_ptrT = (T*)ptr;
    }
};
