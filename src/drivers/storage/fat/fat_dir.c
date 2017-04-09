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
    size_t entriesPerSector = partArray[partIdx].sectorsPerCluster * 0x80;
    size_t clusterCount = partArray[partIdx].fatSectors * 128;

    uint32_t* clusterChain = mem_dynalloc(0);
    size_t chainSize = 0;

    // Follow the cluster chain
    uint32_t currClust = firstClust;

    while (currClust < clusterCount)
    {
        clusterChain = mem_dynresize(clusterChain, ++chainSize * 4);
        clusterChain[chainSize - 1] = currClust;

        size_t readsec = fatBegin + (currClust / entriesPerSector);
        size_t fatentry = currClust % entriesPerSector;

        struct FAT_TABLE* fat = (struct FAT_TABLE*)hddRead(hddArray[partArray[partIdx].hddIdx], readsec);
        currClust = fat->entries[fatentry];
        mem_free(fat);
    }

    clusterChain = mem_dynresize(clusterChain, ++chainSize * 4);
    clusterChain[chainSize - 1] = CLUSTER_CHAIN_TERMINATOR;

    return clusterChain;
}

size_t findEmptyCluster(const uint8_t partIdx)
{
    size_t fatBegin = partArray[partIdx].lbaBegin + partArray[partIdx].reservedSectors;
    size_t entriesPerSector = partArray[partIdx].sectorsPerCluster * 0x80;

    size_t clustIdx = 2;

    size_t secIdx = fatBegin + (clustIdx / entriesPerSector);
    size_t entryIdx = clustIdx % entriesPerSector;

    while (secIdx < partArray[partIdx].fatSectors)
    {
        struct FAT_TABLE* fat = (struct FAT_TABLE*)hddRead(hddArray[partArray[partIdx].hddIdx], secIdx);

        while (entryIdx < 128)
        {
            if (!fat->entries[entryIdx])
            {
                return clustIdx;
            }
            entryIdx++;
        }
        entryIdx = 0;

        mem_free(fat);

        secIdx++;
    }

    return 0;
}

void fatWrite(const uint8_t partIdx, const uint32_t clustIdx, const uint32_t content)
{
    size_t fatBegin = partArray[partIdx].lbaBegin + partArray[partIdx].reservedSectors;
    size_t entriesPerSector = partArray[partIdx].sectorsPerCluster * 0x80;

    size_t secIdx = fatBegin + (clustIdx / entriesPerSector);
    size_t entryIdx = clustIdx % entriesPerSector;

    // Read the old FAT sector
    struct FAT_TABLE* fat = (struct FAT_TABLE*)hddRead(hddArray[partArray[partIdx].hddIdx], secIdx);

    // Change the content of a specified entry
    fat->entries[entryIdx] = content;

    // Write the new FAT sector with the modified entry
    hddWrite(hddArray[partArray[partIdx].hddIdx], secIdx, (uint8_t*)fat);

    mem_free(fat);
}

void prolongClusterChain(const uint8_t partIdx, const uint32_t firstClust)
{
    uint32_t* clusterChain = getClusterChain(partIdx, firstClust);

    // Get index of the last cluster in the chain
    size_t clustIdx = 1;
    while (clusterChain[clustIdx] < CLUSTER_CHAIN_TERMINATOR)
    {
        clustIdx++;
    }

    // Find an empty cluster to add to the chain
    size_t emptyCluster = findEmptyCluster(partIdx);

    fatWrite(partIdx, clusterChain[clustIdx - 1], emptyCluster);
    fatWrite(partIdx, emptyCluster, CLUSTER_CHAIN_TERMINATOR);

    mem_free(clusterChain);
}

char* fileNameToString(const char* const fileName)
{
    size_t namelen = 0;
    size_t extlen = 0;

    // Get length of file name
    for (size_t i = 0; i < 8; i++)
    {
        if (fileName[i] != ' ')
        {
            namelen = i + 1;
        }
    }

    // Get length of file name extension
    for (size_t i = 0; i < 3; i++)
    {
        if (fileName[8 + i] != ' ')
        {
            extlen = i + 1;
        }
    }

    // If there is an extension we need to separate it from file name using '.'
    // otherwise we just need space for the terminator character '\0'
    size_t totallen = namelen + extlen + (extlen ? 2 : 1);
    char* strName = mem_alloc(totallen);

    // Copy the file name and the extension to the output string
    for (size_t i = 0; i < namelen; i++)
    {
        strName[i] = fileName[i];
    }

    if (extlen)
    {
        strName[namelen] = '.';

        for (size_t i = 0; i < extlen; i++)
        {
            strName[namelen + 1 + i] = fileName[8 + i];
        }
    }

    strName[totallen - 1] = '\0';

    return strName;
}

