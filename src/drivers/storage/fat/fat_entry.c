#include <drivers/storage/fat.h>
#include <drivers/memory.h>
#include <drivers/storage/harddrive.h>
#include <c/string.h>
#include <drivers/io/terminal.h>
#include <kernel.h>

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
            struct DIR_SECTOR* dirsec = (struct DIR_SECTOR*)hddRead(partArray[partIdx].hddIdx, clusterBase + iSec);

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
    debug_print("fat_entry.c | findEntry() | Entry couldn't be found!");
    return (struct DIR_ENTRY*)0;
}

struct FILE* generateFileStruct(const uint8_t partIdx, const struct DIR_ENTRY* const direntry)
{
    // Allocate memory space to store the file structure
    struct FILE* file = mem_alloc(sizeof(struct FILE));

    // Convert the file name to a cstring
    char* strName = fileNameToString(&direntry->fileName[0]);
    strcopy(strName, &file->name[0]);
    mem_free(strName);
    
    // Copy all the properties from the directory entry to the file structure
    file->partIdx = partIdx;
    file->attrib = direntry->attrib;
    file->cluster = joinCluster(direntry->clusterHigh, direntry->clusterLow);
    file->size = direntry->fileSize;

    return file;
}

struct FILE* getFile(const uint8_t partIdx, const uint32_t baseDir, const char* const path)
{
    uint32_t targetDir = 0;
    char* pathName = (char*)0;

    // Extract the directory and the file name from the path string
    extractPath(partIdx, baseDir, path, &targetDir, &pathName);

    if (!pathName)
    {
        debug_print("fat_entry.c | getFile() | File doesn't exist!");
        return (struct FILE*)0;
    }
    else if (!targetDir)
    {
        debug_print("fat_entry.c | getFile() | Directory path is invalid!");
        mem_free(pathName);
        return (struct FILE*)0;
    }

    // Find the entry using the extracted directory and file name
    struct DIR_ENTRY* direntry = findEntry(partIdx, targetDir, pathName, FILE_ATTRIB_DIRECTORY, 0);

    mem_free(pathName);
    
    if (!direntry)
    {
        debug_print("fat_entry.c | getFile() | Couldn't find the directory entry!");
        return (struct FILE*)0;
    }

    struct FILE* file = generateFileStruct(partIdx, direntry);

    mem_free(direntry);

    return (file);
}

