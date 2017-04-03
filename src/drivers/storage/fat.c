#include <drivers/storage/fat.h>
#include <drivers/io/terminal.h>
#include <drivers/memory.h>
#include <drivers/storage/harddrive.h>
#include <c/string.h>

static const uint16_t BYTES_PER_SECTOR = 0x200;
static const uint8_t FAT_COUNT = 0x2;
static const uint16_t SIGNATURE = 0xAA55;

bool checkVolumeID(const struct HARDDRIVE hdd, uint64_t lba, char* const strInfo)
{
    struct VOLUMEID* volid = (struct VOLUMEID*)hddRead(hdd, lba);

    /*term_write_convert(volid->bytesPerSector, 16);
    term_write("; ", false);
    term_write_convert(volid->sectorsPerCluster, 16);
    term_write("; ", false);
    term_write_convert(volid->reservedSectors, 16);
    term_write("; ", false);
    term_write_convert(volid->fatCount, 16);
    term_write("; ", false);
    term_write_convert(volid->fatSectors, 16);
    term_write("; ", false);
    term_write_convert(volid->rootDirCluster, 16);
    term_write("; ", false);
    term_writeline_convert(volid->signature, 16);*/

    bool volumeidValid =
        (volid->bytesPerSector == BYTES_PER_SECTOR) &&
        (volid->fatCount == FAT_COUNT) &&
        (volid->signature == SIGNATURE);

    if (volumeidValid)
    {
        size_t strIdx = 0;

        // OEM Name
        for (size_t i = 0; i < 8; i++)
        {
            strInfo[strIdx++] = volid->oemname[i];
        }

        strInfo[strIdx++] = ' ';
        strInfo[strIdx++] = ':';
        strInfo[strIdx++] = ' ';

        // Volume ID
        char* strid = tostr(volid->volumeID, 16);
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

        if (volid->extBootSignature != FAT_ALTERNATIVE_SIGNATURE)
        {
            strInfo[strIdx++] = ' ';
            strInfo[strIdx++] = ':';
            strInfo[strIdx++] = ' ';

            // Volume Label
            for (size_t i = 0; i < 11; i++)
            {
                strInfo[strIdx++] = volid->label[i];
            }

            strInfo[strIdx++] = ' ';
            strInfo[strIdx++] = ':';
            strInfo[strIdx++] = ' ';

            // Volume File System Type
            for (size_t i = 0; i < 8; i++)
            {
                strInfo[strIdx++] = volid->fsType[i];
            }
        }

        strInfo[strIdx] = '\0';
    }

    mem_free(volid);
    return volumeidValid;
}

bool fat_init(const struct HARDDRIVE hdd)
{
    bool isValidFat = false;
    struct MBR* mbr = (struct MBR*)hddRead(hdd, 0);

    // Check the boot segment signature 0xAA55
    if (mbr->signature == SIGNATURE)
    {
        // Look for partitions    
        bool partValid[4];

        char* partinfo = mem_alloc(45);

        for (size_t i = 0; i < 4; i++)
        {
            // A valid partition must not start at the very beginning of the disk
            // and it must occupy at least one disk sector
            if (mbr->part[i].lbabegin > 0 && mbr->part[i].sectors > 0)
            {
                partValid[i] = checkVolumeID(hdd, mbr->part[i].lbabegin, partinfo);
                
                if (partValid[i])
                {
                    isValidFat = true;
                    
                    term_write("Partition ", false);
                    term_write_convert(i, 16);
                    term_write(" : ", false);
                    term_write(partinfo, false);
                    term_write(": 0x", false);
                    term_write_convert(mbr->part[i].sectors * BYTES_PER_SECTOR, 16);
                    term_writeline(" Bytes", false);
                }
            }
            else
            {
                partValid[i] = false;
            }            
        }

        mem_free(partinfo);

        // If there is no valid partition the drive is probably partitionless
        // (a single partition spans across the whole drive)
        if (!partValid[0] && !partValid[1] && !partValid[2] && !partValid[3])
        {
            /*if (checkVolumeID(hdd, 1))
            {
                isValidFat = true;
                term_writeline("Partitionless drive", false);
            }
            else*/
            // We're gonna ignore partitionless drives for now
            {
                term_writeline("FAT Error: No valid Volume ID found!", false);
            }
        }
    }
    else
    {
        term_writeline("FAT Error: The Master Boot Record is broken!", false);
    }

    mem_free(mbr);
    return isValidFat;
}
