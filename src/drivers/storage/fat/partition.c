#include <drivers/storage/fat.h>
#include <drivers/io/terminal.h>
#include <drivers/memory.h>
#include <drivers/storage/harddrive.h>
#include <c/string.h>

struct PARTITION partArray[0x10];
uint8_t partCount = 0;

bool checkVolumeID(const struct HARDDRIVE hdd, uint64_t lba)
{
    struct VOLUMEID* volid = (struct VOLUMEID*)hddRead(hdd, lba);

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
        partArray[partCount].oemname[0x8] = '\0';

        // Partition Label
        for (size_t i = 0; i < 0xB; i++)
        {
            partArray[partCount].label[i] = volid->label[i];
        }
        partArray[partCount].label[0xB] = '\0';

        // Partition File System Type
        for (size_t i = 0; i < 0x8; i++)
        {
            partArray[partCount].fsType[i] = volid->fsType[i];
        }
        partArray[partCount].fsType[0x8] = '\0';

        partCount++;        
    }

    mem_free(volid);
    return volumeidValid;
}

char* getPartInfoStr(const struct PARTITION part)
{
    // Generate the partition info string
    char* strInfo = mem_alloc(128);
    size_t strIdx = 0;

    // OEM Name
    for (size_t i = 0; i < 8; i++)
    {
        strInfo[strIdx++] = part.oemname[i];
    }

    strInfo[strIdx++] = ' ';
    strInfo[strIdx++] = ':';
    strInfo[strIdx++] = ' ';

    // Volume ID
    char* strid = tostr(part.volumeID, 16);
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
    if (part.extBootSignature != FAT_ALTERNATIVE_SIGNATURE)
    {
        strInfo[strIdx++] = ' ';
        strInfo[strIdx++] = ':';
        strInfo[strIdx++] = ' ';

        // Volume Label
        for (size_t i = 0; i < 11; i++)
        {
            strInfo[strIdx++] = part.label[i];
        }

        strInfo[strIdx++] = ' ';
        strInfo[strIdx++] = ':';
        strInfo[strIdx++] = ' ';

        // Volume File System Type
        for (size_t i = 0; i < 8; i++)
        {
            strInfo[strIdx++] = part.fsType[i];
        }
    }

    // Size of the partition in Bytes
    char* strbytes = tostr(part.sectorCount * BYTES_PER_SECTOR, 10);
    size_t byteslen = strlen(strbytes);

    strInfo[strIdx++] = ' ';
    strInfo[strIdx++] = ':';
    strInfo[strIdx++] = ' ';

    for (size_t i = 0; i < byteslen; i++)
    {
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
