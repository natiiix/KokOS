#include <drivers/memory.h>

#include <c/string.h>

/*// Memory size constants
const size_t MEMORY_SIZE_BYTES = 4 << 20;
const size_t MEMORY_SIZE_IN_SIZE_T = MEMORY_SIZE_BYTES / sizeof(size_t);
const size_t MEMORY_USED_BYTES_PER_ELEMENT = sizeof(size_t) * 8;
const size_t MEMORY_USED_SIZE = MEMORY_SIZE_BYTES / MEMORY_USED_BYTES_PER_ELEMENT;

// Used for non-string memory allocations
// Makes it possible to perform operations without specifying length of the segment
const size_t STATIC_SEGMENT_LIMIT = 1024;

// Dynamic memory is being allocated / deallocated DYNAMIC_SEGMENT_SIZE bytes at a time
const size_t DYNAMIC_SEGMENT_SIZE = 256;
const size_t DYNAMIC_SEGMENT_LIMIT = MEMORY_SIZE_BYTES / DYNAMIC_SEGMENT_SIZE;*/

// Memory size constants
#define MEMORY_SIZE_BYTES (4 << 20)
#define MEMORY_SIZE_IN_SIZE_T (MEMORY_SIZE_BYTES / sizeof(size_t))
#define MEMORY_USED_BYTES_PER_ELEMENT (sizeof(size_t) * 8)
#define MEMORY_USED_SIZE (MEMORY_SIZE_BYTES / MEMORY_USED_BYTES_PER_ELEMENT)

// Used for non-string memory allocations
// Makes it possible to perform operations without specifying length of the segment
#define STATIC_SEGMENT_LIMIT 1024

// Dynamic memory is being allocated / deallocated DYNAMIC_SEGMENT_SIZE bytes at a time
#define DYNAMIC_SEGMENT_SIZE 256
#define DYNAMIC_SEGMENT_LIMIT (MEMORY_SIZE_BYTES / DYNAMIC_SEGMENT_SIZE)

size_t mapPage = 0;
size_t* pageTable = (size_t*)0;
size_t pageCount = 0;

size_t memory[MEMORY_SIZE_IN_SIZE_T]    __attribute__((aligned(0x1000)));
size_t memused[MEMORY_USED_SIZE]        __attribute__((aligned(0x1000)));
size_t memstartbyte = (size_t)&memory[0];

size_t statsegbegin[STATIC_SEGMENT_LIMIT];
size_t statseglen[STATIC_SEGMENT_LIMIT];

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

    // Clear the static segment storage
    for (size_t i = 0; i < STATIC_SEGMENT_LIMIT; i++)
    {
        statsegbegin[i] = 0;
        statseglen[i] = 0;
    }

    // Clear the dynamic segment storage
    for (size_t i = 0; i < DYNAMIC_SEGMENT_LIMIT; i++)
    {
        dynsegbegin[i] = 0;
        dynseglen[i] = 0;
    }

    // The first page is already used by the VGA buffer
    mapPage = 1;
    // Load the page table address and the number of mapped pages
    pageTable = (size_t*)(*((size_t*)0xC07FFFFC));
    pageCount = *((size_t*)0xC07FFFF8);
}

size_t* getPageTable(void)
{
    return pageTable;
}

size_t getPageCount(void)
{
    return pageCount;
}

bool _getused(const size_t relbyte)
{
    if (relbyte > MEMORY_SIZE_BYTES)
        return false;

    size_t usedbyte = relbyte / MEMORY_USED_BYTES_PER_ELEMENT;
    size_t usedbit = relbyte % MEMORY_USED_BYTES_PER_ELEMENT;

    return (memused[usedbyte] & (1 << usedbit));
}

