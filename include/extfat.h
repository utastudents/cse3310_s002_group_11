#pragma once

#include <inttypes.h>
#include <stdbool.h>
#include <sys/stat.h>

/* ensure this header can be used in a C++ program */
#ifdef __cplusplus
extern "C"
{
#endif

    // This header defines the layout of data on an extfat disk image.
    
    // For the details, please refer to

    //            https://learn.microsoft.com/en-gb/windows/win32/fileio/exfat-specification

    typedef struct
    {
        char JumpBoot[3];
        char FileSystemName[8];
        char MustBeZero[53];
        long int PartitionOffset;
        long int VolumeLength;
        int FatOffset;
        int FatLength;
        int ClusterHeapOffset;
        int ClusterCount;
        int FirstClusterOfRootDirectory;
        int VolumeSerialNumber;
        short int FileSystemRevision;
        short int VolumeFlags;
        unsigned char BytesPerSectorShift;
        unsigned char SectorsPerClusterShift;
        unsigned char NumberOfFats;
        unsigned char DriveSelect;
        unsigned char PercentInUse;
        unsigned char Reserved[7];
        unsigned char BootCode[390];
        short int BootSignature;
        unsigned char ExcessSpace;
    } Main_Boot;

// Use portable declarations because byte size matters
// Moved from extfat.c
struct bootSector
{
    __u_char jumpBoot[3];
    __u_char fileSystemName[8];
    __u_char mustBeZero[53];
    __uint64_t partitionOffset;
    __uint64_t volumeLength;
    __uint32_t fatOffset;
    __uint32_t fatLength;
    __uint32_t clusterHeapOffset;
    __uint32_t clusterCount;
    __uint32_t firstClusterOfRootDirectory;
    __uint32_t volumeSerialNumber;
    __uint16_t fileSystemRevision;
    __uint16_t volumeFlags;
    __uint8_t bytesPerSectorShift;
    __uint8_t sectorsPerClusterShift;
    __uint8_t numberOfFats;
    __uint8_t driveSelect;
    __uint8_t percentInUse;
    __uint8_t reserved;
    __u_char bootCode[390];
    __u_char bootSignature;
};

// Main memory structure
// Moved from extfat.c
struct instance
{
    bool iflag;
    bool oflag;
    bool cflag;
    bool vflag;
    bool fflag;
    bool mflag;
    int fdInput;
    int fdOutput;
    int opt;
    char * ivalue;
    char * ovalue;
    struct stat inFile;
    struct stat outFile;
    void * memInput;
    void * memOutput;
    const char * function;
    struct bootSector * bootSectorMain; // Sector 0
    struct bootSector * bootSectorBackup; // Sector 12
};

#ifdef __cplusplus
    extern "C"
};
#endif

// Function Declarations
// Moved from extfat.c
int mapFile (struct instance *);
int readFile (struct instance *);
int writeFile (struct instance *);
int openExfat (struct instance *);
void closeExfat (struct instance *);
int readBootSector (struct instance *);
int compareBootSec (struct instance *);