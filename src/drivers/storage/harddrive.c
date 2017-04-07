#include <drivers/storage/harddrive.h>
#include <drivers/storage/atapio.h>
#include <drivers/storage/ahci.h>
#include <drivers/memory.h>
#include <drivers/storage/fat.h>
#include <c/string.h>

struct HARDDRIVE hddArray[0x10];
uint8_t hddCount = 0;

uint8_t hddTestLast(void)
{
    if (hdd_init(hddCount))
    {
        return hddCount++;
    }
    else
    {
        return HDD_INVALID;
    }
}

char* getHddInfoStr(const uint8_t hddIdx)
{
    char* strInfo = mem_alloc(128);
    size_t strIdx = 0;

    if (hddArray[hddIdx].type == HDD_TYPE_IDE)
    {
        // Drive Type
        strcopy("IDE", strInfo, strIdx);
        strIdx += 3;

        // BUS
        uint16_t bus = hddArray[hddIdx].addr >> 8;
        if (bus == BUS_PRIMARY)
        {
            strcopy(" : Primary Bus", strInfo, strIdx);
            strIdx += 14;
        }
        else if (bus == BUS_SECONDARY)
        {
            strcopy(" : Secondary Bus", strInfo, strIdx);
            strIdx += 16;
        }

        // DRIVE
        uint8_t drive = hddArray[hddIdx].addr;
        if (drive == DRIVE_MASTER)
        {
            strcopy(" : Master Drive", strInfo, strIdx);
            strIdx += 15;
        }
        else if (drive == DRIVE_SLAVE)
        {
            strcopy(" : Slave Drive", strInfo, strIdx);
            strIdx += 14;
        }
    }
    else if (hddArray[hddIdx].type == HDD_TYPE_AHCI)
    {
        strcopy("AHCI", strInfo, strIdx);
        strIdx += 4;
    }

    strInfo[strIdx] = '\0';

    return strInfo;
}

uint8_t hddAddIDE(const uint16_t bus, const uint8_t drive)
{
    hddArray[hddCount].type = HDD_TYPE_IDE;
    hddArray[hddCount].addr = (bus << 8) + drive;

    return hddTestLast();
}

uint8_t hddAddAHCI(const HBA_PORT* const port)
{
    hddArray[hddCount].type = HDD_TYPE_AHCI;
    hddArray[hddCount].addr = (size_t)port;

    return hddTestLast();
}

uint8_t* hddRead(const struct HARDDRIVE hdd, const uint64_t lba)
{
    if (hdd.type == HDD_TYPE_IDE)
    {
        return readLBA48((uint16_t)(hdd.addr >> 8), (uint8_t)hdd.addr, lba);
    }
    else if (hdd.type == HDD_TYPE_AHCI)
    {
        uint8_t* buff = (uint8_t*)mem_alloc(0x200);
        ahci_read((HBA_PORT*)hdd.addr, lba, 1, (uint16_t*)buff);
        return buff;
    }

    return (uint8_t*)0;
}

void hddWrite(const struct HARDDRIVE hdd, const uint64_t lba, const uint8_t* const data)
{
    if (hdd.type == HDD_TYPE_IDE)
    {
        writeLBA48((uint8_t)(hdd.addr >> 8), (uint8_t)hdd.addr, lba, data);
    }
    else if (hdd.type == HDD_TYPE_AHCI)
    {
        // TODO
    }
}
