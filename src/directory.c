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

 


 

/*

* Decode the Cluster for deirectory information

* decode_cluster (void * memBase, unsigned int Cluster, exfile ** file, unsigned int * file_count, unsigned int * current_file)

* Returns EXIT_SUCCESS if no errors encountered

* Returns EXIT_FAILURE and prints appropriate error

* Options

* memBase       [void *]       - Pointer to start of file memory

* Cluster       [unsigned int] - The cluster that should be decoded for directory information

* file          [exfile **]    - A pointer to a dynamically allocated file information structure

* file_count    [unsigned int] - The number of elements in the dynamic file information structure

* current_file  [unsigned int] - The current element to decode into

*/

 

int decode_cluster (void * memBase, unsigned int Cluster, exfile ** file, unsigned int * file_count, unsigned int * current_file)

{

    exfile * files = *file;

    directoryEntry * dirEntry = memBase;

    for (unsigned int i = 0; i < (Cluster / 32); i++)

    {

        if (isZero(dirEntry[i].entryType)) continue;

        // Decode the data by performing a lot of castings

        switch (dirEntry[i].entryType)

        {

            case 0x81: // Allocation Bitmap

            case 0x82: // Upper Case Lookup Table

            case 0x83: // Volume Label

                break;

            case 0x85: // File

                *current_file = *file_count - 1;

                files[*current_file].attributes = (void *)(&dirEntry[i].data[3]);

                files[*current_file].creation = (void *)(&dirEntry[i].data[7]);

                files[*current_file].modify = (void *)(&dirEntry[i].data[11]);

                files[*current_file].access = (void *)(&dirEntry[i].data[15]);

                files[*current_file].checksum = *(u_int16_t *)(&dirEntry[i].data[1]);

                files[*current_file].accressDeciSeconds = 0; //*(u_int8_t *)((void *)(&dirEntry[i].entryType)+20);

                files[*current_file].modifyDeciSeconds = 0; //*(u_int8_t *)((void *)(&dirEntry[i].entryType)+21);

                // Increase the directory size to always size +1 for safety

                files = realloc (files, sizeof (exfile) * *file_count);

                *file_count += 1;

                if (isNull(files))

                {

                    // Can't free what completely failed

                    perror ("realloc");

                    return EXIT_FAILURE;

                }

                break;

            case 0xC0: // File Stream

                files[*current_file].cluster = dirEntry[i].firstCluster;

                files[*current_file].nameLength = dirEntry[i].data[2];

                files[*current_file].length = *(u_int64_t *)(&dirEntry[i].data[7]);

                (files[*current_file]).nameHash = *(u_int16_t *)(&dirEntry[i].data[3]);

                break;

            case 0xC1: // File Name

                bzero (files[*current_file].filename, 256);

                for (int j = 1; (j < 30) && !isZero(*(u_int16_t *)(&dirEntry[i].data[j])); j += 2)

                {

                    files[*current_file].filename[j / 2] = (char)(*(u_int16_t *)(&dirEntry[i].data[j]));

                }                   

                break;

        }

    }

    // Since we keep reallocating memory, update the original pointer to point to the latest copy

    *file = files;

    return EXIT_SUCCESS;

}

 

/*

* Print the root directory structure

* int directoryPrint (struct instance * inst)

* Returns EXIT_SUCCESS if no errors encountered

* Returns EXIT_FAILURE and prints appropriate error

* Options

* inst [struct instance *] - Pointer to the main memory for the instance

*/

int directoryPrint (fileInfo * inst)

{

    u_int32_t * fatMain;

    unsigned int Cluster = (1 << inst->M_Boot->BytesPerSectorShift) * (1 << inst->M_Boot->SectorsPerClusterShift);

    unsigned int FirstClusterOffset = (1 << inst->M_Boot->BytesPerSectorShift) * inst->M_Boot->ClusterHeapOffset;

    unsigned int CurrentCluster = 0;

    fatMain = (void *)(inst->Data + (1 << inst->M_Boot->BytesPerSectorShift) * inst->M_Boot->FatOffset);

    // Clusters are 1 based numbers, not 0 based

    CurrentCluster = inst->M_Boot->FirstClusterOfRootDirectory;

    exfile * files;

    unsigned int file_count = 1, current_file = 0;

    files = calloc (1, sizeof (exfile));

    decode_cluster ((void *)(inst->Data + FirstClusterOffset + Cluster * (CurrentCluster - 2)), Cluster, &files, &file_count, &current_file);

    while (fatMain[CurrentCluster] != (u_int32_t)(-1))

    {

        CurrentCluster = fatMain[CurrentCluster];

        decode_cluster ((void *)(inst->Data + FirstClusterOffset + Cluster * (CurrentCluster - 2)), Cluster, &files, &file_count, &current_file);       

    }

    fprintf (stdout, "Entries Found in Root Directory: %d\n", file_count - 1);

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

    // scan for next cluster, retry if found, leave and cleanup otherwise

    free (files);

    return EXIT_SUCCESS;

}