#include <ctype.h>
#include <extfat.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

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
// void compareMemory(bool u, struct instance * v, struct instance * w, [struct insance member, doesn't translate] x, int y, char * z)
// {
//    if (memcmp (&(v->x), &(w->x), y) != 0)
//    {
//        OK = false;
//        printf ("%s: Field %s is inconsistant and too messy to verbose print\n", inst->function, z);
//        OK = u;
//    }
// }
#define compareMemory(u, v, w, x, y, z) { if (memcmp (&(v->x), &(w->x), y) != 0) {OK = false; printf ("%s: Field %s is inconsistant and too messy to verbose print\n", inst->function, z); OK = u; }}
// Inline version of:
// void compareValues(bool v, struct instance * w, struct instance * x, [struct insance member, doesn't translate] y, char * z)
// {
//     if (w->y != x->y)
//     {
//         OK = false;
//         printf ("%s: Field %s is inconsistant [Main:%ld != %ld:Backup]\n", inst->function, z, (__uint64_t)(w->y), (__uint64_t)(x->y));
//         OK = v;
//     }
// }
#define compareValues(v, w, x, y, z) { if (w->y != x->y) {OK = false; printf ("%s: Field %s is inconsistant [Main:%ld != %ld:Backup]\n", inst->function, z, (__uint64_t)(w->y), (__uint64_t)(x->y)); OK = v; }}
#define limitCheck(x,y,z) ((x >= y) && (x <= z)) // Boolean test for x >= y >= z

