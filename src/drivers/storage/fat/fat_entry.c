#include <drivers/storage/fat.h>
#include <drivers/memory.h>
#include <drivers/storage/harddrive.h>
#include <c/string.h>
#include <drivers/io/terminal.h>

struct DIR_ENTRY* findEntry(const uint8_t partIdx, const uint32_t baseDirCluster, const char* const name, const uint8_t attribMask, const uint8_t attrib)
{
    // Get cluster chain
    uint32_t* clusterChain = getClusterChain(partIdx, baseDirCluster);

    bool endOfDir = false;

    // Search through the cluster chain until the end of the directory is reached
    for (size_t chainIdx = 0; clusterChain[chainIdx] < CLUSTER_CHAIN_TERMINATOR && !endOfDir; chainIdx++)
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

uint8_t* readFile(const struct FILE* const file)
{
    // It's impossible to read the contents of an empty file
    if (file->size == 0)
    {
        return (uint8_t*)0;
    }

    uint32_t* clusterChain = getClusterChain(file->partIdx, file->cluster);
    uint8_t* fileContent = mem_alloc(file->size); // used to store the contents of the file
    size_t contentIdx = 0;
    bool fileEnd = false;

    // Reach all clusters that belong to this file
    for (size_t i = 0; clusterChain[i] < CLUSTER_CHAIN_TERMINATOR && !fileEnd; i++)
    {
        uint64_t clusterBase = clusterToSector(file->partIdx, clusterChain[i]);

        // Go through all the sectors in the cluster
        for (size_t j = 0; j < partArray[file->partIdx].sectorsPerCluster && !fileEnd; j++)
        {
            uint8_t* data = hddRead(hddArray[partArray[file->partIdx].hddIdx], clusterBase + j);

            // If the file does not occupy the whole sector copy only as much as necessary
            // This also means the end of file has been reached
            size_t readlen = 0x200;
            if (contentIdx + 0x200 > file->size)
            {
                readlen = file->size - contentIdx;
                fileEnd = true;
            }

            mem_copy(data, &fileContent[contentIdx], readlen);
            contentIdx += readlen;

            mem_free(data);
        }
    }

    mem_free(clusterChain);

    return fileContent;
}

struct FILE* newEntry(const uint8_t partIdx, const uint32_t baseDir, const char* const name, const uint8_t attrib, const uint32_t size)
{	
	struct DIR_ENTRY* existingEntry = findEntry(partIdx, baseDir, name, 0, 0);
	if (existingEntry)
	{
		mem_free(existingEntry);
		
		term_writeline("Directory entry already exists!", false);
        return (struct FILE*)0;
	}

    if (!baseDir)
    {
        term_writeline("Invalid path!", false);
        return (struct FILE*)0;
    }

    size_t unusedIdx = findUnusedDirEntry(partIdx, baseDir);
    if (!unusedIdx)
    {
        term_writeline("Couldn't find unused directory entry!", false);
        return (struct FILE*)0;
    }

    size_t entryIdx = unusedIdx & 0xF;
    size_t secIdx = clusterToSector(partIdx, 0) + (unusedIdx >> 4);

	// How many clusters are used by the entry
	size_t clusterCount = (size / partArray[partIdx].sectorsPerCluster) + (size % partArray[partIdx].sectorsPerCluster);
	// Each entry must use at least one cluster
	if (!clusterCount)
	{
		clusterCount = 1;
	}
	
	// Find a cluster for the file
	uint32_t firstCluster = findEmptyCluster(partIdx);
	uint32_t prevCluster = 0;
	for (size_t i = 0; i < clusterCount; i++)
	{
		uint32_t nextCluster = findEmptyCluster(partIdx);
		if (!nextCluster)
		{
			term_writeline("Couldn't find an empty cluster!", false);
			return (struct FILE*)0;
		}
		
		if (i)
		{
			// The first cluster is later on used when the information about the file itself is being generated
			firstCluster = nextCluster;
		}
		else
		{
			// Write the cluster index of this cluster to the previous cluster
			// This creates the desired cluster chain
			fatWrite(partIdx, prevCluster, nextCluster);
		}
		
		prevCluster = nextCluster;
	}
	// Write the cluster chain terminator to the last cluster in the chain
	fatWrite(partIdx, prevCluster, CLUSTER_CHAIN_TERMINATOR);

