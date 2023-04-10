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
#include <routines.h>

int verifyExfat (fileInfo * inst)
{
    setFunction (inst);
    if (!limitCheck(inst->M_Boot->BytesPerSectorShift, 5, 9))
    {
        fprintf (stderr, "%s: Illegal value of %d for BytesPerSectorShift\n", inst->function, inst->M_Boot->BytesPerSectorShift);
        return EXIT_FAILURE; 
    }
    if (isNEQ(inst->M_Boot->JumpBoot[0], 0xEB) && \
        isNEQ(inst->M_Boot->JumpBoot[1], 0x76) && \
        isNEQ(inst->M_Boot->JumpBoot[2], 0x90))
    {
        fprintf (stderr, "%s: JumpBoot value [%02x][%02x][%02x] is invalid\n", inst->function, inst->M_Boot->JumpBoot[0],\
        inst->M_Boot->JumpBoot[1], inst->M_Boot->JumpBoot[2]);
        return EXIT_FAILURE;
    }
    if (!isZero(memcmp (inst->M_Boot->FileSystemName, "EXFAT   ", 8)))
    {
        fprintf (stderr, "%s: FileSystemName does not match \"EXFAT   \" [%c%c%c%c%c%c%c%c]\n", inst->function, \
        inst->M_Boot->FileSystemName[0], inst->M_Boot->FileSystemName[1], \
        inst->M_Boot->FileSystemName[2], inst->M_Boot->FileSystemName[3], \
        inst->M_Boot->FileSystemName[4], inst->M_Boot->FileSystemName[5], \
        inst->M_Boot->FileSystemName[6], inst->M_Boot->FileSystemName[7]);
        return EXIT_FAILURE;
    }
    for (int i = 0; i < 53; i++)
    {
        if (isNEQ(inst->M_Boot->MustBeZero[i], 0))
        {
            fprintf (stderr, \
                "%s: MustBeZero value [0x%02X] is not zero at offset %d <See exFat file system specification - Section 3.1.3>\n", \
                inst->function, inst->M_Boot->MustBeZero[i], i);
            return EXIT_FAILURE;
        }
    }
    if ((__uint64_t)(inst->M_Boot->VolumeLength) < (__uint64_t)(1 << (20 - inst->M_Boot->BytesPerSectorShift)))
    {
        fprintf (stderr, "%s: VolumeLength field is too small [%ld]\n", inst->function, inst->M_Boot->VolumeLength);
        return EXIT_FAILURE;
    }
    if (!limitCheck(inst->M_Boot->FatOffset, 24, \
        inst->M_Boot->ClusterHeapOffset - (inst->M_Boot->FatLength * inst->M_Boot->NumberOfFats)))
    {
        fprintf (stderr, "%s: FatOffset field is invalid [%d]\n", inst->function, inst->M_Boot->FatOffset);
        return EXIT_FAILURE;
    }
    if (!limitCheck(inst->M_Boot->FatLength, \
        (inst->M_Boot->ClusterCount + 2) / (1 << (inst->M_Boot->BytesPerSectorShift - 2)), \
        (inst->M_Boot->ClusterHeapOffset - inst->M_Boot->FatOffset) / inst->M_Boot->NumberOfFats))
    {
        fprintf (stderr, "%s: FatLength field is invalid [%d]\n", inst->function, inst->M_Boot->FatLength);
        return EXIT_FAILURE;
    }
    if (!limitCheck ((__uint32_t)inst->M_Boot->ClusterHeapOffset, \
         (__uint32_t)(inst->M_Boot->FatOffset + inst->M_Boot->FatLength * inst->M_Boot->NumberOfFats), (__uint32_t)(-1)))
    {
        fprintf (stderr, "%s: ClusterHeapOffset field is invalid [%d]\n", inst->function, inst->M_Boot->ClusterHeapOffset);
        return EXIT_FAILURE;
    }
    if (!limitCheck ((__uint32_t)inst->M_Boot->ClusterCount, \
         (__uint32_t)(inst->M_Boot->FatOffset + inst->M_Boot->FatLength * inst->M_Boot->NumberOfFats), (__uint32_t)(-1) - 11))
    {
        fprintf (stderr, "%s: ClusterCount field is invalid [%d]\n", inst->function, inst->M_Boot->ClusterCount);
        return EXIT_FAILURE;
    }
    if (!limitCheck(inst->M_Boot->FirstClusterOfRootDirectory, 2, inst->M_Boot->ClusterCount + 1))
    {
        fprintf (stderr, "%s: FirstClusterOfRootDirectory field is invalid [%d]\n", inst->function, inst->M_Boot->FirstClusterOfRootDirectory);
        return EXIT_FAILURE;
    }
    if (!limitCheck(inst->M_Boot->PercentInUse, 0, 100) && inst->M_Boot->PercentInUse != 0xFF)
    {
        fprintf (stderr, "%s: PercentInUse field is invalid [%d]", inst->function, inst->M_Boot->PercentInUse);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

// Compare the boot sectors from the main memory structure
int compareBootSec (fileInfo * inst)
{
    setFunction (inst);
    int bytesPerSector = 2 << (inst->M_Boot->BytesPerSectorShift - 1); // used to calc checksum
    UInt32 M_BootChecksum = BootChecksum((UCHAR*) inst->M_Boot, (short) bytesPerSector);
    UInt32 B_BootChecksum = BootChecksum((UCHAR*) inst->B_Boot, (short) bytesPerSector);
    printf("Checksums:\n(Main Boot) %x (Back Boot) %x\n",M_BootChecksum,B_BootChecksum);
    bool OK = (M_BootChecksum == B_BootChecksum);
    if (OK)
    {
        printf("File verified for use.\n");
        return EXIT_SUCCESS;
    }
    else
    {
        return EXIT_FAILURE;
    }
}
