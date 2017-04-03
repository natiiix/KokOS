#pragma once

#include <stddef.h>
#include <stdint.h>

#include <drivers/storage/harddrive.h>

// Sector 0 : Master Boot Record
//  -- 0x00B [0x1BE] : Unknown
//  -- 0x1BE [0x010] : Partition 0
//  -- 0x1CE [0x010] : Partition 1
//  -- 0x1DE [0x010] : Partition 2
//  -- 0x1EE [0x010] : Partition 3
//  -- 0x1FE [0x002] : Boot sector signature (0x55, 0xAA)
//
// Partition:
//  -- 0x0 [0x1] : Boot flag
//  -- 0x1 [0x3] : CHS begin
//  -- 0x4 [0x1] : Type code (0x0B / 0x0C for FAT32)
//  -- 0x5 [0x3] : CHS end
//  -- 0x8 [0x4] : LBA begin
//  -- 0xC [0x4] : Number of sectors
//
// Volume ID
//  -- 0x000 [0x003] : Jump instruction
//  -- 0x003 [0x008] : OEM name
//
// TODO :)
//

static const uint8_t FAT_ALTERNATIVE_SIGNATURE = 0x28;

struct PARTITION
{
    uint8_t bootflag;
    uint8_t chsbegin[3];
    uint8_t typecode;
    uint8_t chsend[3];
    uint32_t lbabegin;
    uint32_t sectors;
} __attribute__((packed));

struct MBR
{
    uint8_t unknown[0x1BE];
    struct PARTITION part[4];
    uint16_t signature;
} __attribute__((packed));

struct VOLUMEID
{
    uint8_t jumpinstr[0x3];     // 0x000
    char oemname[0x8];          // 0x003
    uint16_t bytesPerSector;    // 0x00B
    uint8_t sectorsPerCluster;  // 0x00D
    uint16_t reservedSectors;   // 0x00E
    uint8_t fatCount;           // 0x010
    uint8_t unknown1[0x13];     // 0x011
    uint32_t fatSectors;        // 0x024
    uint8_t unknown2[0x4];      // 0x028
    uint32_t rootDirCluster;    // 0x02C
    uint8_t unknown3[0x12];     // 0x030
    uint8_t extBootSignature;   // 0x042
    uint32_t volumeID;          // 0x043
    char label[0xB];            // 0x047
    char fsType[0x8];           // 0x052
    uint8_t unknown4[0x1A4];    // 0x05A
    uint16_t signature;         // 0x1FE
} __attribute__((packed));

bool fat_init(const struct HARDDRIVE hdd);
