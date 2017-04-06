#include <drivers/storage/fat.h>
#include <drivers/memory.h>
#include <drivers/storage/harddrive.h>
#include <c/string.h>
#include <drivers/io/terminal.h>

uint64_t clusterToSector(const uint8_t partIdx, const uint32_t clust)
{
    size_t fatBegin = partArray[partIdx].lbaBegin + partArray[partIdx].reservedSectors;
    size_t fatSectors = 2 * partArray[partIdx].fatSectors;
    return fatBegin + fatSectors + (partArray[partIdx].sectorsPerCluster * (clust - 2));
}

uint32_t* getClusterChain(const uint8_t partIdx, const uint32_t firstClust)
{
    size_t fatBegin = partArray[partIdx].lbaBegin + partArray[partIdx].reservedSectors;
    size_t clusterbytes = partArray[partIdx].sectorsPerCluster * 0x200;
    size_t clustercount = partArray[partIdx].fatSectors * 128;

    uint32_t* clusterChain = mem_dynalloc(0);
    size_t chainSize = 0;

    // Follow the cluster chain
    uint32_t currClust = firstClust;

    while (currClust < clustercount)
    {
        clusterChain = mem_dynresize(clusterChain, ++chainSize * 4);
        clusterChain[chainSize - 1] = currClust;

        size_t readsec = fatBegin + (currClust / clusterbytes);
        size_t fatentry = currClust % clusterbytes;

        struct FAT_TABLE* fat = (struct FAT_TABLE*)hddRead(hddArray[partArray[partIdx].hddIdx], readsec);
        currClust = fat->entries[fatentry];
        mem_free(fat);
    }

    clusterChain = mem_dynresize(clusterChain, ++chainSize * 4);
    clusterChain[chainSize - 1] = 0xFFFFFFFF;

    return clusterChain;
}

/*void rootDirDump(const uint8_t partIdx)
{
    uint32_t* clusterChain = getClusterChain(partIdx, partArray[partIdx].rootDirCluster);

    size_t chainIdx = 0;
    do
    {
        term_write("0x", false);
        term_writeline_convert(clusterChain[chainIdx], 16);
    }
    while (clusterChain[chainIdx++] < 0xFFFFFFFF);

    mem_free(clusterChain);
}*/

void listDirectory(const uint8_t partIdx, const uint32_t dirFirstClust)
{
    uint32_t* clusterChain = getClusterChain(partIdx, dirFirstClust);

    bool endOfDir = false;
    size_t chainIdx = 0;

    while (clusterChain[chainIdx] < 0xFFFFFFFF && !endOfDir)
    {
        uint64_t clusterBase = clusterToSector(partIdx, clusterChain[chainIdx]);

        for (size_t iSec = 0; iSec < partArray[partIdx].sectorsPerCluster && !endOfDir; iSec++)
        {
            struct DIR_SECTOR* dirsec = (struct DIR_SECTOR*)hddRead(hddArray[partArray[partIdx].hddIdx], clusterBase + iSec);

            for (size_t iEntry = 0; iEntry < 16 && !endOfDir; iEntry++)
            {
                uint8_t entryFirstByte = *(uint8_t*)&(dirsec->entries[iEntry]);

                if (entryFirstByte == DIR_ENTRY_END)
                {
                    endOfDir = true;
                }
                else if (entryFirstByte != DIR_ENTRY_UNUSED &&
                    dirsec->entries[iEntry].attrib != ATTRIB_VOLUME_ID &&
                    dirsec->entries[iEntry].attrib != ATTRIB_LONG_NAME)
                {
                    char filename[14];

                    for (size_t k = 0; k < 11; k++)
                    {
                        if (k > 7)
                        {
                            filename[k + 1] = dirsec->entries[iEntry].fileName[k];
                        }
                        else
                        {
                            filename[k] = dirsec->entries[iEntry].fileName[k];
                        }
                    }

                    filename[8] = '.';
                    filename[12] = '|';
                    filename[13] = '\0';

                    term_writeline(&filename[0], false);
                }
            }

            mem_free(dirsec);
        }
    }

    mem_free(clusterChain);
}
