
#ifndef EXTFAT_H
#define EXTFAT_H

#include <inttypes.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#pragma once

/* ensure this header can be used in a C++ program */
#ifdef __cplusplus
extern "C"
{
#endif


extern int ftruncate64 (int, __off64_t); // I shouldn't have to do this for a glibc function with a manpage

// Macro Declarations
#define isZero(x) (x == 0) // Boolean test for being 0
#define isNull(x) (x == NULL) // Boolean test for being NULL
#define isTrue(x) (x == true) // Boolean test for being true
#define isFalse(x) (x == false) // Boolean test for being false
#define isFault(x) (x == -1) // Boolean test for being -1
#define isNEQ(x,y) (x != y) // Boolean test for being unequal
#define isEQ(x,y) (x != y) // Boolean test for being equal
#define setFunction(x) x->function=__func__ // Set instance structure function to current function
// Inline version of:
// void compareMemory(bool u, struct instance * v, struct instance * w, [struct instance member, doesn't translate] x, int y, char * z)
// {
//    if (memcmp (&(v->x), &(w->x), y) != 0)
//    {
//        OK = false;
//        printf ("%s: Field %s is inconsistant and too messy to verbose print\n", inst->function, z);
//    }
// }
#define compareMemory(u, v, w, x, y, z) { if (memcmp (&(v->x), &(w->x), y) != 0) {OK = false; printf ("%s: Field %s is inconsistant and too messy to verbose print\n", inst->function, z); OK = u; }}
// Inline version of:
// void compareValues(bool v, struct instance * w, struct instance * x, [struct instance member, doesn't translate] y, char * z)
// {
//     if (w->y != x->y)
//     {
//         OK = false;
//         printf ("%s: Field %s is inconsistant [Main:%ld != %ld:Backup]\n", inst->function, z, (__uint64_t)(w->y), (__uint64_t)(x->y));
//         OK = v;
//     }
// }
#define compareValues(v, w, x, y, z) { if (w->y != x->y) {OK = false; printf ("%s: Field %s is inconsistant [Main:%ld != %ld:Backup]\n", inst->function, z, (__uint64_t)(w->y), (__uint64_t)(x->y)); OK = v; }}
#define limitCheck(x,y,z) (((x) >= (y)) && ((x) <= (z))) // Boolean test for x >= y >= z

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
extern int mapFile (struct instance *);
extern int unmapFile (struct instance *);
extern int mmapCopy (struct instance *);

#ifdef CMDLINE_C
int initInstance (struct instance *);
int fillInstance (struct instance *, int, char **);
#else
extern int initInstance (struct instance *);
extern int fillInstance (struct instance *, int, char **);
#endif

#ifdef VERIFY_C
    int verifyExfat (struct instance *);
    int compareBootSec (struct instance *);
#else
    extern int verifyExfat (struct instance *);
    extern int compareBootSec (struct instance *);
#endif

#endif