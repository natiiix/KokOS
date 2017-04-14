#include <drivers/storage/fat.h>
#include <drivers/memory.h>
#include <drivers/storage/harddrive.h>
#include <c/string.h>
#include <drivers/io/terminal.h>
#include <kernel.h>

uint64_t clusterToSector(const uint8_t partIdx, const uint32_t clust)
{
    size_t fatBegin = partArray[partIdx].lbaBegin + partArray[partIdx].reservedSectors;
    size_t fatSectors = 2 * partArray[partIdx].fatSectors;
    return fatBegin + fatSectors + (partArray[partIdx].sectorsPerCluster * (clust - 2));
}

uint32_t* getClusterChain(const uint8_t partIdx, const uint32_t firstClust)
{
    // Calculate the sector offset of the first cluster
    size_t fatBegin = partArray[partIdx].lbaBegin + partArray[partIdx].reservedSectors;
    // Calculate the total number of clusters on this partition
    size_t clusterCount = partArray[partIdx].fatSectors * 0x80;

    // Allocate memory space for the cluster chain
    uint32_t* clusterChain = mem_dynalloc(0);
    size_t chainSize = 0;

    // Follow the cluster chain
    uint32_t currClust = firstClust;

    // The cluster chain terminator sign is higher than the total number of clusters
    while (currClust < clusterCount)
    {
        // Clusters in a chain should never point to a cluster 0 or cluster 1, because those are never used
        if (currClust < 2)
        {
            debug_print("fat_dir.c | getClusterChain() | Found an error in the cluster chain!");

            // Terminate the cluster chain so that it only contains valid clusters and return it
            clusterChain = mem_dynresize(clusterChain, ++chainSize * 4);
            clusterChain[chainSize - 1] = CLUSTER_CHAIN_TERMINATOR;

            return clusterChain;
        }

        // Add the cluster to the chain
        clusterChain = mem_dynresize(clusterChain, ++chainSize * 4);
        clusterChain[chainSize - 1] = currClust;

        // Calculate the sector that leads to the next cluster in the chain
        size_t readsec = fatBegin + (currClust / 0x80);
        size_t fatentry = currClust % 0x80;

        // Read the sector which holds the information about the next cluster in currently processed chain
        struct FAT_TABLE* fat = (struct FAT_TABLE*)hddRead(partArray[partIdx].hddIdx, readsec);
        currClust = fat->entries[fatentry];
        mem_free(fat);
    }

    // Save the terminator sign to the end of the cluster chain
    clusterChain = mem_dynresize(clusterChain, ++chainSize * 4);
    clusterChain[chainSize - 1] = CLUSTER_CHAIN_TERMINATOR;

    return clusterChain;
}

uint32_t findEmptyCluster(const uint8_t partIdx)
{
    // Calculate the sector offset
    size_t fatBegin = partArray[partIdx].lbaBegin + partArray[partIdx].reservedSectors;

    // Go through the FAT tables until an empty cluster is found
    for (size_t secIdx = 0; secIdx < partArray[partIdx].fatSectors; secIdx++)
    {
        // Read the FAT table from the disk
        struct FAT_TABLE* fat = (struct FAT_TABLE*)hddRead(partArray[partIdx].hddIdx, fatBegin + secIdx);

        // Each FAT table contains 128 entries
        for (size_t entryIdx = (secIdx ? 0 : 2); entryIdx < 0x80; entryIdx++)
        {
            // An empty cluster is marked by the value 0
            if (!fat->entries[entryIdx])
            {
                mem_free(fat);
                // Return the cluster index
                return (secIdx * 0x80) + entryIdx;
            }
        }       

        mem_free(fat);
    }

    // No empty cluster found, return 0
    debug_print("fat_dir.c | findEmptyCluster() | Couldn't find an empty cluster!");
    return 0;
}

void fatWrite(const uint8_t partIdx, const uint32_t clustIdx, const uint32_t content)
{
    // Calculate the sector offset
    size_t fatBegin = partArray[partIdx].lbaBegin + partArray[partIdx].reservedSectors;

    // There are 128 FAT entries in each sector
    size_t secIdx = fatBegin + (clustIdx / 0x80);
    size_t entryIdx = clustIdx % 0x80;

    // Read the old FAT sector
    struct FAT_TABLE* fat = (struct FAT_TABLE*)hddRead(partArray[partIdx].hddIdx, secIdx);

    // Change the content of a specified entry
    fat->entries[entryIdx] = content;

    // Write the new FAT sector with the modified entry
    hddWrite(partArray[partIdx].hddIdx, secIdx, (uint8_t*)fat);

    mem_free(fat);
}