void stringToFileName(const char* const strSrc, char* const fileNameDst)
{
    size_t offset = 0;

    for (size_t i = 0; i < 11; i++)
    {
        fileNameDst[i] = ' ';
    }

    for (size_t i = 0; strSrc[i]; i++)
    {
        if (strSrc[i] == '.')
        {
            offset = 8;
        }
        else
        {
            fileNameDst[offset++] = ctoupper(strSrc[i]);
        }
    }
}

bool attribCheck(const uint8_t entryAttrib, const uint8_t attribMask, const uint8_t attrib)
{
    return ((entryAttrib & attribMask) == attrib);
}

void listDirectory(const uint8_t partIdx, const uint32_t dirFirstClust)
{
    uint32_t* clusterChain = getClusterChain(partIdx, dirFirstClust);

    bool endOfDir = false;
    size_t chainIdx = 0;

    while (clusterChain[chainIdx] < CLUSTER_CHAIN_TERMINATOR && !endOfDir)
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
                    dirsec->entries[iEntry].attrib != FILE_ATTRIB_LONG_NAME &&
                    attribCheck(dirsec->entries[iEntry].attrib, FILE_ATTRIB_VOLUME_ID, 0))
                {
                    char* strName = fileNameToString(&dirsec->entries[iEntry].fileName[0]);
                                        
                    // Add a slash after the name if the entry is a directory to make it possible to differenciate them from ordinary files
                    if (attribCheck(dirsec->entries[iEntry].attrib, FILE_ATTRIB_DIRECTORY, FILE_ATTRIB_DIRECTORY))
                    {
                        term_write(strName, true);
                        term_writeline("/", false);
                    }
                    else
                    {
                        term_writeline(strName, true);
                    }
                }
            }

            mem_free(dirsec);
        }
    }

    mem_free(clusterChain);
}

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
            for (size_t iEntry = 0; iEntry < 0x10 && !endOfDir; iEntry++)
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
                    attribCheck(dirsec->entries[iEntry].attrib, attribMask, attrib)) // check attributes
                {
                    char* strName = fileNameToString(&dirsec->entries[iEntry].fileName[0]);
                    bool namesMatch = strcmp(name, strName);
                    mem_free(strName);

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

    // Entry not found, return nullptr
    return (struct DIR_ENTRY*)0;
}

uint32_t joinCluster(const uint16_t clusterHigh, const uint16_t clusterLow)
{
    return (((uint32_t)clusterHigh) << 16) | (uint32_t)clusterLow;
}

uint32_t resolvePath(const uint8_t partIdx, const uint32_t baseDir, const char* const path)
{
    size_t pathsize = strlen(path);

    char strsearch[16];
    size_t stridx = 0;

    // If the path begins with '/' it's absolute path, otherwise the path is relative to directory located at baseDir
    uint32_t searchCluster = (path[0] == '/' ? partArray[partIdx].rootDirCluster : baseDir);

    for (size_t i = 0; i < pathsize; i++)
    {
        if (path[i] != '/')
        {
            strsearch[stridx++] = ctoupper(path[i]);
        }

        if (path[i] == '/' || i == pathsize - 1)
        {
            if (stridx > 0)
            {
                strsearch[stridx] = '\0';

                struct DIR_ENTRY* direntry = findEntry(partIdx, searchCluster, &strsearch[0], FILE_ATTRIB_DIRECTORY, FILE_ATTRIB_DIRECTORY);

                if (direntry)
                {
                    searchCluster = joinCluster(direntry->clusterHigh, direntry->clusterLow);
                }
                else
                {
                    searchCluster = 0;
                }

                mem_free(direntry);

                if (searchCluster == 0)
                {
                    // '..' entry pointing to root directory points to cluster 0 (which isn't the actual root dir cluster) for some reason
                    // so it needs to be translated into the proper value, aside from that the root directory itself doesn't contain
                    // '.' and '..' so we have to emulate them and pretend they're there, both of them point to the root directory itself
                    if (strcmp(&strsearch[0], ".") ||
                        strcmp(&strsearch[0], ".."))
                    {
                        searchCluster = partArray[partIdx].rootDirCluster;
                    }
                    else
                    {
                        return 0;
                    }
                }
            }

            strsearch[stridx = 0] = '\0';
        }
    }

    return searchCluster;
}

