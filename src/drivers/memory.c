#include <drivers/memory.h>
#include <c/string.h>
#include <drivers/io/terminal.h>
#include <kernel.h>

// Memory size constants
#define MEMORY_SIZE_BYTES (4 << 20)
#define MEMORY_SIZE_IN_SIZE_T (MEMORY_SIZE_BYTES / sizeof(size_t))
#define MEMORY_USED_BYTES_PER_ELEMENT (sizeof(size_t) * 8)
#define MEMORY_USED_SIZE (MEMORY_SIZE_BYTES / MEMORY_USED_BYTES_PER_ELEMENT)

// Dynamic memory is being allocated / deallocated DYNAMIC_SEGMENT_SIZE bytes at a time
#define DYNAMIC_SEGMENT_SIZE 0x10
#define DYNAMIC_SEGMENT_LIMIT (MEMORY_SIZE_BYTES / DYNAMIC_SEGMENT_SIZE)

size_t mapPage = 0;
size_t* pageTable = (size_t*)0;
size_t pageCount = 0;

size_t memory[MEMORY_SIZE_IN_SIZE_T]    __attribute__((aligned(0x1000)));
size_t memused[MEMORY_USED_SIZE]        __attribute__((aligned(0x1000)));
size_t memstartbyte = (size_t)&memory[0];

size_t dynsegbegin[DYNAMIC_SEGMENT_LIMIT];
size_t dynseglen[DYNAMIC_SEGMENT_LIMIT];

void mem_init(void)
{
    // Clear the memory
    for (size_t i = 0; i < MEMORY_SIZE_IN_SIZE_T; i++)
    {
        memory[i] = 0;
    }

    // Unallocate the memory
    for (size_t i = 0; i < MEMORY_USED_SIZE; i++)
    {
        memused[i] = 0;
    }

    // Clear the dynamic segment storage
    for (size_t i = 0; i < DYNAMIC_SEGMENT_LIMIT; i++)
    {
        dynsegbegin[i] = 0;
        dynseglen[i] = 0;
    }

    term_writeline("Memory initialized.", false);
}

bool _getused(const size_t relbyte)
{
    if (relbyte > MEMORY_SIZE_BYTES)
    {
        debug_print("memory.c | _getused() | Byte index is outside of memory boundaries!");
        return false;
    }

    size_t usedbyte = relbyte / MEMORY_USED_BYTES_PER_ELEMENT;
    size_t usedbit = relbyte % MEMORY_USED_BYTES_PER_ELEMENT;

    return (memused[usedbyte] & (1 << usedbit));
}

void _setused(const size_t relbyte, const bool isused)
{
    if (relbyte > MEMORY_SIZE_BYTES)
    {
        debug_print("memory.c | _setused() | Byte index is outside of memory boundaries!");
        return;
    }

    size_t usedbyte = relbyte / MEMORY_USED_BYTES_PER_ELEMENT;
    size_t usedbit = relbyte % MEMORY_USED_BYTES_PER_ELEMENT;

    if (isused)
    {
        memused[usedbyte] |= (1 << usedbit);
    }
    else
    {
        memused[usedbyte] &= ~(1 << usedbit);
    }
}

// Counts allocated bytes in memory
size_t mem_used(void)
{
    size_t count = 0;
    size_t tmpused = 0;

    for (size_t i = 0; i < MEMORY_USED_SIZE; i++)
    {
        if (memused[i])
        {
            tmpused = memused[i];
            
            while (tmpused > 0)
            {
                // Get the last bit
                count += (tmpused & 0x1);
                tmpused = tmpused >> 1;
            }
        }
    }

    return count;
}

// Counts unallocated bytes in memory
size_t mem_empty(void)
{
    size_t count = MEMORY_SIZE_BYTES;
    size_t tmpused = 0;

    for (size_t i = 0; i < MEMORY_USED_SIZE; i++)
    {
        if (memused[i])
        {
            tmpused = memused[i];
            
            while (tmpused > 0)
            {
                // Get the last bit
                count -= (tmpused & 0x1);
                tmpused = tmpused >> 1;
            }
        }
    }

    return count;
}

// Returns true if ptrbyte is within memory boundaries
bool _inmemory(const size_t ptrbyte)
{
    // Determines whether the pointer ptr is within memory boundaries
    return (ptrbyte >= memstartbyte && ptrbyte < memstartbyte + MEMORY_SIZE_BYTES);
}

// Returns true if ptr is within memory boundaries
bool _inmemoryptr(const void* const ptr)
{
    // Convert from pointer to size_t to make mathematical operations possible
    return _inmemory((size_t)ptr);
}

// Converts absolute byte address to a relative one
size_t _toreladdress(const size_t ptrbyte)
{
    return ptrbyte - memstartbyte;
}

// Converts poiter to a relative memory address
size_t _toreladdressptr(const void* const ptr)
{
    return (size_t)ptr - memstartbyte;
}

// Same as _statsegstore() except for dynamic memory segment
bool _dynsegstore(const size_t beginrel, const size_t length)
{
    for (size_t i = 0; i < DYNAMIC_SEGMENT_LIMIT; i++)
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
    debug_print("memory.c | _dynsegstore() | Couldn't store another dynamic memory segment!");
    return false;
}

// Finds a segments by its relative beginning address and returns its length
size_t _seglen(const size_t beginrel)
{
    // Scan through dynamic memory segments
    for (size_t i = 0; i < DYNAMIC_SEGMENT_LIMIT; i++)
    {
        // Find the segment with specified beginning address in segment storage
        if (dynsegbegin[i] == beginrel && dynseglen[i] > 0)
        {
            // Segment found, return its length
            return dynseglen[i];
        }
    }

    // Segment not found
    debug_print("memory.c | _seglen() | Memory segment couldn't be found!");
    return 0;
}

