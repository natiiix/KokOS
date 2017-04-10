#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

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

// ---- HARDDRIVE ----
static const uint16_t BYTES_PER_SECTOR = 0x200;
static const uint8_t FAT_COUNT = 0x2;
static const uint16_t FAT_SIGNATURE = 0xAA55;

static const uint8_t FAT_ALTERNATIVE_SIGNATURE = 0x28;

struct MBR_ENTRY
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
    struct MBR_ENTRY part[4];
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

bool hdd_init(const uint8_t hddIdx);

// ---- PARTITION ----
struct PARTITION
{
    char oemname[0x8];
    uint8_t sectorsPerCluster;
    uint16_t reservedSectors;
    uint32_t fatSectors;
    uint32_t rootDirCluster;
    uint8_t extBootSignature;
    uint32_t volumeID;
    char label[0xB];
    char fsType[0x8];

    uint8_t hddIdx;
    uint32_t lbaBegin;
    uint32_t sectorCount;
};

extern struct PARTITION partArray[0x10];
extern uint8_t partCount;

bool checkVolumeID(const uint8_t hddIdx, const uint64_t lba);

#if defined(__cplusplus)
extern "C"
#endif
char* getPartInfoStr(const uint8_t partIdx);

// ---- DIRECTORY ----
static const uint32_t CLUSTER_CHAIN_TERMINATOR = 0xFFFFFFFF;

struct FAT_TABLE
{
    uint32_t entries[128];
} __attribute__((packed));

static const uint8_t DIR_ENTRY_END = 0x00;
static const uint8_t DIR_ENTRY_UNUSED = 0xE5;

struct DIR_ENTRY
{
    char fileName[11];
    uint8_t attrib;
    uint8_t unknown1[8];
    uint16_t clusterHigh;
    uint8_t unknown2[4];
    uint16_t clusterLow;
    uint32_t fileSize;
} __attribute__((packed));

struct DIR_SECTOR
{
    struct DIR_ENTRY entries[16];
} __attribute__((packed));

static const uint8_t FILE_ATTRIB_READ_ONLY  = 0x01;
static const uint8_t FILE_ATTRIB_HIDDEN     = 0x02;
static const uint8_t FILE_ATTRIB_SYSTEM     = 0x04;
static const uint8_t FILE_ATTRIB_VOLUME_ID  = 0x08;
static const uint8_t FILE_ATTRIB_DIRECTORY  = 0x10;
static const uint8_t FILE_ATTRIB_ARCHIVE    = 0x20;

static const uint8_t FILE_ATTRIB_LONG_NAME  = 0x0F;

struct FILE
{
    char name[13];
    uint8_t partIdx;
    uint8_t attrib;
    uint32_t cluster;
    uint32_t size;
};

#if defined(__cplusplus)
extern "C"
{
#endif

// Converts a cluster index to disk sector
uint64_t clusterToSector(const uint8_t partIdx, const uint32_t clust);
// Returns an array of clusters chained after a specified cluster
uint32_t* getClusterChain(const uint8_t partIdx, const uint32_t firstClust);
// Finds a first empty cluster on a specified partition
uint32_t findEmptyCluster(const uint8_t partIdx);
// Writes an entry to the FAT table
void fatWrite(const uint8_t partIdx, const uint32_t clustIdx, const uint32_t content);
// Inserts an empty cluster (found by findEmptyCluster()) to the cluster chain beginning with a specified cluster
void prolongClusterChain(const uint8_t partIdx, const uint32_t firstClust);
// Convert a FAT file name to a standard cstring format ("NAME    EXT" to "NAME.EXT")
char* fileNameToString(const char* const fileName);
// Converts a cstring file name to a FAT file name ("NAME.EXT" to "NAME    EXT")
void stringToFileName(const char* const strSrc, char* const fileNameDst);
// Version of the stringToFileName() function which ignores dots and therefore doesn't handle extensions
void stringToFileNameNoExt(const char* const strSrc, char* const fileNameDst);
// Checks directory entry attributes using mask
bool attribCheck(const uint8_t entryAttrib, const uint8_t attribMask, const uint8_t attrib);
// Lists the content of a directory to the terminal
void listDirectory(const uint8_t partIdx, const uint32_t dirFirstClust);
// Returns high 16 bits and low 16 bits of a cluster index joined into a single 32 bit cluster index
uint32_t joinCluster(const uint16_t clusterHigh, const uint16_t clusterLow);
// Returns cluster index of the directory pointed to by the path string
uint32_t resolvePath(const uint8_t partIdx, const uint32_t baseDir, const char* const path);
// Generates an absolute entry index from mulitple relative indexes (used when returning entry index from a function)
size_t generateDirEntryIndex(const uint8_t partIdx, const size_t clusterIdx, const size_t sectorIdx, const size_t entryIdx);
// Find the first unused directory entry in a specified directory, shift the end of the directory if there is no unused entry
size_t findUnusedDirEntry(const uint8_t partIdx, const uint32_t baseDir);
// Extracts directory cluster and name from a full path
void extractPath(const uint8_t partIdx, const uint32_t baseDir, const char* const pathFull, uint32_t* targetDir, char** const pathNamePtr);

// ---- ENTRY ----
struct DIR_ENTRY* findEntry(const uint8_t partIdx, const uint32_t baseDirCluster, const char* const name, const uint8_t attribMask, const uint8_t attrib);
struct FILE* getFile(const uint8_t partIdx, const uint32_t baseDir, const char* const path);

// Read
uint8_t* readFile(const struct FILE* const file);

// New
struct FILE* newFile(const uint8_t partIdx, const uint32_t baseDir, const char* const path);
struct FILE* newDir(const uint8_t partIdx, const uint32_t baseDir, const char* const path);

// Delete
void deleteEntry(const uint8_t partIdx, const uint32_t baseDir, const char* const path);

#if defined(__cplusplus)
}
#endif