bool prolongClusterChain(const uint8_t partIdx, const uint32_t firstClust, const size_t clustCount)
{
    if (!clustCount)
    {
        debug_print("fat_dir.c | prolongClusterChain() | It's redundant to prolong a cluster chain by 0 clusters!");
        return true; // It was technically successful despite not doing anything
    }

    // Get the cluster chain beginning at the specified cluster
    uint32_t* clusterChain = getClusterChain(partIdx, firstClust);

    // Get the last cluster in the chain
    uint32_t lastCluster = 0;
    for (size_t i = 0; clusterChain[i] < CLUSTER_CHAIN_TERMINATOR; i++)
    {
        lastCluster = clusterChain[i];
    }

    mem_free(clusterChain);

    for (size_t i = 0; i < clustCount; i++)
    {
        // Find an empty cluster to add to the chain
        size_t emptyCluster = findEmptyCluster(partIdx);

        // Valid clusters always have an index of 2 or higher
        if (emptyCluster < 2)
        {
            debug_print("fat_dir.c | prolongClusterChain() | Invalid empty cluster index!");
            mem_free(clusterChain);
            return false;
        }

        // Append the empty cluster to the end of the cluster chain
        fatWrite(partIdx, lastCluster, emptyCluster);

        lastCluster = emptyCluster;
    }

    // Mark the end of the chain by writing the terminator to the last entry in the chain
    fatWrite(partIdx, lastCluster, CLUSTER_CHAIN_TERMINATOR);

    return true;
}

bool shortenClusterChain(const uint8_t partIdx, const uint32_t firstClust, const size_t clustCount)
{
    if (!clustCount)
    {
        debug_print("fat_dir.c | shortenClusterChain() | It's redundant to shorten a cluster chain by 0 clusters!");
        return true; // It was technically successful despite not doing anything
    }

    // Get the cluster chain beginning at the specified cluster
    uint32_t* clusterChain = getClusterChain(partIdx, firstClust);

    // Get the length of the cluster chain
    uint32_t chainlen = 0;
    for (size_t i = 0; clusterChain[i] < CLUSTER_CHAIN_TERMINATOR; i++)
    {
        chainlen++;
    }

    // Make sure we're not trying to shorten the chain by too much
    if (clustCount >= chainlen)
    {
        debug_print("fat_dir.c | shortenClusterChain() | Can't shorten a cluster chain below 1 cluster!");
        mem_free(clusterChain);
        return false;
    }

    // Count down clusters starting from the end
    for (size_t i = 0; i < clustCount; i++)
    {
        // Set the cluster as unused
        fatWrite(partIdx, clusterChain[chainlen - i], 0);
    }

    // Mark the end of the chain by writing the terminator to the last entry in the chain
    fatWrite(partIdx, clusterChain[chainlen - clustCount], CLUSTER_CHAIN_TERMINATOR);

    mem_free(clusterChain);
    return true;
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

    // Copy the file name to the output string
    for (size_t i = 0; i < namelen; i++)
    {
        strName[i] = ctolower(fileName[i]);
    }

    // If there is an extension copy the extension as well
    if (extlen)
    {
        strName[namelen] = '.';

        for (size_t i = 0; i < extlen; i++)
        {
            strName[namelen + 1 + i] = ctolower(fileName[8 + i]);
        }
    }

    // Terminate the output string
    strName[totallen - 1] = '\0';

    return strName;
}

void stringToFileName(const char* const strSrc, char* const fileNameDst)
{
    // Writing starts at the index 0
    size_t offset = 0;

    // Clear the file name
    for (size_t i = 0; i < 11; i++)
    {
        fileNameDst[i] = ' ';
    }

    // Copy the file name from the input string
    for (size_t i = 0; strSrc[i]; i++)
    {
        // If '.' is reached jump to the extension part
        // The '.' character itself is not gonna be part of the name
        if (strSrc[i] == '.')
        {
            offset = 8;
        }
        else
        {
            // Copy the character from input string to the file name
            // Make sure all characters in the file name are uppercase
            fileNameDst[offset++] = ctoupper(strSrc[i]);
        }
    }
}

void stringToFileNameNoExt(const char* const strSrc, char* const fileNameDst)
{
    size_t i = 0;

    // We're assuming there is no extension, therefore we shouldn't write to the extension part
    while (i < 8 && strSrc[i])
    {
        // File name must be all in uppercase
        fileNameDst[i] = ctoupper(strSrc[i]);
        i++;
    }

    // Fill the rest of the file name with spaces
    while (i < 11)
    {
        fileNameDst[i++] = ' ';
    }
}

bool attribCheck(const uint8_t entryAttrib, const uint8_t attribMask, const uint8_t attrib)
{
    return ((entryAttrib & attribMask) == (attrib & attribMask));
}