void _setused(const size_t relbyte, const bool isused)
{
    if (relbyte > MEMORY_SIZE_BYTES)
        return;

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
                count += (tmpused & 0b1);
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
                count -= (tmpused & 0b1);
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

// Stores information about a new static memory segment
// beginrel - relative address of the first byte of the segment
// length - length of the segment
bool _statsegstore(const size_t beginrel, const size_t length)
{
    for (size_t i = 0; i < STATIC_SEGMENT_LIMIT; i++)
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
    return false;
}

// Finds a segments by its relative beginning address and returns its length
size_t _seglen(const size_t beginrel)
{
    // Scan through static memory segments
    for (size_t i = 0; i < STATIC_SEGMENT_LIMIT; i++)
    {
        // Find the segment with specified beginning address in segment storage
        if (statsegbegin[i] == beginrel && statseglen[i] > 0)
        {
            // Segment found, return its length
            return statseglen[i];
        }
    }

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
    return (void*)0;
}

// Allocates space in memory and returns a pointer to the beginning of recently allocated space
// length - amount of bytes to allocate
void* alloc(const size_t length)
{
    void* allocptr = _alloc(length);
    size_t allocbyte = _toreladdressptr(allocptr);
    _statsegstore(allocbyte, length);
    return allocptr;
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
void free(const void* const ptr)
{
    // Convert from pointer to size_t to make mathematical operations possible
    size_t ptrbyte = (size_t)ptr;

    // If the pointer points outside the memory boundaries it should be ignored
    if (!_inmemory(ptrbyte))
        return;

    // Calculate the relative address of the beginning of the segment
    size_t beginrel = _toreladdress(ptrbyte);

    // First the program assumes the memory to be freed is a static non-string segment
    for (size_t i = 0; i < STATIC_SEGMENT_LIMIT; i++)
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
void* mem_copy(const void* const ptrsrc, const size_t length)
{
    // Copy the data
    void* dstptr = _copy(ptrsrc, length);
    // Get relative address of the copied data
    size_t dstbyte = _toreladdressptr(dstptr);
    // Store the static segment information
    _statsegstore(dstbyte, length);
    return dstptr;
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
void* dynalloc(const size_t initsize)
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
void* dynresize(void* const ptr, const size_t newsize)
{
    if (!_inmemoryptr(ptr))
        return (void*)0;

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

    return (void*)0;
}

void* memset(void* ptr, int value, size_t num)
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

void* phystovirt(const size_t physAddr)
{
    // Pages are 0x1000 aligned so the physical address must also be aligned so
    size_t pageOffset = physAddr & (size_t)0xFFF;
    size_t physAddrAligned = physAddr ^ pageOffset;
    size_t pageTableEntry = physAddrAligned | 0x3;
    
    // The amount of entries which can be possibly stored
    size_t entryCount = pageCount << 10;

    // Check if the physical address is already mapped
    for (size_t i = entryCount - 1; i < entryCount; i--)
    {
        // The physical address is already mapped
        if (pageTable[i] == pageTableEntry)
        {
            void* virtAddr = (void*)(0xC0000000 + (i << 12) + pageOffset);
            return virtAddr;
        }
    }

    // Map the physical address to a virtual address
    size_t pageTableIdx = entryCount - 1 - mapPage++;
    pageTable[pageTableIdx] = pageTableEntry;

    // Translate the input physical address to a virtual address
    void* virtAddr = (void*)(0xC0000000 + (pageTableIdx << 12) + pageOffset);
    return virtAddr;    
}

void* phystovirtptr(const void* const physAddr)
{
    return phystovirt((size_t)physAddr);
}

void* virttophys(const size_t virtAddr)
{
    // Pages are 0x1000 aligned
    size_t pageOffset = virtAddr & (size_t)0xFFF;
    size_t virtAddrAligned = virtAddr ^ pageOffset;

    size_t pageIdx = (virtAddrAligned - 0xC0000000) >> 12;
    return (void*)((pageTable[pageIdx] >> 12 << 12) + pageOffset);
}

void* virttophysptr(const void* const virtAddr)
{
    return virttophys((size_t)virtAddr);
}

uint32_t low32(const uint64_t value)
{
    return (uint32_t)value;
}

uint32_t high32(const uint64_t value)
{
    return (uint32_t)(value >> 32);
}