struct FILE* getFile(const uint8_t partIdx, const uint32_t baseDir, const char* const path)
{
    size_t pathsize = strlen(path);
    char strsearch[16]; // contains file name only
    char* pathDir = mem_alloc(pathsize + 0x100); // contains the directory part of the path
    strcopy(path, pathDir);

    // Remove the file name from the directory path
    for (size_t i = 0; i < pathsize; i++)
    {
        if (pathDir[pathsize - 1 - i] == '/')
        {
            strcopy(&pathDir[pathsize - i], &strsearch[0]); // extract the file name into separate string
            pathDir[pathsize - 1 - i] = '\0';
            break;
        }
        // The path doesn't contain a directory part, it's just a file name
        else if (i == pathsize - 1)
        {
            strcopy(&pathDir[0], &strsearch[0]);
            pathDir[0] = '\0';
        }
    }

    strtoupper(&strsearch[0]);

    struct DIR_ENTRY* direntry = findEntry(partIdx, resolvePath(partIdx, baseDir, pathDir), &strsearch[0], FILE_ATTRIB_DIRECTORY, 0);

    mem_free(pathDir);
    
    if ((size_t)direntry == 0)
    {
        return (struct FILE*)0;
    }

    struct FILE* file = mem_alloc(sizeof(struct FILE));

    char* strName = fileNameToString(&direntry->fileName[0]);
    strcopy(strName, &file->name[0]);
    mem_free(strName);
    
    file->partIdx = partIdx;
    file->attrib = direntry->attrib;
    file->cluster = joinCluster(direntry->clusterHigh, direntry->clusterLow);
    file->size = direntry->fileSize;

    mem_free(direntry);

    return (file);
}

size_t _generateDirEntryIndex(const uint8_t partIdx, const size_t clusterIdx, const size_t sectorIdx, const size_t entryIdx)
{
    return (((clusterIdx * partArray[partIdx].sectorsPerCluster) + sectorIdx) * 0x10) + entryIdx;
}