void listDirectory(const uint8_t partIdx, const uint32_t dirFirstClust)
{
    // Get the cluster chain for the directory
    uint32_t* clusterChain = getClusterChain(partIdx, dirFirstClust);

    bool endOfDir = false;
    size_t chainIdx = 0;

    // Go through each cluster in the cluster chain
    while (clusterChain[chainIdx] < CLUSTER_CHAIN_TERMINATOR && !endOfDir)
    {
        // Calculate the first sector of this cluster
        uint64_t clusterBase = clusterToSector(partIdx, clusterChain[chainIdx]);

        // Go through each sector in the cluster
        for (size_t iSec = 0; iSec < partArray[partIdx].sectorsPerCluster && !endOfDir; iSec++)
        {
            // Read the directory sector from disk
            struct DIR_SECTOR* dirsec = (struct DIR_SECTOR*)hddRead(partArray[partIdx].hddIdx, clusterBase + iSec);

            // Go through each of the 16 entries in the directory sector
            for (size_t iEntry = 0; iEntry < 16 && !endOfDir; iEntry++)
            {
                // Read the first byte of the entry to determine if end of directory has been reached or if the entry is empty
                uint8_t entryFirstByte = *(uint8_t*)&(dirsec->entries[iEntry]);

                if (entryFirstByte == DIR_ENTRY_END)
                {
                    // End of directory reached, stop the directory listing
                    endOfDir = true;
                }
                else if (entryFirstByte != DIR_ENTRY_UNUSED && // don't list unused entries
                    dirsec->entries[iEntry].attrib != FILE_ATTRIB_LONG_NAME && // don't list long name entries
                    attribCheck(dirsec->entries[iEntry].attrib, FILE_ATTRIB_VOLUME_ID, 0)) // don't list the Volume ID itself
                {
                    // Get the cstring representation of the file name
                    char* strName = fileNameToString(&dirsec->entries[iEntry].fileName[0]);
                                        
                    // Add a slash after the name if the entry is a directory to make it possible to differenciate them from ordinary files
                    if (attribCheck(dirsec->entries[iEntry].attrib, FILE_ATTRIB_DIRECTORY, FILE_ATTRIB_DIRECTORY))
                    {
                        // Directory
                        term_write(strName, true);
                        term_writeline("/", false);
                    }
                    else
                    {
                        // File
                        term_write(strName, true);

                        // Display the size of the file in bytes
                        term_write(" - ", false);
                        term_write_convert(dirsec->entries[iEntry].fileSize, 10);
                        term_writeline(" Bytes", false);
                    }
                }
            }

            mem_free(dirsec);
        }
    }

    mem_free(clusterChain);
}

uint32_t joinCluster(const uint16_t clusterHigh, const uint16_t clusterLow)
{
    return (((uint32_t)clusterHigh) << 16) | (uint32_t)clusterLow;
}

uint32_t resolvePath(const uint8_t partIdx, const uint32_t baseDir, const char* const path)
{
    // Get the length of the path string
    size_t pathsize = strlen(path);

    char strsearch[16];
    size_t stridx = 0;

    // If the path begins with '/' it's absolute path, otherwise the path is relative to directory located at baseDir
    uint32_t searchCluster = (path[0] == '/' ? partArray[partIdx].rootDirCluster : baseDir);

    for (size_t i = 0; i < pathsize; i++)
    {
        if (path[i] != '/')
        {
            // Copy each directory name in the path in lowercase
            strsearch[stridx++] = ctolower(path[i]);
        }

        // End of a directory name
        if (path[i] == '/' || i == pathsize - 1)
        {
            if (stridx > 0)
            {
                strsearch[stridx] = '\0';

                struct DIR_ENTRY* direntry = findEntry(partIdx, searchCluster, &strsearch[0], FILE_ATTRIB_DIRECTORY, FILE_ATTRIB_DIRECTORY);

                // The entry has been found
                if (direntry)
                {
                    searchCluster = joinCluster(direntry->clusterHigh, direntry->clusterLow);
                    mem_free(direntry);
                }
                // No such entry could be found
                else
                {
                    searchCluster = 0;
                }

                // Invalid cluster occurred
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
                        debug_print("fat_dir.c | resolvePath() | Unable to resolve the path!");
                        return 0;
                    }
                }
            }

            // Terminate the search name string
            strsearch[stridx = 0] = '\0';
        }
    }

    return searchCluster;
}