// Map the files from the main memory structure
int mapFile (struct instance * inst)
{
    int val = 0;
    if (isNull(inst)) return EXIT_FAILURE;
    inst->fdInput = open(inst->ivalue, O_RDWR);
    fstat (inst->fdInput, &(inst->inFile));
    inst->memInput = mmap (NULL, inst->inFile.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, inst->fdInput, 0);
    if (inst->memInput == MAP_FAILED)
    {
        printf ("%s: Cannot map input file to memory\n", inst->function);
        return EXIT_FAILURE;
    }
    inst->bootSectorMain = inst->memInput;
    val = (1 << inst->bootSectorMain->bytesPerSectorShift) * 12;
    inst->bootSectorBackup = ((void *)(inst->memInput) + val);
    if (compareBootSec (inst) == EXIT_FAILURE)
    {
        msync (inst->memInput, inst->inFile.st_size, MS_SYNC);
        munmap (inst->memInput, inst->inFile.st_size);
        return EXIT_FAILURE;
    }
    if (isZero (strcmp (inst->ivalue, inst->ovalue)))
    {
        inst->memOutput = inst->memInput;
        return EXIT_SUCCESS;
    }
    inst->fdOutput = open(inst->ovalue, O_WRONLY);
    inst->memOutput = mmap (NULL, inst->inFile.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, inst->fdOutput, 0);
    if (inst->memOutput == MAP_FAILED)
    {
        printf ("%s: Cannot map output file to memory\n", inst->function);
        return EXIT_FAILURE;
    }
    if (!limitCheck(inst->bootSectorMain->bytesPerSectorShift, 5, 9))
    {
        fprintf (stderr, "%s: Illegal value of %d for BytesPerSectorShift\n", inst->function, inst->bootSectorMain->bytesPerSectorShift);
        return EXIT_FAILURE; 
    }
    if (isNEQ(inst->bootSectorMain->jumpBoot[0], 0xEB) && \
        isNEQ(inst->bootSectorMain->jumpBoot[1], 0x76) && \
        isNEQ(inst->bootSectorMain->jumpBoot[2], 0x90))
    {
        fprintf (stderr, "%s: JumpBoot value [%02x][%02x][%02x] is invalid\n", inst->function, inst->bootSectorMain->jumpBoot[0],\
        inst->bootSectorMain->jumpBoot[1], inst->bootSectorMain->jumpBoot[2]);
        return EXIT_FAILURE;
    }
    for (int i = 0; i < 390; i++)
    {
        if (isNEQ(inst->bootSectorMain->mustBeZero[i], 0))
        {
            fprintf (stderr, "%s: MustBeZero value is not zero at offset %d", inst->function, i);
            return EXIT_FAILURE;
        }
    }
    if (inst->bootSectorMain->volumeLength < (1 << (20 - inst->bootSectorMain->bytesPerSectorShift)))
    {
        fprintf (stderr, "%s: VolumeLength field is too small [%ld]", inst->function, inst->bootSectorMain->volumeLength);
        return EXIT_FAILURE;
    }
    if (!limitCheck(inst->bootSectorMain->fatOffset, 24, \
        inst->bootSectorMain->clusterHeapOffset - (inst->bootSectorMain->fatLength * inst->bootSectorMain->numberOfFats)))
    {
        fprintf (stderr, "%s: FatOffset field is invalid [%d]", inst->function, inst->bootSectorMain->fatOffset);
        return EXIT_FAILURE;
    }
    if (!limitCheck(inst->bootSectorMain->fatLength, \
        (inst->bootSectorMain->clusterCount + 2) * (1 << (inst->bootSectorMain->bytesPerSectorShift - 2)), \
        (inst->bootSectorMain->clusterHeapOffset - inst->bootSectorMain->fatOffset) / inst->bootSectorMain->numberOfFats))
    {
        fprintf (stderr, "%s: FatLength field is invalid [%d]", inst->function, inst->bootSectorMain->fatLength);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

// Unmap the files from the main memory structure
int unmapFile (struct instance * inst)
{
    setFunction (inst);
    msync (inst->memInput, inst->inFile.st_size, MS_SYNC); // Merged from Phu
    munmap (inst->memInput, inst->inFile.st_size);
    close (inst->fdInput);
    msync (inst->memOutput, inst->outFile.st_size, MS_SYNC); // Merged from Phu
    munmap (inst->memInput, inst->inFile.st_size);
    close (inst->fdOutput);
    return EXIT_SUCCESS;
}

// Read the file from the main memory structure, input only
int readFile (struct instance * inst)
{
    if (isNull(inst)) return EXIT_FAILURE;
    return EXIT_SUCCESS;
}

// Write the file from the main memory structure, output only
int writeFile (struct instance * inst)
{
    if (isNull(inst)) return EXIT_FAILURE;
    return EXIT_SUCCESS;
}

// Open Files
int openExfat (struct instance * inst)
{
    setFunction (inst);
    inst->fdInput = open(inst->ivalue, O_RDONLY);
    inst->fdOutput = open(inst->ovalue, O_WRONLY);
    if (readBootSector (inst) == EXIT_FAILURE)
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
    free (inst->bootSectorMain);
    free (inst->bootSectorBackup);
    close (inst->fdInput);
    close (inst->fdOutput);
}

// Initial Boot Sector Read (First 512 bytes)
// Need to transition to fread
// Need to fix various small errors
int readBootSector (struct instance * inst)
{
    int val = 0;
    setFunction (inst);
    inst->bootSectorMain = calloc (1, sizeof (struct bootSector));
    if (isNull(inst->bootSectorMain))
    {
        return EXIT_FAILURE;
    }
    inst->bootSectorBackup = calloc (1, sizeof (struct bootSector));
    if (isNull(inst->bootSectorBackup))
    {
        free (inst->bootSectorMain);
        return EXIT_FAILURE;
    }
    if (read (inst->fdInput, inst->bootSectorMain, 512) !=  512)
    {
        closeExfat (inst);
        setFunction (inst);
        printf ("%s: Cannot read %d bytes at offset %d\n ", inst->function, 512, 0);
        return EXIT_FAILURE;
    }
    if (!limitCheck(inst->bootSectorMain->bytesPerSectorShift, 5, 9))
    {
        closeExfat (inst);
        setFunction (inst);
        printf ("%s: Illegal value of %d for BytesPerSectorShift\n", inst->function, inst->bootSectorMain->bytesPerSectorShift);
        return EXIT_FAILURE;
    }
    val = (1 << inst->bootSectorMain->bytesPerSectorShift) * 12;
    if (lseek (inst->fdInput, val, SEEK_SET) != val)
    {
        closeExfat (inst);
        setFunction (inst);
        printf ("%s: Cannot seek to byte offset %d\n ", inst->function, val);
        return EXIT_FAILURE;
    }
    if (read (inst->fdInput, inst->bootSectorBackup, 512) !=  512)
    {
        closeExfat (inst);
        setFunction (inst);
        printf ("%s: Cannot read %d bytes at offset %d\n ", inst->function, 512, val);
        return EXIT_FAILURE;
    }
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
    compareMemory (false, inst->bootSectorMain, inst->bootSectorBackup, jumpBoot, 3, "JumpBoot");
    compareMemory (false, inst->bootSectorMain, inst->bootSectorBackup, fileSystemName, 8, "FileSystemName");
    compareMemory (false, inst->bootSectorMain, inst->bootSectorBackup, mustBeZero, 53, "MustBeZero");
    compareValues (false, inst->bootSectorMain, inst->bootSectorBackup, partitionOffset, "PartitionOffset");
    compareValues (false, inst->bootSectorMain, inst->bootSectorBackup, volumeLength, "VolumeLength");
    compareValues (false, inst->bootSectorMain, inst->bootSectorBackup, fatOffset, "FatOffset");
    compareValues (false, inst->bootSectorMain, inst->bootSectorBackup, fatLength, "FatLength");
    compareValues (false, inst->bootSectorMain, inst->bootSectorBackup, clusterHeapOffset, "ClusterHeapOffset");
    compareValues (false, inst->bootSectorMain, inst->bootSectorBackup, clusterCount, "ClusterCount");
    compareValues (false, inst->bootSectorMain, inst->bootSectorBackup, firstClusterOfRootDirectory, "FirstClusterOfRootDirectory");
    compareValues (false, inst->bootSectorMain, inst->bootSectorBackup, volumeSerialNumber, "VolumeSerialNumber");
    compareValues (false, inst->bootSectorMain, inst->bootSectorBackup, fileSystemRevision, "FileSystemRevision");
    compareValues (false, inst->bootSectorMain, inst->bootSectorBackup, fileSystemRevision, "FileSystemRevision");
    compareValues (true, inst->bootSectorMain, inst->bootSectorBackup, volumeFlags, "VolumeFlags is implementation specic and can be ignored if it");
    compareValues (false, inst->bootSectorMain, inst->bootSectorBackup, bytesPerSectorShift, "BytesPerSectorShift");
    compareValues (false, inst->bootSectorMain, inst->bootSectorBackup, sectorsPerClusterShift, "SectorsPerClusterShift");
    compareValues (false, inst->bootSectorMain, inst->bootSectorBackup, numberOfFats, "NumberOfFats");
    compareValues (false, inst->bootSectorMain, inst->bootSectorBackup, driveSelect, "DriveSelect");
    compareValues (false, inst->bootSectorMain, inst->bootSectorBackup, percentInUse, "percentInUse");
    compareMemory (false, inst->bootSectorMain, inst->bootSectorBackup, reserved, 7, "Reserved");
    compareMemory (false, inst->bootSectorMain, inst->bootSectorBackup, bootCode, 390, "BootCode");
    compareMemory (false, inst->bootSectorMain, inst->bootSectorBackup, bootSignature, 2, "BootSignature");
    return (OK ? EXIT_SUCCESS : EXIT_FAILURE);
}

int main(int argc, char ** argv)
{
    char * help[] = {
        "extfat utility",
        "    Usage: extfat -i inputFile -o outputFile -c -h ",
        "",
        "           -i xxx    where xxx is the input file name [This is optional, but -i test.image is implied if not specified]",
        "           -o xxx    where xxx is the output file number [This is optional, inputFile will be used if not specified]",
        "           -c        triggers the copying of input to output (This is optional)",
        "           -m        use mmap for file access. [implied if -f and -m not specified]", // Added from Rency
        "           -f        use fread for file access", // Addded from Rency
        "           -v        verify exfat image", // Added from Rency
        "           -h        is this help message",
        NULL
    };    
    int i = 0;
    struct instance exfat;
    setFunction ((&exfat));
    exfat.iflag = false;
    exfat.oflag = false;
    exfat.cflag = false;
    exfat.vflag = false;
    exfat.fflag = false;
    exfat.mflag = false;
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
            case 'c':
                exfat.cflag = true;
                break;
            case 'i':
                exfat.iflag = true;
                exfat.ivalue = optarg;
                break;
            case 'o':
                exfat.oflag = true;
                exfat.ovalue = optarg;
                break;
            case ':':
                if (optopt == 'i' || optopt == 'o')
                fprintf (stderr, "Option requires an argument.\n");
                return EXIT_FAILURE;
            case 'm': // Added from Rency
                exfat.mflag = true;
                break;
            case 'f': // Added from Rency
                exfat.fflag = true;
                break;
            case 'v': // Added from Rency
                exfat.vflag = true;
                break;
            case 'h':
                while (help[i] != NULL)
                {
                    fprintf (stderr, "%s\n", help[i]);
                    i++;
                }
                return EXIT_SUCCESS;
            case '?':
                fprintf (stderr, "Option not recognized\n");
                return EXIT_FAILURE;
            default:;
        }
    }
    if (isFalse(exfat.iflag)) exfat.ivalue = "test.image";
    if (isFalse(exfat.oflag)) exfat.ovalue = exfat.ivalue;
    if (isFalse(exfat.fflag) && isFalse(exfat.mflag)) exfat.mflag = true;
    if (isTrue(exfat.fflag) && isTrue(exfat.mflag)) // Added from Rency
    {
        fprintf (stderr, "-f and -m options are mutually exclusive");
        return EXIT_FAILURE;
    }
    if (mapFile (&exfat) == EXIT_FAILURE)
    {
        unmapFile (&exfat);
        return EXIT_FAILURE;
    }
    unmapFile (&exfat);
    return EXIT_SUCCESS;
}