uint8_t* readFile(const struct FILE* const file)
{
    // Security check to prevent further problems
    if (!file)
    {
        debug_print("fat_entry.c | readFile() | Can't read from a nullptr!");
        return (uint8_t*)0;
    }

    // It's impossible to read the contents of an empty file
    if (file->size == 0)
    {
        debug_print("fat_entry.c | readFile() | Can't read from an empty file!");
        return (uint8_t*)0;
    }

    uint32_t* clusterChain = getClusterChain(file->partIdx, file->cluster);

    if (!clusterChain)
    {
        debug_print("fat_entry.c | readFile() | Couldn't get the cluster chain!");
        return (uint8_t*)0;
    }

    // Allocate memory space for the file content
    uint8_t* fileContent = mem_alloc(file->size + 1); // used to store the contents of the file
    size_t contentIdx = 0;
    bool fileEnd = false;

    // Reach all clusters that belong to this file
    for (size_t i = 0; clusterChain[i] < CLUSTER_CHAIN_TERMINATOR && !fileEnd; i++)
    {
        // Calculate the index of the first sector of the cluster
        uint64_t clusterBase = clusterToSector(file->partIdx, clusterChain[i]);

        // Go through all the sectors in the cluster
        for (size_t j = 0; j < partArray[file->partIdx].sectorsPerCluster && !fileEnd; j++)
        {
            // Read a sector of the file content from the disk
            uint8_t* data = hddRead(partArray[file->partIdx].hddIdx, clusterBase + j);

            // If the file does not occupy the whole sector copy only as much as necessary
            // This also means the end of file has been reached
            size_t readlen = 0x200;
            if (contentIdx + readlen > file->size)
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

    // Used to avoid having to copy the whole file's content when creating a string object from it
    fileContent[file->size] = '\0';

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
        debug_print("fat_entry.c | newEntry() | Couldn't find unused directory entry!");
        return (struct FILE*)0;
    }

    size_t entryIdx = unusedIdx & 0xF;
    size_t secIdx = clusterToSector(partIdx, 0) + (unusedIdx >> 4);

    // How many clusters are used by the entry
	size_t clusterCount = bytesToClusterCount(partIdx, size);	
	// Find the first cluster for the file
	uint32_t firstCluster = findEmptyCluster(partIdx);
    // Set the first cluster as the last cluster
    fatWrite(partIdx, firstCluster, CLUSTER_CHAIN_TERMINATOR);

    // Prolong the cluster chain if necessary
    if (clusterCount > 1)
    {
        prolongClusterChain(partIdx, firstCluster, clusterCount - 1);
    }

    // Read old directory entries from the sector
    struct DIR_SECTOR* dirsec = (struct DIR_SECTOR*)hddRead(partArray[partIdx].hddIdx, secIdx);
    if (!dirsec)
    {
        debug_print("fat_entry.c | newEntry() | Unable to read directory sector!");
        return (struct FILE*)0;
    }

    // Write the file information into the proper directory entry
    stringToFileName(name, &dirsec->entries[entryIdx].fileName[0]);
    dirsec->entries[entryIdx].attrib = attrib;
    dirsec->entries[entryIdx].clusterHigh = (uint16_t)(firstCluster >> 0x10);
    dirsec->entries[entryIdx].clusterLow = (uint16_t)firstCluster;
    dirsec->entries[entryIdx].fileSize = size;

    hddWrite(partArray[partIdx].hddIdx, secIdx, (uint8_t*)dirsec);

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

struct FILE* newFile(const uint8_t partIdx, const uint32_t baseDir, const char* const path, const size_t fileSize)
{
	uint32_t targetDir = 0;
	char* pathName = (char*)0;
	
    // Extract the directory and the file name from the path
	extractPath(partIdx, baseDir, path, &targetDir, &pathName);
	
	if (!pathName)
	{
		term_writeline("Invalid file name!", false);
		return (struct FILE*)0;		
	}
	
	if (!targetDir)
	{
        mem_free(pathName);
		term_writeline("Invalid directory path!", false);
		return (struct FILE*)0;
	}
	
    // Create a new directory entry
	struct FILE* file = newEntry(partIdx, targetDir, pathName, FILE_ATTRIB_ARCHIVE, fileSize);
	
	mem_free(pathName);
	
	return file;	
}

struct FILE* newDir(const uint8_t partIdx, const uint32_t baseDir, const char* const path)
{
	uint32_t targetDir = 0;
	char* pathName = (char*)0;

    // Extract the target directory and the directory name from the path
	extractPath(partIdx, baseDir, path, &targetDir, &pathName);
	
	if (!pathName)
	{
		term_writeline("Invalid directory name!", false);
		return (struct FILE*)0;
	}
	
	if (!targetDir)
	{
        mem_free(pathName);
		term_writeline("Invalid directory path!", false);        
		return (struct FILE*)0;
	}
	
	// Create a new directory entry
	const uint32_t dirSize = 0x20 * 3;
    struct FILE* dir = newEntry(partIdx, targetDir, pathName, FILE_ATTRIB_DIRECTORY, dirSize);
	
	mem_free(pathName);
	
	if (!dir)
	{
		debug_print("fat_entry.c | newDir() | Failed to create new entry!");
		return (struct FILE*)0;
	}
	
	// Get the index of the first directory sector
	uint64_t firstDirSector = clusterToSector(partIdx, dir->cluster);		
	
	// Allocate memory space to store the generated directory sector
	struct DIR_SECTOR* dirsec = (struct DIR_SECTOR*)mem_alloc(0x200);
	
	// First entry points to the directory itself
    // Standard stringToFileName() wouldn't allow us to write dots in the file name
	stringToFileNameNoExt(".", &dirsec->entries[0].fileName[0]);
	dirsec->entries[0].attrib = FILE_ATTRIB_DIRECTORY;
	dirsec->entries[0].clusterHigh = (uint16_t)(firstDirSector >> 0x10);
	dirsec->entries[0].clusterLow = (uint16_t)firstDirSector;
	dirsec->entries[0].fileSize = dirSize;
	
	// Second entry points to the base directory
	stringToFileNameNoExt("..", &dirsec->entries[1].fileName[0]);
	dirsec->entries[1].attrib = FILE_ATTRIB_DIRECTORY;
	dirsec->entries[1].clusterHigh = (uint16_t)(targetDir >> 0x10);
	dirsec->entries[1].clusterLow = (uint16_t)targetDir;
	dirsec->entries[1].fileSize = 0; // I don't know this one, but I guess it should work just fine with 0 bytes length
	
	// Mark the third entry as the end of the directory
	*(uint8_t*)&(dirsec->entries[2]) = DIR_ENTRY_END;
	
	// Write the newly created directory sector to the disk
	hddWrite(partArray[partIdx].hddIdx, firstDirSector, (uint8_t*)dirsec);
	
	mem_free(dirsec);
	
	return dir;
}

bool dirIsEmpty(const uint8_t partIdx, const uint32_t dirFirstClust)
{
    // Get cluster chain
    uint32_t* clusterChain = getClusterChain(partIdx, dirFirstClust);

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
            struct DIR_SECTOR* dirsec = (struct DIR_SECTOR*)hddRead(partArray[partIdx].hddIdx, clusterBase + iSec);

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
                    dirsec->entries[iEntry].attrib != FILE_ATTRIB_LONG_NAME) // mustn't be a long name entry
                {
                    char* strName = fileNameToString(&dirsec->entries[iEntry].fileName[0]);
                    bool validEntry = !strcmp(".", strName) && !strcmp("..", strName); // "." and ".." aren't real entries
                    mem_free(strName);

                    // This directory contains a valid entry
                    // That means it can't be safely deleted
                    if (validEntry)
                    {
                        mem_free(dirsec);
                        mem_free(clusterChain);

                        // Directory is not empty
                        return false;
                    }
                }
            }

            mem_free(dirsec);
        }
    }

    mem_free(clusterChain);

    // Directory is empty
    return true;
}

