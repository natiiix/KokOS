#include <drivers/storage/fat.h>
#include <drivers/memory.h>
#include <drivers/storage/harddrive.h>
#include <c/string.h>
#include <drivers/io/terminal.h>

void rootDirDump(const uint8_t partIdx)
{
    size_t fatBegin = partArray[partIdx].lbaBegin + partArray[partIdx].reservedSectors;
    size_t clusterbytes = partArray[partIdx].sectorsPerCluster * 0x200;

    // Follow the cluster chain for root directory
    uint32_t rootClust = partArray[partIdx].rootDirCluster;
    term_writeline_convert(rootClust, 16);

    while (rootClust != 0xFFFFFFFF)
    {
        size_t readsec = fatBegin + (rootClust / clusterbytes);
        size_t fatentry = rootClust % clusterbytes;

        struct FAT_TABLE* fat = (struct FAT_TABLE*)hddRead(hddArray[partArray[partIdx].hddIdx], readsec);

        rootClust = fat->entries[fatentry];

        term_writeline_convert(rootClust, 16);

        mem_free(fat);
    }
}
