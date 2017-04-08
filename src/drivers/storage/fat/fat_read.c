#include <drivers/storage/fat.h>
#include <drivers/memory.h>
#include <drivers/storage/harddrive.h>

uint8_t* fatReadFile(const struct FILE file)
{
    // It's impossible to read the contents of an empty file
    if (file->size == 0)
    {
        return (uint8_t*)0;
    }

    uint32_t* clusterChain = getClusterChain(partIdx, dirFirstClust);
    uint8_t* fileContent = mem_alloc(file->size); // used to store the contents of the file
    size_t contentIdx = 0;
    bool fileEnd = false;

    // Reach all clusters that belong to this file
    for (size_t i = 0; clusterChain[i] < CLUSTER_CHAIN_TERMINATOR && !fileEnd; i++)
    {
        uint64_t clusterBase = clusterToSector(partIdx, clusterChain[i]);

        // Go through all the sectors in the cluster
        for (size_t j = 0; j < partArray[partIdx].sectorsPerCluster && !fileEnd; j++)
        {
            uint8_t* data = hddRead(hddArray[partArray[partIdx].hddIdx], clusterBase + j);

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
