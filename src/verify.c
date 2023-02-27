#define VERIFY_C

#include <ctype.h>
#include <errno.h>
#include <extfat.h>
#include <fcntl.h>
#include <getopt.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <extfat.h>

int verifyExfat (struct instance * inst)
{
    setFunction (inst);
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
    if (!isZero(memcmp (inst->bootSectorMain->fileSystemName, "EXFAT   ", 8)))
    {
        fprintf (stderr, "%s: FileSystemName does not match \"EXFAT   \" [%c%c%c%c%c%c%c%c]\n", inst->function, \
        inst->bootSectorMain->fileSystemName[0], inst->bootSectorMain->fileSystemName[1], \
        inst->bootSectorMain->fileSystemName[2], inst->bootSectorMain->fileSystemName[3], \
        inst->bootSectorMain->fileSystemName[4], inst->bootSectorMain->fileSystemName[5], \
        inst->bootSectorMain->fileSystemName[6], inst->bootSectorMain->fileSystemName[7]);
        return EXIT_FAILURE;
    }
    for (int i = 0; i < 53; i++)
    {
        if (isNEQ(inst->bootSectorMain->mustBeZero[i], 0))
        {
            fprintf (stderr, \
                "%s: MustBeZero value [0x%02X] is not zero at offset %d <See exFat file system specification - Section 3.1.3>\n", \
                inst->function, inst->bootSectorMain->mustBeZero[i], i);
            return EXIT_FAILURE;
        }
    }
    if (inst->bootSectorMain->volumeLength < (__uint64_t)(1 << (20 - inst->bootSectorMain->bytesPerSectorShift)))
    {
        fprintf (stderr, "%s: VolumeLength field is too small [%ld]\n", inst->function, inst->bootSectorMain->volumeLength);
        return EXIT_FAILURE;
    }
    if (!limitCheck(inst->bootSectorMain->fatOffset, 24, \
        inst->bootSectorMain->clusterHeapOffset - (inst->bootSectorMain->fatLength * inst->bootSectorMain->numberOfFats)))
    {
        fprintf (stderr, "%s: FatOffset field is invalid [%d]\n", inst->function, inst->bootSectorMain->fatOffset);
        return EXIT_FAILURE;
    }
    if (!limitCheck(inst->bootSectorMain->fatLength, \
        (inst->bootSectorMain->clusterCount + 2) / (1 << (inst->bootSectorMain->bytesPerSectorShift - 2)), \
        (inst->bootSectorMain->clusterHeapOffset - inst->bootSectorMain->fatOffset) / inst->bootSectorMain->numberOfFats))
    {
        fprintf (stderr, "%s: FatLength field is invalid [%d]\n", inst->function, inst->bootSectorMain->fatLength);
        return EXIT_FAILURE;
    }
    if (!limitCheck(inst->bootSectorMain->clusterHeapOffset, \
        inst->bootSectorMain->fatOffset + inst->bootSectorMain->fatLength * inst->bootSectorMain->numberOfFats, (__uint32_t)(-1)))
    {
        fprintf (stderr, "%s: ClusterHeapOffset field is invalid [%d]\n", inst->function, inst->bootSectorMain->clusterHeapOffset);
        return EXIT_FAILURE;
    }
    if (!limitCheck(inst->bootSectorMain->clusterCount, \
        inst->bootSectorMain->fatOffset + inst->bootSectorMain->fatLength * inst->bootSectorMain->numberOfFats, (__uint32_t)(-1) - 11))
    {
        fprintf (stderr, "%s: ClusterCount field is invalid [%d]\n", inst->function, inst->bootSectorMain->clusterCount);
        return EXIT_FAILURE;
    }
    if (!limitCheck(inst->bootSectorMain->firstClusterOfRootDirectory, 2, inst->bootSectorMain->clusterCount + 1))
    {
        fprintf (stderr, "%s: FirstClusterofRootDirectory field is invalid [%d]\n", inst->function, inst->bootSectorMain->firstClusterOfRootDirectory);
        return EXIT_FAILURE;
    }
    if (!limitCheck(inst->bootSectorMain->percentInUse, 0, 100) && inst->bootSectorMain->percentInUse != 0xFF)
    {
        fprintf (stderr, "%s: PercentInUse field is invalid [%d]", inst->function, inst->bootSectorMain->percentInUse);
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
//    compareValues (true, inst->bootSectorMain, inst->bootSectorBackup, volumeFlags, "VolumeFlags is implementation specic and can be ignored if it");
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
