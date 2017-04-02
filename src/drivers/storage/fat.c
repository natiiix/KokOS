#include <drivers/storage/fat.h>
#include <drivers/io/terminal.h>
#include <drivers/memory.h>
#include <drivers/storage/harddrive.h>

static const uint16_t BYTES_PER_SECTOR = 0x200;
static const uint8_t FAT_COUNT = 0x2;
static const uint16_t SIGNATURE = 0xAA55;

bool checkVolumeID(const struct HARDDRIVE hdd, uint64_t lba)
{
    struct VOLUMEID* volid = (struct VOLUMEID*)hddRead(hdd, lba);

    char* strvolid = mem_alloc(513);
    for (size_t i = 0; i < 512; i++)
    {
        char c = ((char*)volid)[i];
        if (c == '\0')
        {
            strvolid[i] = '.';
        }
        else
        {
            strvolid[i] = c;
        }
    }
    strvolid[512] = '\0';
    term_writeline(strvolid, true);

    term_writeline_convert(sizeof(struct VOLUMEID), 16);
    term_writeline_convert((size_t)volid, 16);
    term_writeline_convert((size_t)&volid->bytesPerSector, 16);
    term_writeline_convert((size_t)&volid->fatCount, 16);
    term_writeline_convert((size_t)&volid->signature, 16);
    term_writeline_convert(volid->bytesPerSector, 16);
    term_writeline_convert(volid->fatCount, 16);
    term_writeline_convert(volid->signature, 16);

    bool volumeidValid =
        (volid->bytesPerSector == BYTES_PER_SECTOR) &&
        (volid->fatCount == FAT_COUNT) &&
        (volid->signature == SIGNATURE);

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
        char oemname[9];

        for (size_t i = 0; i < 8; i++)
        {
            oemname[i] = (char)mbr->oemname[i];
        }

        oemname[8] = '\0';

        term_write("FAT: ", false);
        term_writeline(&oemname[0], false);

        // Look for partitions    
        bool partValid[4];

        for (size_t i = 0; i < 4; i++)
        {
            struct PARTITION* part = (struct PARTITION*)mbr->part[i];

            // A valid partition must not start at the very beginning of the disk
            // and it must occupy at least one disk sector
            if (part->lbabegin > 0 && part->sectors > 0)
            {
                partValid[i] = checkVolumeID(hdd, part->lbabegin);
                
                if (partValid[i])
                {
                    isValidFat = true;
                    
                    term_write("Partition ", false);
                    term_write_convert(i, 16);
                    term_write(": 0x", false);
                    term_write_convert(part->sectors * BYTES_PER_SECTOR, 16);
                    term_writeline(" Bytes", false);
                }
            }
            else
            {
                partValid[i] = false;
            }
        }

        // If there is no valid partition the drive is probably partitionless
        // (a single partition spans across the whole drive)
        if (!partValid[0] && !partValid[1] && !partValid[2] && !partValid[3])
        {
            if (checkVolumeID(hdd, 1))
            {
                isValidFat = true;
                term_writeline("Partitionless drive", false);
            }
            else
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
