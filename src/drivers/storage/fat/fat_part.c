#include <drivers/storage/fat.h>
#include <drivers/memory.h>
#include <drivers/storage/harddrive.h>
#include <c/string.h>
#include <kernel.h>

#include <drivers/io/terminal.h>

struct PARTITION partArray[0x10];
uint8_t partCount = 0;

bool checkVolumeID(const uint8_t hddIdx, const uint64_t lba)
{
    struct VOLUMEID* volid = (struct VOLUMEID*)hddRead(hddIdx, lba);

    bool volumeidValid =
        (volid->bytesPerSector == BYTES_PER_SECTOR) &&
        (volid->fatCount == FAT_COUNT) &&
        (volid->signature == FAT_SIGNATURE);

    if (volumeidValid)
    {
        // Copy the partition info to the partition array
        partArray[partCount].sectorsPerCluster = volid->sectorsPerCluster;
        partArray[partCount].reservedSectors = volid->reservedSectors;
        partArray[partCount].fatSectors = volid->fatSectors;
        partArray[partCount].rootDirCluster = volid->rootDirCluster;
        partArray[partCount].extBootSignature = volid->extBootSignature;
        partArray[partCount].volumeID = volid->volumeID;

        // OEM Name
        for (size_t i = 0; i < 0x8; i++)
        {
            partArray[partCount].oemname[i] = volid->oemname[i];
        }

        // Partition Label
        for (size_t i = 0; i < 0xB; i++)
        {
            partArray[partCount].label[i] = volid->label[i];
        }

        // Partition File System Type
        for (size_t i = 0; i < 0x8; i++)
        {
            partArray[partCount].fsType[i] = volid->fsType[i];
        }

        partCount++;        
    }

    mem_free(volid);
    return volumeidValid;
}

char* getPartInfoStr(const uint8_t partIdx)
{
    // Generate the partition info string
    char* strInfo = mem_dynalloc(128);
    size_t strIdx = 0;

    // OEM Name
    for (size_t i = 0; i < 8; i++)
    {
        strInfo[strIdx++] = partArray[partIdx].oemname[i];
    }

    strInfo[strIdx++] = ' ';
    strInfo[strIdx++] = ':';
    strInfo[strIdx++] = ' ';

    // Volume ID
    char* strid = tostr(partArray[partIdx].volumeID, 16);
    size_t idlen = strlen(strid);
    size_t padlen = 8 - idlen;

    for (size_t i = 0; i < padlen; i++)
    {
        strInfo[strIdx++] = '0';
    }

    for (size_t i = 0; i < idlen; i++)
    {
        strInfo[strIdx++] = strid[i];
    }

    mem_free(strid);

    // Put a '-' in the middle of the Volume ID
    for (size_t i = 0; i < 4; i++)
    {
        strInfo[strIdx - i] = strInfo[strIdx - i - 1];
    }

    strInfo[strIdx - 4] = '-';
    strIdx++;

    // Only generate this part of the info string if extBootSignature is correct
    if (partArray[partIdx].extBootSignature != FAT_ALTERNATIVE_SIGNATURE)
    {
        strInfo[strIdx++] = ' ';
        strInfo[strIdx++] = ':';
        strInfo[strIdx++] = ' ';

        // Volume Label
        for (size_t i = 0; i < 11; i++)
        {
            strInfo[strIdx++] = partArray[partIdx].label[i];
        }

        // Remove spaces from the end
        for (size_t i = 0; i < 11; i++)
        {
            if (strInfo[strIdx - 1] == ' ')
            {
                strIdx--;
            }
            else
            {
                break;
            }
        }

        strInfo[strIdx++] = ' ';
        strInfo[strIdx++] = ':';
        strInfo[strIdx++] = ' ';

        // Volume File System Type
        for (size_t i = 0; i < 8; i++)
        {
            strInfo[strIdx++] = partArray[partIdx].fsType[i];
        }

        // Remove spaces from the end
        for (size_t i = 0; i < 11; i++)
        {
            if (strInfo[strIdx - 1] == ' ')
            {
                strIdx--;
            }
            else
            {
                break;
            }
        }
    }

    // Size of the partition in Bytes
    char* strbytes = tostr(partArray[partIdx].sectorCount * BYTES_PER_SECTOR, 10);
    size_t byteslen = strlen(strbytes);

    strInfo[strIdx++] = ' ';
    strInfo[strIdx++] = ':';
    strInfo[strIdx++] = ' ';

    for (size_t i = 0; i < byteslen; i++)
    {
        // Separate each 3 digits by ','
        if (i > 0 && (byteslen - i) % 3 == 0)
        {
            strInfo[strIdx++] = ',';
        }

        strInfo[strIdx++] = strbytes[i];
    }

    mem_free(strbytes);

    strInfo[strIdx++] = ' ';
    strInfo[strIdx++] = 'B';
    strInfo[strIdx++] = 'y';
    strInfo[strIdx++] = 't';
    strInfo[strIdx++] = 'e';
    strInfo[strIdx++] = 's';

    strInfo[strIdx] = '\0';

    return strInfo;
}
