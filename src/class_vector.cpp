#include "class_vector.hpp"

#include "class_string.hpp"

template <>
void vector<string>::dispose(void)
{
    for (size_t i = 0; i < m_size; i++)
	{
		m_ptrT[i].dispose();
	}

    delete m_ptrT;
}