bool deleteEntry(const uint8_t partIdx, const uint32_t baseDir, const char* const path)
{
	uint32_t targetDir = 0;
	char* pathName = (char*)0;
	
	// Get the directory path and entry name from the full path
	extractPath(partIdx, baseDir, path, &targetDir, &pathName);
	
	if (!pathName)
	{
		term_writeline("Invalid entry name!", false);
		return false;
	}
	
	if (!targetDir)
	{
        mem_free(pathName);
		term_writeline("Invalid directory path!", false);
		return false;
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
            struct DIR_SECTOR* dirsec = (struct DIR_SECTOR*)hddRead(partArray[partIdx].hddIdx, clusterBase + iSec);

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
                        if (dirsec->entries[iEntry].attrib & FILE_ATTRIB_DIRECTORY &&
                            !dirIsEmpty(partIdx, joinCluster(dirsec->entries[iEntry].clusterHigh, dirsec->entries[iEntry].clusterLow)))
                        {
                            mem_free(dirsec);
                            mem_free(dircc);
                            mem_free(pathName);

                            term_writeline("Cannot delete a non-empty directory!", false);
                            return false;
                        }

						entryCluster = joinCluster(dirsec->entries[iEntry].clusterHigh, dirsec->entries[iEntry].clusterLow);
						
						// Mark this entry as unused
                        *(uint8_t*)&(dirsec->entries[iEntry]) = DIR_ENTRY_UNUSED;
						
						// Write the modified directory sector to disk
						hddWrite(partArray[partIdx].hddIdx, clusterBase + iSec, (uint8_t*)dirsec);

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
		
		term_writeline("Specified entry doesn't exist!", false);
		return false;
	}
	
	if (!entryCluster)
	{
		debug_print("fat_entry.c | deleteEntry() | Entry had an invalid begin cluster!");
		return false;
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
    
    return true;
}

struct FILE* writeFile(const uint8_t partIdx, const uint32_t baseDir, const char* const path, const uint8_t* const data, const size_t dataSize)
{
    uint32_t targetDir = 0;
    char* pathName = (char*)0;
    
    // Extract the directory and the file name from the path
    extractPath(partIdx, baseDir, path, &targetDir, &pathName);

    if (!pathName)
	{
		term_writeline("Invalid file name!", false);
		return (struct FILE*)0;		
	}
	
	if (!targetDir)
	{
        mem_free(pathName);
		term_writeline("Invalid directory path!", false);
		return (struct FILE*)0;
	}

    // Try to find an already existing entry
    struct DIR_ENTRY* existingEntry = findEntry(partIdx, targetDir, pathName, 0, 0);

    struct FILE* file = (struct FILE*)0;

    // File already exists
    if (existingEntry)
    {
        debug_print("fat_entry.c | writeFile() | File already exists! Updating file information!");

        // Check if the found entry is a directory
        if (attribCheck(existingEntry->attrib, FILE_ATTRIB_DIRECTORY, FILE_ATTRIB_DIRECTORY))
        {
            term_writeline("Can't overwrite a directory with a file!", false);
            mem_free(existingEntry);
		    return (struct FILE*)0;
        }

        // Get cluster chain
        uint32_t* dircc = getClusterChain(partIdx, targetDir);

        bool endOfDir = false;
        bool entryFound = false;

        // Search through the cluster chain until the end of the directory is reached or the entry is found
        for (size_t ccIdx = 0; dircc[ccIdx] < CLUSTER_CHAIN_TERMINATOR && !endOfDir && !entryFound; ccIdx++)
        {
            // Convert cluster to sector for LBA addressing
            uint64_t dirClusterBase = clusterToSector(partIdx, dircc[ccIdx]);

            // Look through each sector within the cluster
            for (size_t secIdx = 0; secIdx < partArray[partIdx].sectorsPerCluster && !endOfDir && !entryFound; secIdx++)
            {
                // Read the sector from the drive
                struct DIR_SECTOR* dirsec = (struct DIR_SECTOR*)hddRead(partArray[partIdx].hddIdx, dirClusterBase + secIdx);

                // Look through all the entries in the sector
                for (size_t entryIdx = 0; entryIdx < 0x10 && !endOfDir && !entryFound; entryIdx++)
                {
                    // Get the first byte of the entry
                    // Used to find unused entries and the end of the directory
                    uint8_t entryFirstByte = *(uint8_t*)&(dirsec->entries[entryIdx]);

                    // End of directory reached
                    if (entryFirstByte == DIR_ENTRY_END)
                    {
                        endOfDir = true;
                    }
                    else if (entryFirstByte != DIR_ENTRY_UNUSED && // mustn't be an unused entry
                        dirsec->entries[entryIdx].clusterHigh == existingEntry->clusterHigh && // compare this entry with the entry we're looking for
                        dirsec->entries[entryIdx].clusterLow == existingEntry->clusterLow)
                    {
                        char* strName = fileNameToString(dirsec->entries[entryIdx].fileName);
                        bool match = strcmp(strName, pathName);
                        mem_free(strName);

                        if (!match)
                        {
                            continue;
                        }

                        // Get the number of clusters used by the file before the overwrite
                        size_t clustCountOld = bytesToClusterCount(partIdx, existingEntry->fileSize);
                        if (!clustCountOld)
                        {
                            clustCountOld = 1;
                        }

                        // Get the number of clusters used by the file after the overwrite
                        size_t clustCountNew = bytesToClusterCount(partIdx, dataSize);
                        if (!clustCountNew)
                        {
                            clustCountNew = 1;
                        }

                        uint32_t fileFirstCluster = joinCluster(existingEntry->clusterHigh, existingEntry->clusterLow);

                        // The cluster chain must be prolonged
                        if (clustCountNew > clustCountOld)
                        {
                            prolongClusterChain(partIdx, fileFirstCluster, clustCountNew - clustCountOld);
                        }
                        // The cluster chain must be shortened
                        else if (clustCountNew < clustCountOld)
                        {
                            shortenClusterChain(partIdx, fileFirstCluster, clustCountOld - clustCountNew);
                        }

                        // Update the file size in the directory entry
                        dirsec->entries[entryIdx].fileSize = dataSize;

                        // Write the updated directory sector to the disk
                        hddWrite(partArray[partIdx].hddIdx, dirClusterBase + secIdx, (uint8_t*)dirsec);

                        // Generate the FILE structure with the updated size
                        file = generateFileStruct(partIdx, existingEntry);

                        mem_free(existingEntry);

                        entryFound = true;
                    }
                }

                mem_free(dirsec);
            }
        }

        mem_free(dircc);
        mem_free(pathName);

        if (entryFound)
        {
            debug_print("fat_entry.c | writeFile() | The file information has been updated successfully!");
        }
        else
        {
            debug_print("fat_entry.c | writeFile() | Unable to find file in the directory even though getFile() found it!");
            mem_free(existingEntry);
            return (struct FILE*)0;
        }
    }
    // File doesn't exist
    else
    {
        debug_print("fat_entry.c | writeFile() | File doesn't exist! Creating new file!");

        // Create a new file
        file = newFile(partIdx, targetDir, pathName, dataSize);
        mem_free(pathName);

        if (!file)
        {
            term_writeline("Failed to create the file!", false);
		    return (struct FILE*)0;
        }
        else
        {
            debug_print("fat_entry.c | writeFile() | File created successfully!");
        }
    }

    // Write the data
    uint32_t* clusterChain = getClusterChain(partIdx, file->cluster);

    if (!clusterChain)
    {
        debug_print("fat_entry.c | writeFile() | The cluster chain of the file is broken!");
        mem_free(file);
        return (struct FILE*)0;
    }

    size_t dataIdx = 0;

    // Write all the clusters
    for (size_t chainIdx = 0; clusterChain[chainIdx] < CLUSTER_CHAIN_TERMINATOR && dataIdx < dataSize; chainIdx++)
    {
        // Get the first sector of the cluster
        uint64_t clusterBase = clusterToSector(partIdx, clusterChain[chainIdx]);

        // Write all the sectors in a cluster
        for (size_t iSec = 0; iSec < partArray[partIdx].sectorsPerCluster && dataIdx < dataSize; iSec++)
        {
            // Write the data to the disk
            hddWrite(partArray[partIdx].hddIdx, clusterBase + iSec, &data[dataIdx]);

            dataIdx += 0x200;
        }
    }

    mem_free(clusterChain);

    debug_print("fat_entry.c | writeFile() | File has been written successfully!");

    return file;
}

bool dirPathValid(const uint8_t partIdx, const uint32_t baseDir, const char* const path)
{
    uint32_t targetDir = 0;
    char* pathName = (char*)0;

    // Extract the directory and the file name from the path string
    extractPath(partIdx, baseDir, path, &targetDir, &pathName);

    if (pathName)
    {
        mem_free(pathName);
    }

    // Directory path doesn't exist if that returned target directory from extractPath() is 0
    return !!targetDir;
}

bool renameEntry(const uint8_t partIdx, const uint32_t baseDir, const char* const path, const char* const newName)
{
	uint32_t targetDir = 0;
	char* pathName = (char*)0;
	
	// Get the directory path and entry name from the full path
	extractPath(partIdx, baseDir, path, &targetDir, &pathName);
	
	if (!pathName)
	{
		term_writeline("Invalid entry name!", false);
		return false;
	}
	
	if (!targetDir)
	{
        mem_free(pathName);
		term_writeline("Invalid directory path!", false);
		return false;
	}

    // Make sure the new name doesn't conflict with an existing file
    struct DIR_ENTRY* existingEntry = findEntry(partIdx, targetDir, newName, 0, 0);

    // Entry with specified name already exists in this directory
    if (existingEntry)
    {
        term_writeline("Directory already contains an entry with specified name!", false);

        mem_free(existingEntry);
        mem_free(pathName);
        return false;
    }

    // Get cluster chain
    uint32_t* clusterChain = getClusterChain(partIdx, targetDir);

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
            struct DIR_SECTOR* dirsec = (struct DIR_SECTOR*)hddRead(partArray[partIdx].hddIdx, clusterBase + iSec);

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
                    dirsec->entries[iEntry].attrib != FILE_ATTRIB_LONG_NAME) // mustn't be a long name entry
                {
                    char* strName = fileNameToString(dirsec->entries[iEntry].fileName);
                    bool namesMatch = strcmp(pathName, strName);
                    mem_free(strName);

                    // Names match, rename the entry
                    if (namesMatch)
                    {
                        // Generate file name from the string
                        stringToFileName(newName, dirsec->entries[iEntry].fileName);

                        // Write the updated directory sector to the disk
                        hddWrite(partArray[partIdx].hddIdx, clusterBase + iSec, (uint8_t*)dirsec);

                        mem_free(dirsec);
                        mem_free(clusterChain);
                        mem_free(pathName);

                        debug_print("fat_entry.c | renameEntry() | Entry was renamed successfully!");
                        return true;
                    }
                }
            }

            mem_free(dirsec);
        }
    }

    mem_free(clusterChain);
    mem_free(pathName);

    // Entry not found
    term_writeline("Directory doesn't contain specified entry!", false);
    return false;
}
