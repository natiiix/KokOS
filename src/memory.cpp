#include "memory.hpp"

namespace mem
{
    const size_t MEMORY_SIZE = 1024*1024;
    uint8_t memory[MEMORY_SIZE];
    bool memused[MEMORY_SIZE];
    void* memstart = (void*) &memory[0];

    void init(void)
    {
        // Clean and unallocate the whole memory
        for (size_t i = 0; i < MEMORY_SIZE; i++)
        {
            memory[i] = 0;
            memused[i] = false;
        }
    }

    void* alloc(const size_t bytes)
    {
        if (bytes <= 0)
            return nullptr;

        // Find first unallocated byte
        for (size_t i = 0; i < (MEMORY_SIZE - (bytes - 1)); i++)
        {
            // If byte is not allocated
            if (!memused[i])
            {
                bool spacefound = true;

                // Check is there are enough unallocated bytes
                for (size_t j = 0; j < bytes; j++)
                {
                    if (memused[i + j])
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
                        memused[i + j] = true;
                    }

                    // Return pointer to the first recently allocated byte
                    return (void*)&memory[i];
                }
            }
        }

        return nullptr;
    }

    void free(const void* const ptr, const size_t bytes)
    {   
        size_t ptrbyte = (size_t)ptr;
        size_t memstartbyte = (size_t)memstart;

        if (ptrbyte < memstartbyte || ptrbyte > memstartbyte + MEMORY_SIZE)
            return;

        size_t firstbyte = ptrbyte - memstartbyte;

        for (size_t i = 0; i < bytes; i++)
        {
            memused[firstbyte + i] = false;
        }
    }

    void free(const void* const ptr)
    {
        // The '\0' must be freed too for proper functionality
        free(ptr, str::len((char*)ptr) + 1);
    }

    void* copy(const void* const ptrsrc, const size_t bytes)
    {
        uint8_t* ptrsrcbyte = (uint8_t*)ptrsrc;
        void* ptrdst = alloc(bytes);
        uint8_t* ptrdstbyte = (uint8_t*)ptrdst;

        for (size_t i = 0; i < bytes; i++)
        {
            ptrdstbyte[i] = ptrsrcbyte[i];
        }

        return ptrdst;
    }

    void* copy(const void* const ptrsrc)
    {
        // The '\0' must be copied too for proper functionality
        return copy(ptrsrc, str::len((char*)ptrsrc) + 1);
    }

    // Counts allocated bytes in memory
    size_t used(void)
    {
        size_t count = 0;

        for (size_t i = 0; i < MEMORY_SIZE; i++)
        {
            if (memused[i])
                count++;
        }

        return count;
    }

    // Counts unallocated bytes in memory
    size_t empty(void)
    {
        size_t count = 0;

        for (size_t i = 0; i < MEMORY_SIZE; i++)
        {
            if (!memused[i])
                count++;
        }

        return count;
    }
}