    // Read old directory entries from the sector
    struct DIR_SECTOR* dirsec = (struct DIR_SECTOR*)hddRead(hddArray[partArray[partIdx].hddIdx], secIdx);
    if (!dirsec)
    {
        term_writeline("Unable to read directory sector!", false);
        return (struct FILE*)0;
    }

    // Write the file information into the proper directory entry
    stringToFileName(name, &dirsec->entries[entryIdx].fileName[0]);
    dirsec->entries[entryIdx].attrib = attrib; // I don't know why, but all the files seem to have this
    dirsec->entries[entryIdx].clusterHigh = (uint16_t)(firstCluster >> 0x10);
    dirsec->entries[entryIdx].clusterLow = (uint16_t)firstCluster;
    dirsec->entries[entryIdx].fileSize = size;

    hddWrite(hddArray[partArray[partIdx].hddIdx], secIdx, (uint8_t*)dirsec);

    mem_free(dirsec);

    // Generate the FILE structure
    struct FILE* file = mem_alloc(sizeof(struct FILE));

    strcopy(name, &file->name[0]);
    file->partIdx = partIdx;
    file->attrib = attrib;
    file->cluster = firstCluster;
    file->size = size;

    return file;
}

struct FILE* newFile(const uint8_t partIdx, const uint32_t baseDir, const char* const path)
{
	uint32_t targetDir = 0;
	char* pathName = (char*)0;
	
	extractPath(partIdx, baseDir, path, &targetDir, &pathName);
	
	if (!targetDir)
	{
		term_writeline("Invalid directory path!", false);
		return (struct FILE*)0;
	}
	
	if (!pathName)
	{
		term_writeline("Invalid file name!", false);
		return (struct FILE*)0;
		
	}
	
	struct FILE* file = newEntry(partIdx, targetDir, pathName, FILE_ATTRIB_ARCHIVE, 0);
	
	mem_free(pathName);
	
	return file;	
}

struct FILE* newDir(const uint8_t partIdx, const uint32_t baseDir, const char* const path)
{
	uint32_t targetDir = 0;
	char* pathName = (char*)0;
	
	extractPath(partIdx, baseDir, path, &targetDir, &pathName);
	
	if (!targetDir)
	{
		term_writeline("Invalid directory path!", false);
		return (struct FILE*)0;
	}
	
	if (!pathName)
	{
		term_writeline("Invalid directory name!", false);
		return (struct FILE*)0;
	}
	
	// Create a new directory entry
	const uint32_t dirSize = 0x20 * 3;
    struct FILE* dir = newEntry(partIdx, targetDir, pathName, FILE_ATTRIB_DIRECTORY, dirSize);
	
	mem_free(pathName);
	
	if (!dir)
	{
		term_writeline("Failed to create new entry!", false);
		return (struct FILE*)0;
	}
	
	// Get the index of the first directory sector
	uint64_t firstDirSector = clusterToSector(partIdx, dir->cluster);		
	
	// Allocate memory space to store the generated directory sector
	struct DIR_SECTOR* dirsec = (struct DIR_SECTOR*)mem_alloc(0x200);
	
	// First entry points to the directory itself
	stringToFileName(".", &dirsec->entries[0].fileName[0]);
	dirsec->entries[0].attrib = FILE_ATTRIB_DIRECTORY;
	dirsec->entries[0].clusterHigh = (uint16_t)(firstDirSector >> 0x10);
	dirsec->entries[0].clusterLow = (uint16_t)firstDirSector;
	dirsec->entries[0].fileSize = dirSize;
	
	// Second entry points to the base directory
	stringToFileName("..", &dirsec->entries[1].fileName[0]);
	dirsec->entries[1].attrib = FILE_ATTRIB_DIRECTORY;
	dirsec->entries[1].clusterHigh = (uint16_t)(targetDir >> 0x10);
	dirsec->entries[1].clusterLow = (uint16_t)targetDir;
	dirsec->entries[1].fileSize = 0; // I don't know this one, but I guess it should work just fine with 0 bytes length
	