size_t generateDirEntryIndex(const uint8_t partIdx, const size_t clusterIdx, const size_t sectorIdx, const size_t entryIdx)
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
            struct DIR_SECTOR* dirsec = (struct DIR_SECTOR*)hddRead(partArray[partIdx].hddIdx, clusterBase + iSec);

            // Look through all the entries in the sector
            for (size_t iEntry = 0; iEntry < 0x10; iEntry++)
            {
                // The previous entry has been marked as unused, but it was the last entry of the sector
                // so the end of directory entry must be set in the current iteration
                if (writeEndOfDir)
                {
                    *(uint8_t*)&(dirsec->entries[iEntry]) = DIR_ENTRY_END;
                    // Write it to the disk
                    hddWrite(partArray[partIdx].hddIdx, clusterBase + iSec, (uint8_t*)dirsec);

                    mem_free(dirsec);
                    mem_free(clusterChain);

                    return unusedDirEntryIdx;
                }

                // Get the first byte of the entry
                // Used to find unused entries and the end of the directory
                uint8_t entryFirstByte = *(uint8_t*)&(dirsec->entries[iEntry]);

                // Unused directory entry found
                if (entryFirstByte == DIR_ENTRY_UNUSED)
                {
                    unusedDirEntryIdx = generateDirEntryIndex(partIdx, clusterChain[chainIdx], iSec, iEntry);

                    mem_free(dirsec);
                    mem_free(clusterChain);

                    return unusedDirEntryIdx;
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
                        hddWrite(partArray[partIdx].hddIdx, clusterBase + iSec, (uint8_t*)dirsec);

                        // The next entry will be marked as the end of the directory
                        writeEndOfDir = true;

                        unusedDirEntryIdx = generateDirEntryIndex(partIdx, clusterChain[chainIdx], iSec, iEntry);
                    }
                    // This is the last entry of the last sector of the last cluster in the cluster chain that belongs to this directory
                    else
                    {
                        mem_free(dirsec);
                        mem_free(clusterChain);

                        if (prolongClusterChain(partIdx, baseDir, 1))
                        {
                            return findUnusedDirEntry(partIdx, baseDir);
                        }
                        // Failed to prolong the cluster chain
                        else
                        {
                            debug_print("fat_dir.c | findUnusedDirEntry() | Can't find an unused directory entry due to a failed attempt to prolong the cluster chain!");
                            // Return 0 as a sign of failure
                            return 0;
                        }
                    }
                }
            }

            mem_free(dirsec);
        }
    }

    mem_free(clusterChain);

    // Should be unreachable
    debug_print("fat_dir.c | findUnusedDirEntry() | Couldn't find an unused directory entry!");
    return 0;
}

bool extractPath(const uint8_t partIdx, const uint32_t baseDir, const char* const pathFull, uint32_t* targetDir, char** const pathNamePtr)
{
	size_t pathlen = strlen(pathFull);
    // Find the last slash in the path
    // The last slash separates the path from the entry name
    size_t lastSlashIdx = strlast(pathFull, '/');
	
	// If there is no slash the whole path is a name
	// This value is overriden if a slash is found
	size_t nameBeginIdx = 0;
	
	// There is no slash in the path, the whole path is just a name
    if (lastSlashIdx == ~((size_t)0))
    {
		*targetDir = baseDir;
    }
	// The path contains not only the name, but also a directory path
	else
	{
		// Allocate space for the directory path
		char* pathDir = mem_alloc(lastSlashIdx + 2);
		
		// Copy the directory path
        mem_copy(pathFull, pathDir, lastSlashIdx + 1);
		// Terminate the directory path string
        pathDir[lastSlashIdx + 1] = '\0';
		
		// Name begins after the last slash
        nameBeginIdx = lastSlashIdx + 1;

		// Resolve the directory path
        *targetDir = resolvePath(partIdx, baseDir, pathDir);

        mem_free(pathDir);
	}
	
	size_t namelen = pathlen - nameBeginIdx;
	// Name has a non-zero length, it can be worked with
	if (namelen)
	{
		// Allocate space for the name string
		char* pathName = mem_alloc(namelen + 1);
		// Copy the name from the original path to the name string
        for (size_t i = 0; i < namelen; i++)
        {
            pathName[i] = ctolower(pathFull[nameBeginIdx + i]);
        }
		// Terminate the name string
		pathName[namelen] = '\0';
		// Set the name string pointer
		*pathNamePtr = pathName;

        return true;
	}
	// The name string must not be empty, set the name string pointer to nullptr
	else
	{
        debug_print("fat_dir.c | extractPath() | Path contains an empty entry name!");
		*pathNamePtr = (char*)0;
        
        return false;
	}
}

size_t bytesToClusterCount(const uint8_t partIdx, const uint32_t sizeInBytes)
{
    size_t clusterSize = 0x200 * partArray[partIdx].sectorsPerCluster;
    return (sizeInBytes / clusterSize) + !!(sizeInBytes % clusterSize);
}
