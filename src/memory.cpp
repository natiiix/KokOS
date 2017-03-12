#include "memory.hpp"

namespace mem
{
    const size_t MEMORY_SIZE = 1024*1024;
    uint8_t memory[MEMORY_SIZE];
    bool memused[MEMORY_SIZE];
    //void* memstart = (void*) &memory[0];
    size_t memstartbyte = (size_t)&memory[0];

    // Used for non-string memory allocations
    // Makes it possible to perform operations without specifying length of the segment
    const size_t STATIC_SEGMENT_MAX = 1024;
    size_t statsegbegin[STATIC_SEGMENT_MAX];
    size_t statseglen[STATIC_SEGMENT_MAX];

    // Dynamic memory is being allocated / deallocated DYN_SEGMENT_SIZE bytes at a time
    //const size_t DYN_SEGMENT_SIZE = 256;

    void init(void)
    {
        // Clear and unallocate the whole memory
        for (size_t i = 0; i < MEMORY_SIZE; i++)
        {
            memory[i] = 0;
            memused[i] = false;
        }

        // Clear the static segment storage
        for (size_t i = 0; i < STATIC_SEGMENT_MAX; i++)
        {
            statsegbegin[i] = 0;
            statseglen[i] = 0;
        }
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

    // Returns true if ptrbyte is within memory boundaries
    bool inmemory(size_t ptrbyte)
    {
        // Determines whether the pointer ptr is within memory boundaries
        return (ptrbyte >= memstartbyte && ptrbyte < memstartbyte + MEMORY_SIZE);
    }

    // Returns true if ptr is within memory boundaries
    bool inmemory(const void* const ptr)
    {
        // Convert from pointer to size_t to make mathematical operations possible
        return inmemory((size_t)ptr);
    }

    // Converts absolute byte address to a relative one
    size_t toreladdress(size_t ptrbyte)
    {
        return ptrbyte - memstartbyte;
    }

    // Stores information about a new static memory segment
    // begin - relative address of the first byte of the segment
    // length - length of the segment
    bool statsegstore(size_t begin, size_t length)
    {
        for (size_t i = 0; i < STATIC_SEGMENT_MAX; i++)
        {
            // Find empty spot in the static segment storage
            if (!statseglen[i])
            {
                statsegbegin[i] = begin;
                statseglen[i] = length;
                // New static segment has been stores successfully
                return true;
            }
        }

        // The static segment storage is already full
        // Couldn't store another segment
        return false;
    }

    // Extension for the alloc() function that allows not to store the newly create static segment
    // because it's unnecessary for strings as they should always end with the '\0' character
    // _alloc() shall NOT be accessed externally, it should be only used locally for the sake of safety
    void* _alloc(const size_t bytes, bool storestatseg = true)
    {
        // Empty space cannot be allocated
        if (bytes == 0)
        {
            return nullptr;
        }

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

                // If there are enough unallocated bytes
                if (spacefound)
                {
                    // If the allocated space is for a string, there's no need to store the static segment
                    // Add the space to the static segment storage
                    if (!storestatseg || statsegstore(i, bytes))
                    {
                        // Allocate those bytes
                        for (size_t j = 0; j < bytes; j++)
                        {
                            memused[i + j] = true;
                        }

                        // Return pointer to the first recently allocated byte
                        return (void*)&memory[i];
                    }
                    else
                    {
                        // There isn't enought space in the segment storage to store another segments
                        return nullptr;
                    }
                }
            }
        }

        // There isn't enough space in the memory to allocate the desired amout of bytes
        return nullptr;
    }

    // Allocates space in memory and returns a pointer to the beginning of recently allocated space
    // length - amount of bytes to allocate
    void* alloc(const size_t length)
    {
        return _alloc(length, true);
    }

    // Internal function used for unallocating space in memory
    // beginrel - relative address of the beginning of the segment to unallocate
    // length - amout of bytes to unallocate
    void _free(size_t beginrel, const size_t length)
    {
        for (size_t i = 0; i < length; i++)
        {
            memused[beginrel + i] = false;
        }
    }

    // Unallocates memory segment starting at address stored in ptr
    void free(const void* const ptr)
    {
        // Convert from pointer to size_t to make mathematical operations possible
        size_t ptrbyte = (size_t)ptr;

        // If the pointer points outside the memory boundaries it should be ignored
        if (!inmemory(ptrbyte))
            return;

        // Calculate the relative address of the beginning of the segment
        size_t beginrel = toreladdress(ptrbyte);

        // First the program assumes the memory to be freed is a static non-string segment
        for (size_t i = 0; i < STATIC_SEGMENT_MAX; i++)
        {
            // Find the segment with specified beginning address in segment storage
            if (statsegbegin[i] == beginrel)
            {
                _free(beginrel, statseglen[i]);

                statsegbegin[i] = 0;
                statseglen[i] = 0;

                // Static segment has successfully been found and deleted
                return;
            }
        }

        // Static segment with specified beginning address couldn't have been found
        // The program will assume it's a string and will attempt to delete it as such
        // The '\0' must be freed too for proper functionality, hence the +1
        _free(beginrel, str::len((char*)ptr) + 1);
    }

    // Unsafe local extension of copy() that allows copying bytes from outside of the memory boundaries
    void* _copy(const void* const ptrsrc, const size_t length, bool storestatseg = true)
    {
        uint8_t* ptrsrcbyte = (uint8_t*)ptrsrc;
        void* ptrdst = _alloc(length, storestatseg);
        uint8_t* ptrdstbyte = (uint8_t*)ptrdst;

        for (size_t i = 0; i < length; i++)
        {
            ptrdstbyte[i] = ptrsrcbyte[i];
        }

        return ptrdst;
    }

    void* copy(const void* const ptrsrc, const size_t length)
    {
        if (inmemory(ptrsrc))
        {
            return _copy(ptrsrc, length, true);
        }
        else
        {
            return nullptr;
        }
    }

    // This overload of copy() shall only by used for copying strings
    void* copy(const char* const str)
    {
        // The '\0' must be copied too for proper functionality
        // There is no need to store this segment in the static segment storage
        // because it's a string and therefore it's guaranteed to end with the '\0' character
        return _copy((void*)str, str::len(str) + 1, false);
    }

    /*void* dynalloc(const size_t initsize)
    {

    }*/
}
