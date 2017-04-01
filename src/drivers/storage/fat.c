#include <drivers/storage/fat.h>
#include <drivers/io/terminal.h>
#include <drivers/storage/atapio.h>
#include <drivers/memory.h>

static const uint16_t SIGNATURE = 0xAA55;

bool checkVolumeID(const enum BUS bus, const enum DRIVE drive, uint64_t lba)
{
    uint8_t* volumeid = readLBA48(bus, drive, lba);

    uint16_t* signptr = (uint16_t*)(volumeid + 0x1FE);
    bool volumeidValid =  (*signptr == SIGNATURE);

    mem_free(volumeid);
    return volumeidValid;
}

void fat_init_ide(const enum BUS bus, const enum DRIVE drive)
{
    struct MBR* mbr = (struct MBR*)readLBA48(bus, drive, 0);

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
                partValid[i] = checkVolumeID(bus, drive, part->lbabegin);
                
                if (partValid[i])
                {
                    term_write("Partition ", false);
                    term_write_convert(i, 16);
                    term_write(": 0x", false);
                    term_write_convert(part->sectors * 0x200, 16);
                    term_writeline(" Bytes", false);
                }
            }
        }

        // If there is no valid partition the drive is probably partitionless
        // (a single partition spans across the whole drive)
        if (!partValid[0] && !partValid[1] && !partValid[2] && !partValid[3])
        {
            if (checkVolumeID(bus, drive, 1))
            {
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
}
