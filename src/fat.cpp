#include "fat.hpp"

#include "terminal.hpp"
#include "atapio.hpp"

static const uint16_t SIGNATURE = 0xAA55;

bool checkVolumeID(BUS bus, DRIVE drive, uint64_t lba)
{
    uint8_t* volumeid = readLBA48(bus, drive, lba);

    uint16_t* signptr = (uint16_t*)(volumeid + 0x1FE);
    bool volumeidValid =  (*signptr == SIGNATURE);

    delete volumeid;
    return volumeidValid;
}

void fat_init_ide(BUS bus, DRIVE drive)
{
    MBR* mbr = (MBR*)readLBA48(bus, drive, 0);

    // Check the boot segment signature 0xAA55
    if (mbr->signature == SIGNATURE)
    {
        char oemname[9];

        for (size_t i = 0; i < 8; i++)
        {
            oemname[i] = (char)mbr->oemname[i];
        }

        oemname[8] = '\0';

        term::write("FAT: ");
        term::writeline(&oemname[0]);

        // Look for partitions    
        bool partValid[4];

        for (size_t i = 0; i < 4; i++)
        {
            PARTITION* part = (PARTITION*)mbr->part[i];

            // A valid partition must not start at the very beginning of the disk
            // and it must occupy at least one disk sector
            if (part->lbabegin > 0 && part->sectors > 0)
            {
                partValid[i] = checkVolumeID(bus, drive, part->lbabegin);
                
                if (partValid[i])
                {
                    term::write("Partition ");
                    term::write(i, 16);
                    term::write(": 0x");
                    term::write(part->sectors * 0x200, 16);
                    term::writeline(" Bytes");
                }
            }
        }

        // If there is no valid partition the drive is probably partitionless
        // (a single partition spans across the whole drive)
        if (!partValid[0] && !partValid[1] && !partValid[2] && !partValid[3])
        {
            if (checkVolumeID(bus, drive, 1))
            {
                term::writeline("Partitionless drive");
            }
            else
            {
                term::writeline("FAT Error: No valid Volume ID found!");
            }
        }
    }
    else
    {
        term::writeline("FAT Error: The Master Boot Record is broken!");
    }

    delete mbr;
}