	// Mark the third entry as the end of the directory
	*(uint8_t*)&(dirsec->entries[2]) = DIR_ENTRY_END;
	
	// Write the newly created directory sector to the disk
	hddWrite(hddArray[partArray[partIdx].hddIdx], firstDirSector, (uint8_t*)dirsec);
	
	mem_free(dirsec);
	
	return dir;
}

void deleteEntry(const uint8_t partIdx, const uint32_t baseDir, const char* const path)
{
	uint32_t targetDir = 0;
	char* pathName = (char*)0;
	
	// Get the directory path and entry name from the full path
	extractPath(partIdx, baseDir, path, &targetDir, &pathName);
	
	if (!targetDir)
	{
		term_writeline("Invalid directory path!", false);
		return;
	}
	
	if (!pathName)
	{
		term_writeline("Invalid entry name!", false);
		return;
	}
	
	// -- Delete the directory entry
	
    // Get cluster chain of the directory that contains the entry
    uint32_t* dircc = getClusterChain(partIdx, targetDir);

    bool endOfDir = false;
	bool entryFound = false;
	
	uint32_t entryCluster = 0;

    // Search through the cluster chain until the end of the directory is reached
	for (size_t i = 0; dircc[i] < CLUSTER_CHAIN_TERMINATOR && !endOfDir && !entryFound; i++)
    {
        // Convert cluster to sector for LBA addressing
        uint64_t clusterBase = clusterToSector(partIdx, dircc[i]);

        // Look through each sector within the cluster
        for (size_t iSec = 0; iSec < partArray[partIdx].sectorsPerCluster && !endOfDir && !entryFound; iSec++)
        {
            // Read the sector from the drive
            struct DIR_SECTOR* dirsec = (struct DIR_SECTOR*)hddRead(hddArray[partArray[partIdx].hddIdx], clusterBase + iSec);

            // Look through all the entries in the sector
            for (size_t iEntry = 0; iEntry < 0x10 && !endOfDir && !entryFound; iEntry++)
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
                    dirsec->entries[iEntry].attrib != FILE_ATTRIB_LONG_NAME) // mustn't be a long name entry
                {
                    char* strName = fileNameToString(&dirsec->entries[iEntry].fileName[0]);
                    bool namesMatch = strcmp(pathName, strName);
                    mem_free(strName);

                    // Names match, we've found the entry
                    if (namesMatch)
                    {
						entryCluster = joinCluster(dirsec->entries[iEntry].clusterHigh, dirsec->entries[iEntry].clusterLow);
						
						// Mark this entry as unused
                        *(uint8_t*)&(dirsec->entries[iEntry]) = DIR_ENTRY_UNUSED;
						
						// Write the modified directory sector to disk
						hddWrite(hddArray[partArray[partIdx].hddIdx], clusterBase + iSec, (uint8_t*)dirsec);

                        mem_free(dirsec);
                        mem_free(dircc);
						mem_free(pathName);

						// Entry has been found and deleted
                        entryFound = true;
                    }
                }
            }

            mem_free(dirsec);
        }
    }
	
	if (!entryFound)
	{
		mem_free(dircc);
		mem_free(pathName);
		
		term_writeline("Entry could not be found in the directory!", false);
		return;
	}
	
	if (!entryCluster)
	{
		term_writeline("Entry had an invalid begin cluster!", false);
		return;
	}
	
	// -- Free all the cluster in the cluster chain
	
	// Get cluster chain of the entry
    uint32_t* entrycc = getClusterChain(partIdx, entryCluster);

    // Free each one of the clusters used by the entry
	for (size_t i = 0; entrycc[i] < CLUSTER_CHAIN_TERMINATOR && !endOfDir && !entryFound; i++)
    {
		// Free the cluster
		fatWrite(partIdx, entrycc[i], 0);
    }

    mem_free(entrycc);
}
