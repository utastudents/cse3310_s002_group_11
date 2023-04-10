
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

typedef struct 
{
    unsigned char entryType;
    unsigned char data[19];
    u_int32_t firstCluster;
    u_int64_t datalength;
}directoryEntry;


// Main memory structure
// Moved from extfat.c
typedef struct 
{
    Main_Boot *M_Boot;
    Main_Boot *B_Boot;
    void * FAT;
    void * Data; //void * memInput
    void * memOutput;
    char * filename; //char * ivalue
    char * ovalue;
    int fd; //int fdInput
    int fdOutput;
    int SectorSize;
    int FileLength;
    bool iflag;
    bool oflag;
    bool cflag;
    bool vflag;
    bool fflag;
    bool mflag;  
    bool dflag;
    int opt;
    struct stat inFile; 
    struct stat outFile;
    const char * function;
}fileInfo;

#ifdef DIRECTORY_C
    int directoryPrint(fileInfo *);
#else
    extern int directoryPrint(fileInfo *);
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