#include <drivers/storage/fat.h>
#include <drivers/io/terminal.h>
#include <drivers/memory.h>
#include <drivers/storage/harddrive.h>
#include <c/string.h>

#include <kernel.h>

bool hdd_init(const uint8_t hddIdx)
{
    bool isValidFat = false;
    struct MBR* mbr = (struct MBR*)hddRead(hddArray[hddIdx], 0);

    // Check the boot segment signature 0xAA55
    if (mbr->signature == FAT_SIGNATURE)
    {
        // Look for partitions    
        bool partValid[4];

        for (size_t i = 0; i < 4; i++)
        {
            // A valid partition must not start at the very beginning of the disk
            // and it must occupy at least one disk sector
            if (mbr->part[i].lbabegin > 0 && mbr->part[i].sectors > 0)
            {
                debug_print("Start");

                partValid[i] = checkVolumeID(hddIdx, mbr->part[i].lbabegin);

                debug_print("Stop");
                
                if (partValid[i])
                {
                    isValidFat = true;

                    partArray[partCount - 1].lbaBegin = mbr->part[i].lbabegin;
                    partArray[partCount - 1].lbaBegin = mbr->part[i].lbabegin;
                    partArray[partCount - 1].sectorCount = mbr->part[i].sectors;

                    term_writeline(getPartInfoStr(partCount - 1), true);
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
            /*if (checkVolumeID(hddArray[hddIdx], 1))
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

void hdd_init_last(void)
{
    if (hdd_init(hddCount))
    {
        hddCount++;
    }
}