// Used by dynalloc() to avoid redundant code
// _alloc() shall NOT be accessed externally, it should be only used locally for the sake of safety
void* _alloc(const size_t length)
{
    // Empty space cannot be allocated
    if (length == 0)
    {
        debug_print("memory.c | _alloc() | Can't allocate an empty space!");
        return (void*)0;
    }

    // Find first unallocated byte
    for (size_t i = 0; i < (MEMORY_SIZE_BYTES - (length - 1)); i++)
    {
        // If byte is not allocated
        if (!_getused(i))
        {
            bool spacefound = true;

            // Check is there are enough unallocated bytes
            for (size_t j = 0; j < length; j++)
            {
                if (_getused(i + j))
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
                    _setused(i + j, true);
                }

                // Return pointer to the first recently allocated byte
                return (void*)(memstartbyte + i);
            }
        }
    }

    // There isn't enough space in the memory to allocate the desired amout of bytes
    debug_print("memory.c | _alloc() | Not enough space in the memory to perform the allocation!");
    return (void*)0;
}

// Internal function used for unallocating space in memory
// beginrel - relative address of the beginning of the segment to unallocate
// length - amout of bytes to unallocate
void _free(const size_t beginrel, const size_t length)
{
    for (size_t i = 0; i < length; i++)
    {
        _setused(beginrel + i, false);
    }
}

// Unallocates memory segment starting at address stored in ptr
void mem_free(const void* const ptr)
{
    if (!ptr)
    {
        debug_print("memory.c | mem_free() | Can't free a nullptr!");
        return;
    }

    // Convert from pointer to size_t to make mathematical operations possible
    size_t ptrbyte = (size_t)ptr;

    // If the pointer points outside the memory boundaries it should be ignored
    if (!_inmemory(ptrbyte))
    {
        debug_print("memory.c | mem_free() | Can't free a pointer outside of memory boundaries!");
        return;
    }

    // Calculate the relative address of the beginning of the segment
    size_t beginrel = _toreladdress(ptrbyte);

    // If there is no such static segment it will now assume it may be a dynamic segment
    for (size_t i = 0; i < DYNAMIC_SEGMENT_LIMIT; i++)
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

    debug_print("memory.c | mem_free() | Pointer couldn't be freed because it wasn't found as allocated!");
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

// Copy a specified amount of bytes from source to destination
void mem_copy(const void* const ptrsrc, const void* const ptrdst, const size_t length)
{
    uint8_t* byteptrsrc = (uint8_t*)ptrsrc;
    uint8_t* byteptrdst = (uint8_t*)ptrdst;

    for (size_t i = 0; i < length; i++)
    {
        byteptrdst[i] = byteptrsrc[i];
    }
}

// Finds the smallest dynamic segment size with at least segsize bytes
size_t _dynfindsize(const size_t segsize)
{
    size_t allocsize = DYNAMIC_SEGMENT_SIZE;
    
    while (allocsize < segsize)
        allocsize += DYNAMIC_SEGMENT_SIZE;

    return allocsize;
}

// Allocates dynamic memory segment
void* mem_dynalloc(const size_t initsize)
{
    size_t allocsize = _dynfindsize(initsize);
    void* allocptr = _alloc(allocsize);
    _dynsegstore(_toreladdressptr(allocptr), allocsize);
    return allocptr;
}

// Used when resizing dynamic memory segment
// Returns true if all the bytes within specified range are unallocated
bool _unallocated(const size_t beginrel, const size_t length)
{
    // Can't tell whether the memory is allocated or not if the
    // requested range is outside availible memory's boundaries
    if (beginrel + length >= MEMORY_SIZE_BYTES)
    {
        debug_print("memory.c | _unallocated() | Requested range is outside of memory boundaries!");
        return false;
    }

    for (size_t i = 0; i < length; i++)
    {
        // One of the bytes is already allocated
        if (_getused(beginrel + i))
            return false;
    }

    // All of the bytes within the requested range are unallocated
    return true;
}

// Resizes a dynamically allocated memory segment
void* mem_dynresize(void* const ptr, const size_t newsize)
{
    if (!_inmemoryptr(ptr))
    {
        debug_print("memory.c | mem_dynresize() | Pointer is outside of memory boundaries!");
        return (void*)0;
    }

    size_t beginrel = _toreladdressptr(ptr);
    size_t newallocsize = _dynfindsize(newsize);

    // Find the segment in dynamic segment storage
    for (size_t i = 0; i < DYNAMIC_SEGMENT_LIMIT; i++)
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
                if (_unallocated(beginrel + dynseglen[i], lendiff))
                {
                    // The segment will simply be prolonged and additional bytes will be allocated
                    for (size_t j = 0; j < lendiff; j++)
                    {
                        _setused(beginrel + dynseglen[i] + j, true);
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
                    size_t newbeginrel = _toreladdressptr(newptr);

                    // Store the new information about the segment
                    dynsegbegin[i] = newbeginrel;
                    dynseglen[i] = newallocsize;

                    // Return the new pointer because the segment has moved
                    return newptr;
                }
            }
        }
    }

    debug_print("memory.c | mem_dynresize() | Failed to resize a dynamic memory segment!");
    return (void*)0;
}

void* mem_set(void* ptr, int value, size_t num)
{
    unsigned char* dst = (unsigned char*)ptr;
    unsigned char cval = (unsigned char)value;

    while (num-- > 0)
    {
        *dst = cval;
        dst++;
    }

    return ptr;
}
