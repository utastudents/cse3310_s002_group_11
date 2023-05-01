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
#include <extfat.h>

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

int decode_cluster (void * memBase, unsigned int Cluster, exfile ** file, unsigned int * file_count, unsigned int * current_file, fileInfo * inst)
{
    exfile * files = *file;
    directoryEntry * dirEntry = memBase;
    unsigned char index = 0;
    for (unsigned int i = 0; i < (Cluster / 32); i++)
    {
        if (isZero(dirEntry[i].raw.data[0])) continue;
        // Decode the data by performing a lot of castings
        switch (dirEntry[i].raw.data[0])
        {
            case 0x81: // Allocation Bitmap
                inst->allocationBitmap = dirEntry[i].raw.bitmap.cluster;
                break;

            case 0x82: // Upper Case Lookup Table
                break;

            case 0x83: // Volume Label
                break;

            case 0x85: // File
                index = 0;
                *current_file = *file_count - 1;
                bzero (files[*current_file].directoryFile, 256 * sizeof (unsigned char *));
                files[*current_file].attributes = &dirEntry[i].raw.file.attributes;
                files[*current_file].creation = &dirEntry[i].raw.file.create;
                files[*current_file].modify = &dirEntry[i].raw.file.lastModified;
                files[*current_file].access = &dirEntry[i].raw.file.lastAccessed;
                files[*current_file].checksum = dirEntry[i].raw.file.checksum;
                files[*current_file].createDeciSeconds = dirEntry[i].raw.file.create10ms;
                files[*current_file].modifyDeciSeconds = dirEntry[i].raw.file.modify10ms;
                files[*current_file].directoryFile[index] = (void *)dirEntry[i].raw.data; // Preserve the location of the directory entry
                // Increase the directory size to always size +1 for safety
                *file_count += 1;
                files = realloc (files, sizeof (exfile) * *file_count);
                if (isNull(files))
                {
                    // Can't free what completely failed
                    perror ("realloc");
                    return EXIT_FAILURE;
                }
                break;

            case 0xC0: // File Stream
                index++;
                files[*current_file].cluster = dirEntry[i].raw.stream.firstCluster;
                files[*current_file].nameLength = dirEntry[i].raw.stream.nameLength;
                files[*current_file].length = dirEntry[i].raw.stream.dataLength;
                files[*current_file].nameHash = dirEntry[i].raw.stream.nameHash;
                files[*current_file].directoryFile[index] = dirEntry[i].raw.data;
                break;

            case 0xC1: // File Name
                index++;
                bzero (files[*current_file].filename, 256);
                for (int j = 0; (j < 30) && !isZero(*(u_int16_t *)(&dirEntry[i].raw.filename.filename[j])); j += 2)
                    files[*current_file].filename[j / 2] = dirEntry[i].raw.filename.filename[j];
                files[*current_file].directoryFile[index] = dirEntry[i].raw.data;
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
    unsigned int Cluster = getClusterSize(inst);
    unsigned int FirstClusterOffset = getFirstCluster(inst);
    unsigned int CurrentCluster = inst->M_Boot->FirstClusterOfRootDirectory - 2, file_count = 1, current_file = 0;
    char * months[] = { "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };
    fatMain = (void *)getFATStart(inst);
    exfile * files;
    files = calloc (1, sizeof (exfile));
    while (true)
    {
        decode_cluster ((void *)(inst->Data + FirstClusterOffset + Cluster * CurrentCluster), Cluster, &files, &file_count, &current_file, inst);           
        CurrentCluster = fatMain[CurrentCluster];
        if (CurrentCluster == (u_int32_t)(-1)) break;
    }
    fprintf (stdout, "Entries Found in Root Directory: %u\n", file_count - 1);
    for (unsigned int i = 0; i < (file_count - 1); i++)
    {
        fprintf (stdout, "%-30s ", files[i].filename);
        fprintf (stdout, "%s%s%s%s%s ", \
            files[i].attributes->readOnly ? "R" : "_", \
            files[i].attributes->hidden ? "H" : "_", \
            files[i].attributes->system ? "S" : "_", \
            files[i].attributes->directory ? "D" : "_", \
            files[i].attributes->archive ? "A" : "_");
        if (files[i].attributes->directory) fprintf (stdout, "%-21s", "< DIRECTORY >");
        else fprintf (stdout, "%*lu ", -20, files[i].length);
        fprintf (stdout, "%s %02u %04u ", months[files[i].modify->month], files[i].modify->day, files[i].modify->year + 1980);
        fprintf (stdout, "%u:%u:%u\n", files[i].modify->hour, files[i].modify->minute, ((files[i].modify->doublesecs * 20) + files[i].modifyDeciSeconds) / 10);
    }
    // scan for next cluster, retry if found, leave and cleanup otherwise
    free (files);
    return EXIT_SUCCESS;
}

/*
* Delete file or directory from the root directory structure (Does not handle recursion)
* int deleteFile (struct instance * inst)
* Returns EXIT_SUCCESS if no errors encountered
* Returns EXIT_FAILURE and prints appropriate error
* Options
* inst [struct instance *] - Pointer to the main memory for the instance
*/

int deleteFile (fileInfo * inst)
{
    u_int32_t * fatMain;
    bool found = false;
    unsigned int Cluster = getClusterSize(inst);
    unsigned int FirstClusterOffset = getFirstCluster(inst);
    unsigned int CurrentCluster = inst->M_Boot->FirstClusterOfRootDirectory - 2, file_count = 1, current_file = 0;
    exfile * files;
    fatMain = getFATStart (inst);
    files = calloc (1, sizeof (exfile));
    while (true)
    {
        decode_cluster ((void *)(inst->Data + FirstClusterOffset + Cluster * CurrentCluster), Cluster, &files, &file_count, &current_file, inst);           
        CurrentCluster = fatMain[CurrentCluster];
        if (CurrentCluster == (u_int32_t)(-1)) break;
    }
    for (unsigned int i = 0; i < file_count - 1; i++)
    {
        if (isZero(strcmp(files[i].filename,inst->Dvalue))) // Scan until we find the file
        {
            fprintf (stdout, "Deleting %s\n", files[i].filename);
            found = true; // Set flag if we found something
            unsigned char j = 0;
            while (true)
            {
                unsigned char * ptr = files[i].directoryFile[j];
                if (isNull(ptr)) break;
                j++; 
                ptr[0] &= 0x7F; // Unset the occupied bit
            }
            unsigned char * bitmap = inst->Data + getFirstCluster(inst) + getClusterSize(inst) * (inst->allocationBitmap - 2);
            fprintf (stdout, "inst->allocationBitmap = %u [%p]\n", inst->allocationBitmap, bitmap);
            fprintf (stdout, "files[%u].cluster = %u\n", i, files[i].cluster);
            CurrentCluster = files[i].cluster - 2;
            while (true)
            {
                fprintf (stdout, "CurrentCluster = %u\n", CurrentCluster);
                bitmap[CurrentCluster / 8] &= ~(1 << (CurrentCluster % 8)); // Unset the bitmap entries
                CurrentCluster = fatMain[CurrentCluster];
                if (CurrentCluster == (u_int32_t)(-1)) break;
                CurrentCluster -= 2;
            }
            break;
       } 
    }
    if (isFalse(found)) fprintf (stderr, "Nothing to delete\n"); 
    free (files);
    return EXIT_SUCCESS;
}