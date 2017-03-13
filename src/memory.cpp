#include "memory.hpp"

namespace mem
{
    const size_t MEMORY_SIZE = 1024*1024;
    uint8_t memory[MEMORY_SIZE];
    bool memused[MEMORY_SIZE];
    size_t memstartbyte = (size_t)&memory[0];

    // Used for non-string memory allocations
    // Makes it possible to perform operations without specifying length of the segment
    const size_t STATIC_SEGMENT_MAX = 1024;
    size_t statsegbegin[STATIC_SEGMENT_MAX];
    size_t statseglen[STATIC_SEGMENT_MAX];

    // Dynamic memory is being allocated / deallocated DYNAMIC_SEGMENT_SIZE bytes at a time
    const size_t DYNAMIC_SEGMENT_SIZE = 256;
    const size_t DYNAMIC_SEGMENT_MAX = 1024;
    size_t dynsegbegin[STATIC_SEGMENT_MAX];
    size_t dynseglen[STATIC_SEGMENT_MAX];

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

        // Clear the dynamic segment storage
        for (size_t i = 0; i < DYNAMIC_SEGMENT_MAX; i++)
        {
            dynsegbegin[i] = 0;
            dynseglen[i] = 0;
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
    bool inmemory(const size_t ptrbyte)
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
    size_t toreladdress(const size_t ptrbyte)
    {
        return ptrbyte - memstartbyte;
    }

    // Converts poiter to a relative memory address
    size_t toreladdress(const void* const ptr)
    {
        return (size_t)ptr - memstartbyte;
    }

    // Stores information about a new static memory segment
    // beginrel - relative address of the first byte of the segment
    // length - length of the segment
    bool statsegstore(const size_t beginrel, const size_t length)
    {
        for (size_t i = 0; i < STATIC_SEGMENT_MAX; i++)
        {
            // Find empty spot in the static segment storage
            if (!statseglen[i])
            {
                statsegbegin[i] = beginrel;
                statseglen[i] = length;
                // New static segment has been stored successfully
                return true;
            }
        }

        // The static segment storage is already full
        // Couldn't store another segment
        return false;
    }

    // Same as statsegstore() except for dynamic memory segment
    bool dynsegstore(const size_t beginrel, const size_t length)
    {
        for (size_t i = 0; i < DYNAMIC_SEGMENT_MAX; i++)
        {
            // Find empty spot in the static segment storage
            if (!dynseglen[i])
            {
                dynsegbegin[i] = beginrel;
                dynseglen[i] = length;
                // New dynamic segment has been stored successfully
                return true;
            }
        }

        // The dynamic segment storage is already full
        // Couldn't store another segment
        return false;
    }

    // Finds a segments by its relative beginning address and returns its length
    size_t seglen(const size_t beginrel)
    {
        // Scan through static memory segments
        for (size_t i = 0; i < STATIC_SEGMENT_MAX; i++)
        {
            // Find the segment with specified beginning address in segment storage
            if (statsegbegin[i] == beginrel && statseglen[i] > 0)
            {
                // Segment found, return its length
                return statseglen[i];
            }
        }

        // Scan through dynamic memory segments
        for (size_t i = 0; i < DYNAMIC_SEGMENT_MAX; i++)
        {
            // Find the segment with specified beginning address in segment storage
            if (dynsegbegin[i] == beginrel && dynseglen[i] > 0)
            {
                // Segment found, return its length
                return dynseglen[i];
            }
        }

        // Segment not found
        return 0;
    }

    // Extension for the alloc() function that allows not to store the newly create static segment
    // because it's unnecessary for strings as they should always end with the '\0' character
    // Also used by dynalloc() to avoid redundant code
    // _alloc() shall NOT be accessed externally, it should be only used locally for the sake of safety
    void* _alloc(const size_t length)
    {
        // Empty space cannot be allocated
        if (length == 0)
        {
            return nullptr;
        }

        // Find first unallocated byte
        for (size_t i = 0; i < (MEMORY_SIZE - (length - 1)); i++)
        {
            // If byte is not allocated
            if (!memused[i])
            {
                bool spacefound = true;

                // Check is there are enough unallocated bytes
                for (size_t j = 0; j < length; j++)
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
                    // Allocate those bytes
                    for (size_t j = 0; j < length; j++)
                    {
                        memused[i + j] = true;
                    }

                    // Return pointer to the first recently allocated byte
                    return (void*)(memstartbyte + i);
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
        void* allocptr = _alloc(length);
        size_t allocbyte = toreladdress(allocptr);
        statsegstore(allocbyte, length);
        return allocptr;
    }

    // Internal function used for unallocating space in memory
    // beginrel - relative address of the beginning of the segment to unallocate
    // length - amout of bytes to unallocate
    void _free(const size_t beginrel, const size_t length)
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
            if (statsegbegin[i] == beginrel && statseglen[i] > 0)
            {
                _free(beginrel, statseglen[i]);

                statsegbegin[i] = 0;
                statseglen[i] = 0;

                // Static segment has successfully been found and deleted
                return;
            }
        }

        // If there is no such static segment it will now assume it may be a dynamic segment
        for (size_t i = 0; i < DYNAMIC_SEGMENT_MAX; i++)
        {
            // Find the segment with specified beginning address in segment storage
            if (dynsegbegin[i] == beginrel && dynseglen[i] > 0)
            {
                _free(beginrel, dynseglen[i]);

                dynsegbegin[i] = 0;
                dynseglen[i] = 0;

                // Dynamic segment has successfully been found and deleted
                return;
            }
        }

        // No static / dynamic segment was found, the program will assume it must be a string
        // The '\0' must be freed too for proper functionality, hence the + 1
        _free(beginrel, str::len((char*)ptr) + 1);
    }

    // Unsafe local extension of copy() that allows copying bytes from outside of the memory boundaries
    void* _copy(const void* const ptrsrc, const size_t length)
    {
        uint8_t* ptrsrcbyte = (uint8_t*)ptrsrc;
        void* ptrdst = _alloc(length);
        uint8_t* ptrdstbyte = (uint8_t*)ptrdst;

        for (size_t i = 0; i < length; i++)
        {
            ptrdstbyte[i] = ptrsrcbyte[i];
        }

        return ptrdst;
    }

    // Data are always copied to a static segment
    void* copy(const void* const ptrsrc, const size_t length)
    {
        // Copy the data
        void* dstptr = _copy(ptrsrc, length);
        // Get relative address of the copied data
        size_t dstbyte = toreladdress(dstptr);
        // Store the static segment information
        statsegstore(dstbyte, length);
        return dstptr;
    }

    // This overload of copy() can only be used for copying previously stored memory segments
    void* copy(const void* const ptrsrc)
    {
        if (inmemory(ptrsrc))
        {
            return copy(ptrsrc, seglen(toreladdress(ptrsrc)));
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
        return _copy((void*)str, str::len(str) + 1);
    }

    // Finds the smallest dynamic segment size with at least segsize bytes
    size_t dynfindsize(const size_t segsize)
    {
        size_t allocsize = DYNAMIC_SEGMENT_SIZE;
        
        while (allocsize < segsize)
            allocsize += DYNAMIC_SEGMENT_SIZE;

        return allocsize;
    }

    // Allocates dynamic memory segment
    void* dynalloc(const size_t initsize)
    {
        size_t allocsize = dynfindsize(initsize);
        void* allocptr = _alloc(allocsize);
        dynsegstore(toreladdress(allocptr), allocsize);
        return allocptr;
    }

    // Used when resizing dynamic memory segment
    // Returns true if all the bytes within specified range are unallocated
    bool unallocated(const size_t beginrel, const size_t length)
    {
        // Can't tell whether the memory is allocated or not if the
        // requested range is outside availible memory's boundaries
        if (beginrel + length >= MEMORY_SIZE)
        {
            return false;
        }

        for (size_t i = 0; i < length; i++)
        {
            // One of the bytes is already allocated
            if (memused[beginrel + i])
                return false;
        }

        // All of the bytes within the requested range are unallocated
        return true;
    }

    // Resizes a dynamically allocated memory segment
    void* dynresize(void* const ptr, const size_t newsize)
    {
        if (!inmemory(ptr))
            return nullptr;

        size_t beginrel = toreladdress(ptr);
        size_t newallocsize = dynfindsize(newsize);

        // Find the segment in dynamic segment storage
        for (size_t i = 0; i < DYNAMIC_SEGMENT_MAX; i++)
        {
            // Segment found
            if (dynsegbegin[i] == beginrel)
            {
                // If the segment already has the size it's supposed to resize to
                if (dynseglen[i] == newallocsize)
                {
                    // The old pointer can be returned without making any changes
                    return ptr;
                }
                // If the allocated segment is longer than it's supposed to be
                else if (dynseglen[i] > newallocsize)
                {
                    // Calculate the length difference between the old segment length and the new one
                    size_t lendiff = dynseglen[i] - newallocsize;
                    // Unallocate the no longer needed part of the dynamic segment
                    _free(beginrel + dynseglen[i] - lendiff, lendiff);
                    // Store the new segment length
                    dynseglen[i] = newallocsize;
                    // The original pointer is returned because the segment hasn't moved
                    return ptr;
                }
                // The resized segment is supposed to be longer than it currently is
                else if (dynseglen[i] < newallocsize)
                {
                    size_t lendiff = newallocsize - dynseglen[i];

                    // If there are enough unallocated bytes after the already allocated segment
                    if (unallocated(beginrel + dynseglen[i], lendiff))
                    {
                        // The segment will simply be prolonged and additional bytes will be allocated
                        for (size_t j = 0; j < lendiff; j++)
                        {
                            memused[beginrel + dynseglen[i] + j] = true;
                        }

                        // Store the new segment length
                        dynseglen[i] = newallocsize;

                        // The original pointer can be returned because the segment hasn't moved
                        return ptr;
                    }
                    else
                    {
                        // Copies data from the old segment to a new place in the memory
                        // that has enough unallocated bytes to fit the new size of the segment
                        void* newptr = _copy(ptr, newallocsize);
                        // Unallocate bytes that used to belong to this segment
                        _free(beginrel, dynseglen[i]);

                        // Get the new relative memory address
                        size_t newbeginrel = toreladdress(newptr);

                        // Store the new information about the segment
                        dynsegbegin[i] = newbeginrel;
                        dynseglen[i] = newallocsize;

                        // Return the new pointer because the segment has moved
                        return newptr;
                    }
                }
            }
        }

        return nullptr;
    }
}
