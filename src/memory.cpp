#include "memory.hpp"

namespace mem
{
    const size_t memsize = 1024*1024;
    uint8_t memory[memsize];
    bool bitmap[memsize];
    void* memstart = (void*) &memory[0];
    void* invalidptr = (void*) &memory[memsize];

    void init(void)
    {
        // Clean and unallocate the whole memory
        for (size_t i = 0; i < memsize; i++)
        {
            memory[i] = 0;
            bitmap[i] = false;
        }
    }

    void* alloc(const size_t bytes)
    {
        if (bytes <= 0)
            return invalidptr;

        // Find first unallocated byte
        for (size_t i = 0; i < (memsize - (bytes - 1)); i++)
        {
            // If byte is not allocated
            if (!bitmap[i])
            {
                bool spacefound = true;

                // Check is there are enough unallocated bytes
                for (size_t j = 0; j < bytes; j++)
                {
                    if (bitmap[i + j])
                    {
                        spacefound = false;
                        break;
                    }
                }

                // If there are enough unallocate bytes
                if (spacefound)
                {
                    // Allocate those bytes
                    for (size_t j = 0; j < bytes; j++)
                    {
                        bitmap[i + j] = true;
                    }

                    // Return pointer to the first recently allocated byte
                    return (void*)&memory[i];
                }
            }
        }

        return invalidptr;
    }

    void free(void* const ptr)
    {
        size_t firstbyte = (uint8_t*)ptr - (uint8_t*)memstart;
        //size_t firstbyte = (size_t)ptr - (size_t)memstart;

        for (size_t i = 0; memory[i] != '\0'; i++)
        {
            bitmap[firstbyte + i] = false;
        }
    }

    void free(void* const ptr, const size_t bytes)
    {
        size_t firstbyte = (uint8_t*)ptr - (uint8_t*)memstart;
        //size_t firstbyte = (size_t)ptr - (size_t)memstart;

        for (size_t i = 0; i < bytes; i++)
        {
            bitmap[firstbyte + i] = false;
        }
    }
}
