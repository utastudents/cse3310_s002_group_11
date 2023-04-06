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
    if (!limitCheck(inst->bootSectorMain->BytesPerSectorShift, 5, 9))
    {
        fprintf (stderr, "%s: Illegal value of %d for BytesPerSectorShift\n", inst->function, inst->bootSectorMain->BytesPerSectorShift);
        return EXIT_FAILURE; 
    }
    if (isNEQ(inst->bootSectorMain->JumpBoot[0], 0xEB) && \
        isNEQ(inst->bootSectorMain->JumpBoot[1], 0x76) && \
        isNEQ(inst->bootSectorMain->JumpBoot[2], 0x90))
    {
        fprintf (stderr, "%s: JumpBoot value [%02x][%02x][%02x] is invalid\n", inst->function, inst->bootSectorMain->JumpBoot[0],\
        inst->bootSectorMain->JumpBoot[1], inst->bootSectorMain->JumpBoot[2]);
        return EXIT_FAILURE;
    }
    if (!isZero(memcmp (inst->bootSectorMain->FileSystemName, "EXFAT   ", 8)))
    {
        fprintf (stderr, "%s: FileSystemName does not match \"EXFAT   \" [%c%c%c%c%c%c%c%c]\n", inst->function, \
        inst->bootSectorMain->FileSystemName[0], inst->bootSectorMain->FileSystemName[1], \
        inst->bootSectorMain->FileSystemName[2], inst->bootSectorMain->FileSystemName[3], \
        inst->bootSectorMain->FileSystemName[4], inst->bootSectorMain->FileSystemName[5], \
        inst->bootSectorMain->FileSystemName[6], inst->bootSectorMain->FileSystemName[7]);
        return EXIT_FAILURE;
    }
    for (int i = 0; i < 53; i++)
    {
        if (isNEQ(inst->bootSectorMain->MustBeZero[i], 0))
        {
            fprintf (stderr, \
                "%s: MustBeZero value [0x%02X] is not zero at offset %d <See exFat file system specification - Section 3.1.3>\n", \
                inst->function, inst->bootSectorMain->MustBeZero[i], i);
            return EXIT_FAILURE;
        }
    }
    if ((u_int64_t)inst->bootSectorMain->VolumeLength < (__uint64_t)(1 << (20 - inst->bootSectorMain->BytesPerSectorShift)))
    {
        fprintf (stderr, "%s: VolumeLength field is too small [%ld]\n", inst->function, inst->bootSectorMain->VolumeLength);
        return EXIT_FAILURE;
    }
    if (!limitCheck(inst->bootSectorMain->FatOffset, 24, \
        inst->bootSectorMain->ClusterHeapOffset - (inst->bootSectorMain->FatLength * inst->bootSectorMain->NumberOfFats)))
    {
        fprintf (stderr, "%s: FatOffset field is invalid [%d]\n", inst->function, inst->bootSectorMain->FatOffset);
        return EXIT_FAILURE;
    }
    if (!limitCheck(inst->bootSectorMain->FatLength, \
        (inst->bootSectorMain->ClusterCount + 2) / (1 << (inst->bootSectorMain->BytesPerSectorShift - 2)), \
        (inst->bootSectorMain->ClusterHeapOffset - inst->bootSectorMain->FatOffset) / inst->bootSectorMain->NumberOfFats))
    {
        fprintf (stderr, "%s: FatLength field is invalid [%d]\n", inst->function, inst->bootSectorMain->FatLength);
        return EXIT_FAILURE;
    }
    if (!limitCheck((__uint32_t)inst->bootSectorMain->ClusterHeapOffset, \
        (__uint32_t)(inst->bootSectorMain->FatOffset + inst->bootSectorMain->FatLength * inst->bootSectorMain->NumberOfFats), (__uint32_t)(-1)))
    {
        fprintf (stderr, "%s: ClusterHeapOffset field is invalid [%d]\n", inst->function, inst->bootSectorMain->ClusterHeapOffset);
        return EXIT_FAILURE;
    }
    if (!limitCheck((__uint32_t)inst->bootSectorMain->ClusterCount, \
        (__uint32_t)(inst->bootSectorMain->FatOffset + inst->bootSectorMain->FatLength * inst->bootSectorMain->NumberOfFats), (__uint32_t)(-1) - 11))
    {
        fprintf (stderr, "%s: ClusterCount field is invalid [%d]\n", inst->function, inst->bootSectorMain->ClusterCount);
        return EXIT_FAILURE;
    }
    if (!limitCheck(inst->bootSectorMain->FirstClusterOfRootDirectory, 2, inst->bootSectorMain->ClusterCount + 1))
    {
        fprintf (stderr, "%s: FirstClusterofRootDirectory field is invalid [%d]\n", inst->function, inst->bootSectorMain->FirstClusterOfRootDirectory);
        return EXIT_FAILURE;
    }
    if (!limitCheck(inst->bootSectorMain->PercentInUse, 0, 100) && inst->bootSectorMain->PercentInUse != 0xFF)
    {
        fprintf (stderr, "%s: PercentInUse field is invalid [%d]", inst->function, inst->bootSectorMain->PercentInUse);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

// Compare the boot sectors from the main memory structure
int compareBootSec (struct instance * inst)
{
    bool OK = true;
    setFunction (inst);
    compareMemory (false, inst->bootSectorMain, inst->bootSectorBackup, JumpBoot, 3, "JumpBoot");
    compareMemory (false, inst->bootSectorMain, inst->bootSectorBackup, FileSystemName, 8, "FileSystemName");
    compareMemory (false, inst->bootSectorMain, inst->bootSectorBackup, MustBeZero, 53, "MustBeZero");
    compareValues (false, inst->bootSectorMain, inst->bootSectorBackup, PartitionOffset, "PartitionOffset");
    compareValues (false, inst->bootSectorMain, inst->bootSectorBackup, VolumeLength, "VolumeLength");
    compareValues (false, inst->bootSectorMain, inst->bootSectorBackup, FatOffset, "FatOffset");
    compareValues (false, inst->bootSectorMain, inst->bootSectorBackup, FatLength, "FatLength");
    compareValues (false, inst->bootSectorMain, inst->bootSectorBackup, ClusterHeapOffset, "ClusterHeapOffset");
    compareValues (false, inst->bootSectorMain, inst->bootSectorBackup, ClusterCount, "ClusterCount");
    compareValues (false, inst->bootSectorMain, inst->bootSectorBackup, FirstClusterOfRootDirectory, "FirstClusterOfRootDirectory");
    compareValues (false, inst->bootSectorMain, inst->bootSectorBackup, VolumeSerialNumber, "VolumeSerialNumber");
    compareValues (false, inst->bootSectorMain, inst->bootSectorBackup, FileSystemRevision, "FileSystemRevision");
    compareValues (false, inst->bootSectorMain, inst->bootSectorBackup, FileSystemRevision, "FileSystemRevision");
//    compareValues (true, inst->bootSectorMain, inst->bootSectorBackup, volumeFlags, "VolumeFlags is implementation specic and can be ignored if it");
    compareValues (false, inst->bootSectorMain, inst->bootSectorBackup, BytesPerSectorShift, "BytesPerSectorShift");
    compareValues (false, inst->bootSectorMain, inst->bootSectorBackup, SectorsPerClusterShift, "SectorsPerClusterShift");
    compareValues (false, inst->bootSectorMain, inst->bootSectorBackup, NumberOfFats, "NumberOfFats");
    compareValues (false, inst->bootSectorMain, inst->bootSectorBackup, DriveSelect, "DriveSelect");
    compareValues (false, inst->bootSectorMain, inst->bootSectorBackup, PercentInUse, "percentInUse");
    compareMemory (false, inst->bootSectorMain, inst->bootSectorBackup, Reserved, 7, "Reserved");
    compareMemory (false, inst->bootSectorMain, inst->bootSectorBackup, BootCode, 390, "BootCode");
    compareMemory (false, inst->bootSectorMain, inst->bootSectorBackup, BootSignature, 2, "BootSignature");
    return (OK ? EXIT_SUCCESS : EXIT_FAILURE);
}
