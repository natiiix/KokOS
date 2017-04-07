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
    clusterChain[chainSize - 1] = CLUSTER_CHAIN_TERMINATOR;

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

/*void listDirectory(const uint8_t partIdx, const uint32_t dirFirstClust)
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
}*/

struct DIR_ENTRY* findEntry(const uint8_t partIdx, const uint32_t baseDirCluster, const char* const name, const uint8_t attribMask, const uint8_t attrib)
{
    // Get cluster chain
    uint32_t* clusterChain = getClusterChain(partIdx, baseDirCluster);

    bool endOfDir = false;
    size_t chainIdx = 0;

    // Search through the cluster chain until the end of the directory is reached
    while (clusterChain[chainIdx] < CLUSTER_CHAIN_TERMINATOR && !endOfDir)
    {
        // Convert cluster to sector for LBA addressing
        uint64_t clusterBase = clusterToSector(partIdx, clusterChain[chainIdx]);

        // Look through each sector within the cluster
        for (size_t iSec = 0; iSec < partArray[partIdx].sectorsPerCluster && !endOfDir; iSec++)
        {
            // Read the sector from the drive
            struct DIR_SECTOR* dirsec = (struct DIR_SECTOR*)hddRead(hddArray[partArray[partIdx].hddIdx], clusterBase + iSec);

            // Look through all the entries in the sector
            for (size_t iEntry = 0; iEntry < 16 && !endOfDir; iEntry++)
            {
                // Get the first byte of the entry
                // Used to find unused entries and the end of the directory
                uint8_t entryFirstByte = *(uint8_t*)&(dirsec->entries[iEntry]);

                // End of directory reached
                if (entryFirstByte == DIR_ENTRY_END)
                {
                    endOfDir = true;
                }
                else if (entryFirstByte != DIR_ENTRY_UNUSED && // mustn't be an unused entry
                    dirsec->entries[iEntry].attrib != FILE_ATTRIB_LONG_NAME && // mustn't be a long name entry
                    ((dirsec->entries[iEntry].attrib & attribMask) == attrib)) // check attributes
                {
                    bool namesMatch = true;
                    bool nameEnd = false; // end of the name reached (entry name must contain only spaces after the terminator)

                    // Check if the name in the entry matches the name requested
                    for (size_t i = 0; i < 11; i++)
                    {
                        if (name[i] == '\0')
                        {
                            nameEnd = true;
                        }

                        if ((nameEnd && dirsec->entries[iEntry].fileName[i] != ' ') ||
                            (!nameEnd && dirsec->entries[iEntry].fileName[i] != name[i]))
                        {
                            // Names don't match, don't check further
                            namesMatch = false;
                            break;
                        }
                    }

                    // Names match, return the directory entry
                    if (namesMatch)
                    {
                        struct DIR_ENTRY* direntry = mem_alloc(sizeof(struct DIR_ENTRY));
                        mem_copy(&dirsec->entries[iEntry], direntry, sizeof(struct DIR_ENTRY));

                        mem_free(dirsec);
                        mem_free(clusterChain);

                        return direntry;
                    }
                }
            }

            mem_free(dirsec);
        }
    }

    mem_free(clusterChain);

    // Entry not found, return 0 (it's safe, because clusters 0 and 1 are never really used)
    return 0;
}

struct FILE getFile(const uint8_t partIdx, const char* const path)
{
    struct FILE file;
    mem_set(&file, 0, sizeof(struct FILE));

    size_t pathsize = strlen(path);

    char strsearch[16];
    size_t stridx = 0;

    uint32_t searchCluster = partArray[partIdx].rootDirCluster;

    for (size_t i = 0; i < pathsize; i++)
    {
        if (path[i] == '/' && stridx > 0)
        {
            strsearch[stridx] = '\0';

            struct DIR_ENTRY* direntry = findEntry(partIdx, searchCluster, &strsearch[0], FILE_ATTRIB_DIRECTORY, FILE_ATTRIB_DIRECTORY);
            searchCluster = (((uint32_t)direntry->clusterHigh) << 16) | direntry->clusterLow;

            if (searchCluster == 0)
            {
                return file;
            }

            strsearch[stridx = 0] = '\0';
        }
        else
        {
            strsearch[stridx++] = path[i];
        }
    }

    return file;
}
