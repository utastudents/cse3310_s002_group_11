#include <ctype.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

// Insert constants here

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

// Insert more structures here

// Main memory structure
struct instance
{
    bool iflag;
    bool oflag;
    bool cflag;
    int fdInput;
    int fdOutput;
    int opt;
    char * ivalue;
    char * ovalue;
    struct stat inFile;
    struct stat outFile;
    void * memInput;
    void * memOutput;
    char * function;
    struct bootSector * bootSectorMain; // Sector 0
    struct bootSector * bootSectorBackup; // Sector 12
};

// Function Declarations
int mapFile (struct instance *);
int readFile (struct instance *);
int writeFile (struct instance *);
int openExfat (struct instance *);
void closeExfat (struct instance *);
int readBootSector (struct instance *);
int compareBootSec (struct instance *);

// Macro Declarations
#define setFunction(x) x->function=__func__
#define compareMemory(v, w, x, y, z) { if (memcmp (v->x, w->x, y) != 0) {OK = false; printf ("%s: Field %s is inconsistant", inst->function, z); }}
#define compareValues(w, x, y, z) { if (w->y != x->y) {OK = false; printf ("%s: Field %s is inconsistant", inst->function, z); }}

// Map the files from the main memory structure
int mapFile (struct instance * inst)
{
    return EXIT_SUCCESS;
}

// Read the file from the main memory structure, input only
int readFile (struct instance * inst)
{
    return EXIT_SUCCESS;
}

// Write the file from the main memory structure, output only
int writeFile (struct instance * inst)
{
    return EXIT_SUCCESS;
}

// Open Files
int openExfat (struct instance * inst)
{
    int val = 0;
    setFunction (inst);
    inst->fdInput = open(inst->ivalue, O_RDONLY);
    inst->fdOutput = open(inst->ovalue, O_WRONLY);
    if ((val = readBootSector (inst)) == EXIT_FAILURE)
    {
        return EXIT_FAILURE;
    }
    setFunction (inst);
    return EXIT_SUCCESS;
}

// Close Files
void closeExfat (struct instance * inst)
{
    setFunction (inst);
    close (inst->fdInput);
    close (inst->fdOutput);
}

// Initial Boot Sector Read (First 512 bytes)
int readBootSector (struct instance * inst)
{
    int val = 0;
    setFunction (inst);
    inst->bootSectorMain = calloc (1, sizeof (struct bootSector));
    if (inst->bootSectorMain == NULL)
    {
        return EXIT_FAILURE;
    }
    inst->bootSectorBackup = calloc (1, sizeof (struct bootSector));
    if (inst->bootSectorBackup == NULL)
    {
        free (inst->bootSectorMain);
        return EXIT_FAILURE;
    }
    if ((val = read (inst->fdInput, inst->bootSectorMain, 512)) !=  512)
    {
        closeExfat (inst);
        setFunction (inst);
        return EXIT_FAILURE;
    }
    if (inst->bootSectorMain->bytesPerSectorShift)
    if ((val = compareBootSec (inst)) == EXIT_FAILURE)
    {
        setFunction (inst);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

// Compare the boot sectors from the main memory structure
int compareBootSec (struct instance * inst)
{
    bool OK = true;
    setFunction (inst);
    compareMemory (inst->bootSectorMain, inst->bootSectorBackup, jumpBoot, 3, "JumpBoot");
    compareMemory (inst->bootSectorMain, inst->bootSectorBackup, fileSystemName, 8, "FileSystemName");
    compareMemory (inst->bootSectorMain, inst->bootSectorBackup, mustBeZero, 53, "MustBeZero");
    compareValues (inst->bootSectorMain, inst->bootSectorBackup, partitionOffset, "PartitionOffset");
    compareValues (inst->bootSectorMain, inst->bootSectorBackup, volumeLength, "VolumeLength");
    compareValues (inst->bootSectorMain, inst->bootSectorBackup, fatOffset, "FatOffset");
    compareValues (inst->bootSectorMain, inst->bootSectorBackup, fatLength, "FatLength");
    compareValues (inst->bootSectorMain, inst->bootSectorBackup, clusterHeapOffset, "ClusterHeapOffset");
    compareValues (inst->bootSectorMain, inst->bootSectorBackup, clusterCount, "clusterCount");
    compareValues (inst->bootSectorMain, inst->bootSectorBackup, firstClusterOfRootDirectory, "FirstClusterOfRootDirectory");
    compareValues (inst->bootSectorMain, inst->bootSectorBackup, volumeSerialNumber, "VolumeSerialNumber");
    compareValues (inst->bootSectorMain, inst->bootSectorBackup, fileSystemRevision, "FileSystemRevision");
    compareValues (inst->bootSectorMain, inst->bootSectorBackup, fileSystemRevision, "FileSystemRevision");
    compareValues (inst->bootSectorMain, inst->bootSectorBackup, volumeFlags, "VolumeFlags");
    compareValues (inst->bootSectorMain, inst->bootSectorBackup, bytesPerSectorShift, "BytesPerSectorShift");
    compareValues (inst->bootSectorMain, inst->bootSectorBackup, sectorsPerClusterShift, "SectorsPerClusterShift");
    compareValues (inst->bootSectorMain, inst->bootSectorBackup, numberOfFats, "NumberOfFats");
    compareValues (inst->bootSectorMain, inst->bootSectorBackup, driveSelect, "DriveSelect");
    compareValues (inst->bootSectorMain, inst->bootSectorBackup, percentInUse, "percentInUse");
    compareMemory (inst->bootSectorMain, inst->bootSectorBackup, reserved, 7, "Reserved");
    compareMemory (inst->bootSectorMain, inst->bootSectorBackup, bootCode, 390, "BootCode");
    compareMemory (inst->bootSectorMain, inst->bootSectorBackup, bootSignature, 2, "BootSignature");
    return (OK ? EXIT_SUCCESS : EXIT_FAILURE);
}

int main(int argc, char ** argv)
{
    char * help[] = {
        "This is help line 1.",
        "This is help line 2.",
        "This is help line 3.",
        "This is help line 4.",
        "This is help line 5.",
        "This is help line 6.",
        NULL
    };    
    struct instance exfat;
    setFunction ((&exfat));
    exfat.iflag = false;
    exfat.oflag = false;
    exfat.cflag = false;
    exfat.fdInput = -1;
    exfat.fdOutput = -1;
    exfat.opt = -1;
    exfat.ivalue = NULL;
    exfat.ovalue = NULL;
    bzero (&exfat.inFile, sizeof (struct stat));
    bzero (&exfat.outFile, sizeof (struct stat));
    exfat.memInput = NULL;
    while ((exfat.opt = getopt (argc, argv, "i:co:h")) != -1)
    {
        switch (exfat.opt)
        {
            case 'c': exfat.cflag = true; break;
            case 'i': exfat.iflag = true; exfat.ivalue = optarg; break;
            case 'o': exfat.oflag = true; exfat.ovalue = optarg; break;
            case ':':
                if (optopt == 'i' || optopt == 'o')
                fprintf (stderr, "Option requires an argument.\n");
                return EXIT_FAILURE;
            case 'h': while (help[0] != NULL) { fprintf (stderr, "%s\n", help[0]); help[0] = help[1]; } break;
            case '?': fprintf (stderr, "Option not recognized\n"); break;
            default:;
        }
    }
    return EXIT_SUCCESS;
}
