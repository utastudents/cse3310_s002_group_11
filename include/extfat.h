
#ifndef EXTFAT_H
#define EXTFAT_H

#include <inttypes.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

extern int ftruncate64 (int, __off64_t); // I shouldn't have to do this for a glibc function with a manpage

// Macro Declarations
#define isZero(x) (x == 0) // Boolean test for being 0
#define isNull(x) (x == NULL) // Boolean test for being NULL
#define isTrue(x) (x == true) // Boolean test for being true
#define isFalse(x) (x == false) // Boolean test for being false
#define isFault(x) (x == -1) // Boolean test for being -1
#define isNEQ(x,y) (x != y) // Boolean test for being unequal
#define isEQ(x,y) (x == y) // Boolean test for being equal
#define setFunction(x) x->function=__func__ // Set instance structure function to current function

// Math equations to cleanup code
#define getClusterSize(x) ((1 << x->M_Boot->BytesPerSectorShift) * (1 << x->M_Boot->SectorsPerClusterShift))
#define getFirstCluster(x) ((1 << x->M_Boot->BytesPerSectorShift) * x->M_Boot->ClusterHeapOffset)
#define getFATStart(x) (void *)(x->Data + (1 << x->M_Boot->BytesPerSectorShift) * x->M_Boot->FatOffset)
#define getAllocationTableOffset(x) (getFirstCluster(x) + getClusterSize(x) * (x->allocationBitmap - 2))

// Inline version of:
// void compareMemory(bool u, fileInfo * v, fileInfo * w, [fileInfo member, doesn't translate] x, int y, char * z)
// {
//    if (memcmp (&(v->x), &(w->x), y) != 0)
//    {
//        OK = false;
//        printf ("%s: Field %s is inconsistant and too messy to verbose print\n", inst->function, z);
//    }
// }
#define compareMemory(u, v, w, x, y, z) { if (memcmp (&(v->x), &(w->x), y) != 0) {OK = false; printf ("%s: Field %s is inconsistant and too messy to verbose print\n", inst->function, z); OK = u; }}
// Inline version of:
// void compareValues(bool v, fileInfo * w, fileInfo * x, [fileInfo member, doesn't translate] y, char * z)
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

// Main memory structure
typedef struct 
{
    Main_Boot *M_Boot;
    Main_Boot *B_Boot;
    void * FAT;
    void * Data; //void * memInput
    void * memOutput;
    char * filename; //char * ivalue
    char * ovalue;
    char * xvalue;
    char * Dvalue;
    int fd; //int fdInput
    int fdOutput;
    int SectorSize;
    int FileLength;
    bool iflag;
    bool oflag;
    bool cflag;
    bool vflag; 
    bool xflag;
    bool dflag;
    bool Dflag;
    int opt;
    struct stat inFile; 
    struct stat outFile;
    const char * function;
    unsigned int allocationBitmap; // Pointer to Allocation Bitmap
}fileInfo;

// From ExFAT specification
typedef struct
{
    u_int32_t doublesecs:5;
    u_int32_t minute:6;
    u_int32_t hour:5;
    u_int32_t day:5;
    u_int32_t month:4;
    u_int32_t year:7;
} timestamp;

// From ExFAT specification
typedef struct
{
    u_int16_t readOnly:1;
    u_int16_t hidden:1;
    u_int16_t system:1;
    u_int16_t reserved1:1;
    u_int16_t directory:1;
    u_int16_t archive:1;
    u_int16_t reserved2:10;
} attr;

// From ExFAT specification
typedef struct
{
    unsigned char entryType;
    unsigned char flags;
    unsigned char reserved[18];
    unsigned int cluster;
    unsigned long int length;
} type81;

// From ExFAT specification
typedef struct
{
    unsigned char entryType;
    unsigned char reserved1[3];
    unsigned int checksum;
    unsigned char reserved2[12];
    unsigned int cluster;
    unsigned long int length;
} type82;

// From ExFAT specification
typedef struct 
{
    unsigned char entryType;
    unsigned char secondaryCount;
    unsigned short int checksum;
    attr attributes;
    unsigned short int reserved1;
    timestamp create;
    timestamp lastModified;
    timestamp lastAccessed;
    unsigned char create10ms;
    unsigned char modify10ms;
    unsigned char createUTCOffset;
    unsigned char modifyUTCOffset;
    unsigned char accessUTCOffset;
    unsigned char reserved[7];
} type85;

// From ExFAT specification
typedef struct 
{   
    unsigned char entryType;
    unsigned char allocationPossible:1;
    unsigned char noFatChain:1;
    unsigned char customDefined:6;
    unsigned char reserved1;
    unsigned char nameLength;
    unsigned short int nameHash;
    unsigned char Reserved2[2];
    unsigned long int validDataLength;
    unsigned char Reserved3[4];
    unsigned int firstCluster;
    unsigned long int dataLength;
} typeC0;

// From ExFAT specification
typedef struct 
{
    unsigned char entryType;
    unsigned char secondaryCount;
    unsigned char filename[30];
} typeC1;

// Overlapping mapping of a directory entry via union
typedef struct 
{
    union
    {
        unsigned char data[32];
        type81 bitmap;
        type82 table;
        type85 file;
        typeC0 stream;
        typeC1 filename;
    } raw;
}directoryEntry;

// File information extracted from ExFAT directory entries
typedef struct
{
    char filename[256];
    unsigned int nameLength;
    unsigned int nameHash;
    attr * attributes;
    timestamp * modify;
    timestamp * creation;
    timestamp * access;
    unsigned int cluster;
    unsigned int checksum;
    unsigned char modifyDeciSeconds;
    unsigned char createDeciSeconds;
    unsigned long int length;
    void * directoryFile[256]; // Pointers to the start of all associated directory entries used to build this
    bool allocationPossible;
    bool noFatChain;
} exfile;

typedef struct
{
    unsigned char b0:1;
    unsigned char b1:1;
    unsigned char b2:1;
    unsigned char b3:1;
    unsigned char b4:1;
    unsigned char b5:1;
    unsigned char b6:1;
    unsigned char b7:1;
} binaryByte;

#ifdef EXTRACT_C
    int extractfile(fileInfo *);
    extern int decode_cluster(void *, unsigned int , exfile **, unsigned int *, unsigned int *, fileInfo *);
    extern unsigned int nextCluster (unsigned int *, unsigned int, bool, bool);
#else
    extern int extractfile(fileInfo *);
   
#endif

#ifdef DIRECTORY_C
    int directoryPrint(fileInfo *);
    int deleteFile (fileInfo *);
    int decode_cluster(void *, unsigned int , exfile **, unsigned int *, unsigned int *, fileInfo *);
    unsigned int nextCluster (unsigned int *, unsigned int, bool, bool);
#else
    extern int directoryPrint(fileInfo *);
    extern int deleteFile (fileInfo *);
#endif

#ifdef MMAP_C
    int mapFile (fileInfo *);
    int unmapFile (fileInfo *);
    int mmapCopy (fileInfo *);
#else
    extern int mapFile (fileInfo *);
    extern int unmapFile (fileInfo *);
    extern int mmapCopy (fileInfo *);
#endif

#ifdef CMDLINE_C
    int initInstance (fileInfo *);
    int fillInstance (fileInfo *, int, char **);
#else
    extern int initInstance (fileInfo *);
    extern int fillInstance (fileInfo *, int, char **);
#endif

#ifdef VERIFY_C
    int verifyExfat (fileInfo *);
    int compareBootSec (fileInfo *);
#else
    extern int verifyExfat (fileInfo *);
    extern int compareBootSec (fileInfo *);
#endif

#endif
