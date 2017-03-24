#pragma once

#include <stddef.h>
#include <stdint.h>

// Sector 0 : Master Boot Record
//  -- 0x000 [0x003] : Jump instruction
//  -- 0x003 [0x008] : OEM name
//  --------------------------------
//  Without partition table:
//  -- 0x00B [0x1F2] : Boot code
//  -- 0x1FD [0x001] : Physical drive number
//  With partition table:
//  -- 0x00B [0x1B3] : Unknown
//  -- 0x1BE [0x010] : Partition 0
//  -- 0x1CE [0x010] : Partition 1
//  -- 0x1DE [0x010] : Partition 2
//  -- 0x1EE [0x010] : Partition 3
//  --------------------------------
//  -- 0x1FE [0x002] : Boot sector signature (0x55, 0xAA)
//
// Partition:
//  -- 0x0 [0x1] : Boot flag
//  -- 0x1 [0x3] : CHS begin
//  -- 0x4 [0x1] : Type code (0x0B / 0x0C for FAT32)
//  -- 0x5 [0x3] : CHS end
//  -- 0x8 [0x4] : LBA begin
//  -- 0xC [0x4] : Number of sectors
