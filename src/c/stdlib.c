#include <drivers/memory.h>

void free(const void* const ptr)
{
    mem_free(ptr);
}

void* malloc(const size_t size)
{
    return mem_dynalloc(size);
}

void* calloc(const size_t size)
{
    void* ptr = malloc(size);
    uint8_t* byteptr = (uint8_t*)ptr;

    for (size_t i = 0; i < size; i++)
    {
        byteptr[i] = 0;
    }

    return ptr;
}

void* realloc(void* const ptr, size_t size)
{
    return mem_dynresize(ptr, size);
}
