#define DIRECTORY_C

#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <locale.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <wchar.h>

#include "../include/extfat.h"

typedef struct
{
    u_int32_t doublesecs:5;
    u_int32_t minute:6;
    u_int32_t hour:5;
    u_int32_t day:5;
    u_int32_t month:4;
    u_int32_t year:7;
} timestamp;

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
    unsigned int modifyDeciSeconds;
    unsigned int accressDeciSeconds;
    unsigned long int length;
} exfile;

const char * months[] = { "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };

// Read directory structure
int directoryPrint (struct instance * inst)
{
    u_int32_t * fatMain;
    directoryEntry * dirEntry;
    unsigned int SectorSize = 1 << inst->bootSectorMain->BytesPerSectorShift;
    unsigned int ClusterSize = 1 << inst->bootSectorMain->SectorsPerClusterShift;
    unsigned int Cluster = SectorSize * ClusterSize;
    unsigned int ClusterHeapOffset = inst->bootSectorMain->ClusterHeapOffset;
    unsigned int FirstClusterOffset = SectorSize * ClusterHeapOffset;
    unsigned int FatOffset = inst->bootSectorMain->FatOffset;
    fatMain = (void *)(inst->memInput + SectorSize * FatOffset);
    unsigned int RootDirectory = inst->bootSectorMain->FirstClusterOfRootDirectory;
    unsigned int RootDirectoryOffset = FirstClusterOffset + Cluster * (fatMain[RootDirectory - 2] - 1);
    dirEntry = (void *)(inst->memInput + RootDirectoryOffset);
    exfile * files;
    unsigned int file_count = 1;
    unsigned int current_file = 0;
    files = calloc (1, sizeof (exfile));
    for (unsigned int i = 0; i < (Cluster / 32); i++)
    {
        if (isZero(dirEntry[i].entryType)) continue;
        switch (dirEntry[i].entryType)
        {
            case 0x81: // Allocation Bitmap
            case 0x82: // Upper Case Lookup Table
            case 0x83: // Volume Label
                break;
            case 0x85: // File
                current_file = file_count - 1;
                files[current_file].attributes = (void *)(&dirEntry[i].data[3]);
                files[current_file].creation = (void *)(&dirEntry[i].data[7]);
                files[current_file].modify = (void *)(&dirEntry[i].data[11]);
                files[current_file].access = (void *)(&dirEntry[i].data[15]);
                files[current_file].checksum = *(u_int16_t *)(&dirEntry[i].data[1]);
                files[current_file].accressDeciSeconds = *(u_int16_t *)(&dirEntry[i].data[19]);
                files[current_file].modifyDeciSeconds = *(u_int16_t *)(&dirEntry[i].data[20]);
                files = realloc (files, sizeof (exfile) * file_count++);
                break;
            case 0xC0: // File Stream
                files[current_file].cluster = dirEntry[i].firstCluster;
                files[current_file].nameLength = dirEntry[i].data[2];
                files[current_file].length = *(u_int64_t *)(&dirEntry[i].data[7]);
                files[current_file].nameHash = *(u_int16_t *)(&dirEntry[i].data[3]);
                break;
            case 0xC1: // File Name
                bzero (files[current_file].filename, 256);
                for (int j = 1; (j < 30) && !isZero(*(u_int16_t *)(&dirEntry[i].data[j])); j += 2)
                {
                    files[current_file].filename[j / 2] = (char)(*(u_int16_t *)(&dirEntry[i].data[j]));
                }                    
                break;
        }
    }
    fprintf (stdout, "File Count of %d\n", file_count - 1);
    for (unsigned int i = 0; i < file_count - 1; i++)
    {
        fprintf (stdout, "%-30s ", files[i].filename);
        fprintf (stdout, "%s%s%s%s%s ", \
            files[i].attributes->readOnly ? "R" : "_", \
            files[i].attributes->hidden ? "H" : "_", \
            files[i].attributes->system ? "S" : "_", \
            files[i].attributes->directory ? "D" : "_", \
            files[i].attributes->archive ? "A" : "_");
        if (files[i].attributes->directory) fprintf (stdout, "%-20s", "< DIRECTORY >");
        else fprintf (stdout, "%*lu ", -20, files[i].length);
        fprintf (stdout, "%s %02u %04u ", months[files[i].modify->month], files[i].modify->day, files[i].modify->year + 1980);
        fprintf (stdout, "%u:%u:%u", files[i].modify->hour, files[i].modify->minute, files[i].modify->doublesecs * 2 + files[i].modifyDeciSeconds);
        fprintf (stdout, "\n");
    }
    free (files);
    return EXIT_SUCCESS;
}