size_t findUnusedDirEntry(const uint8_t partIdx, const uint32_t baseDir)
{
    // Get cluster chain
    uint32_t* clusterChain = getClusterChain(partIdx, baseDir);
    size_t chainIdx = 0;

    bool writeEndOfDir = false;
    size_t unusedDirEntryIdx = 0;    

    // Search through the cluster chain until the end of the directory is reached
    while (clusterChain[chainIdx] < CLUSTER_CHAIN_TERMINATOR)
    {
        // Convert cluster to sector for LBA addressing
        uint64_t clusterBase = clusterToSector(partIdx, clusterChain[chainIdx]);

        // Look through each sector within the cluster
        for (size_t iSec = 0; iSec < partArray[partIdx].sectorsPerCluster; iSec++)
        {
            // Read the sector from the drive
            struct DIR_SECTOR* dirsec = (struct DIR_SECTOR*)hddRead(hddArray[partArray[partIdx].hddIdx], clusterBase + iSec);

            // Look through all the entries in the sector
            for (size_t iEntry = 0; iEntry < 0x10; iEntry++)
            {
                // The previous entry has been marked as unused, but it was the last entry of the sector
                // so the end of directory entry must be set in the current iteration
                if (writeEndOfDir)
                {
                    *(uint8_t*)&(dirsec->entries[iEntry]) = DIR_ENTRY_END;
                    // Write it to the disk
                    hddWrite(hddArray[partArray[partIdx].hddIdx], clusterBase + iSec, (uint8_t*)dirsec);

                    return unusedDirEntryIdx;
                }

                // Get the first byte of the entry
                // Used to find unused entries and the end of the directory
                uint8_t entryFirstByte = *(uint8_t*)&(dirsec->entries[iEntry]);

                // Unused directory entry found
                if (entryFirstByte == DIR_ENTRY_UNUSED)
                {
                    mem_free(dirsec);
                    mem_free(clusterChain);

                    return _generateDirEntryIndex(partIdx, clusterChain[chainIdx], iSec, iEntry);
                }

                // End of directory reached before finding an unused entry
                if (entryFirstByte == DIR_ENTRY_END)
                {
                    if (iEntry + 1 < 0x10 || // not the last entry in the sector
                        iSec + 1 < partArray[partIdx].sectorsPerCluster || // not the last sector in cluster
                        clusterChain[chainIdx + 1] < CLUSTER_CHAIN_TERMINATOR) // not the last cluster in the cluster chain
                    {
                        // Mark this entry as unused
                        *(uint8_t*)&(dirsec->entries[iEntry]) = DIR_ENTRY_UNUSED;
                        // Write it to the disk
                        hddWrite(hddArray[partArray[partIdx].hddIdx], clusterBase + iSec, (uint8_t*)dirsec);

                        // The next entry will be marked as the end of the directory
                        writeEndOfDir = true;

                        unusedDirEntryIdx = _generateDirEntryIndex(partIdx, clusterChain[chainIdx], iSec, iEntry);
                    }
                    // This is the last entry of the last sector of the last cluster in the cluster chain that belongs to this directory
                    else
                    {
                        prolongClusterChain(partIdx, baseDir);
                        return findUnusedDirEntry(partIdx, baseDir);
                    }
                }
            }

            mem_free(dirsec);
        }
    }

    mem_free(clusterChain);

    // Should be unreachable
    return 0;
}

struct FILE* newFile(const uint8_t partIdx, const uint32_t baseDir, const char* const path)
{
    uint32_t targetDir = baseDir;
    size_t nameBeginIdx = 0;

    size_t lastSlashIdx = strlast(path, '/');
    if (lastSlashIdx != ~((size_t)0))
    {
        char* pathDir = mem_alloc(lastSlashIdx + 2);
        strcopy(path, pathDir);

        pathDir[lastSlashIdx + 1] = '\0';
        nameBeginIdx = lastSlashIdx + 1;

        targetDir = resolvePath(partIdx, baseDir, pathDir);

        mem_free(pathDir);
    }

    size_t unusedIdx = findUnusedDirEntry(partIdx, targetDir);

    size_t entryIdx = unusedIdx % 0x10;
    unusedIdx /= 0x10;
    size_t readSec = clusterToSector(partIdx, unusedIdx / partArray[partIdx].sectorsPerCluster) + unusedIdx % partArray[partIdx].sectorsPerCluster;

    // Find a cluster for the file
    size_t fileCluster = findEmptyCluster(partIdx);
    fatWrite(partIdx, fileCluster, CLUSTER_CHAIN_TERMINATOR);

    // Read old directory entries from the sector
    struct DIR_SECTOR* dirsec = (struct DIR_SECTOR*)hddRead(hddArray[partArray[partIdx].hddIdx], readSec);

    // Write the file information into the proper directory entry
    stringToFileName(&path[nameBeginIdx], &dirsec->entries[entryIdx].fileName[0]);
    dirsec->entries[entryIdx].attrib = FILE_ATTRIB_ARCHIVE; // I don't know why, but all the files seem to have this
    dirsec->entries[entryIdx].clusterHigh = (uint16_t)(fileCluster >> 0x10);
    dirsec->entries[entryIdx].clusterLow = (uint16_t)fileCluster;
    dirsec->entries[entryIdx].fileSize = 0;

    hddWrite(hddArray[partArray[partIdx].hddIdx], readSec, (uint8_t*)dirsec);

    mem_free(dirsec);

    // Generate the FILE structure
    struct FILE* file = mem_alloc(sizeof(struct FILE));

    strcopy(&path[nameBeginIdx], &file->name[0]);
    file->partIdx = partIdx;
    file->attrib = FILE_ATTRIB_ARCHIVE;
    file->cluster = fileCluster;
    file->size = 0;

    return file;
}
