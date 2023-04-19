#define EXTRACT_C
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

#include "../include/extfat.h"




int extractfile (fileInfo * inst)

{

    u_int32_t * fatMain;

    unsigned int Cluster = (1 << inst->M_Boot->BytesPerSectorShift) * (1 << inst->M_Boot->SectorsPerClusterShift);

    unsigned int FirstClusterOffset = (1 << inst->M_Boot->BytesPerSectorShift) * inst->M_Boot->ClusterHeapOffset;

    unsigned int CurrentCluster = 0;

    inst->FAT = (void *)(inst->Data + (1 << inst->M_Boot->BytesPerSectorShift) * inst->M_Boot->FatOffset);
    fatMain=inst->FAT;
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

    for (unsigned int i = 0; i < file_count - 1; i++)

    {
         printf("\n\nFile:%s\n\n",files[i].filename);
         printf("\nLength:%ld %d \n\n",files[i].length,files[i].cluster);

        if isZero(strcmp(inst->xvalue,files[i].filename))
        {
            FILE *fp;
            printf("Ovalue :%s",inst->ovalue);
            fp=fopen(inst->ovalue,"w+");
             if (fp == NULL){
              perror("fopen");
            }
            printf("\n Open file!!!!\n\n");
            do{
              printf("\n DO while loop woorks!!!!\n\n");
              CurrentCluster=files[i].cluster;
              fwrite((void *)(inst->Data + FirstClusterOffset + Cluster * (CurrentCluster - 2)),(files[i].length>Cluster)?Cluster:files[i].length,1,fp);
              if(files[i].length<=Cluster)
              {
                files[i].length=0;
              }
              if(files[i].length>Cluster)
              {
                files[i].length-=Cluster;
                files[i].cluster=fatMain[CurrentCluster];
                printf("\n\nCLUSTER :%u\n\n",files[i].cluster);
                fflush(stdout); 

              }
            }while(files[i].length>0);

            fclose(fp);
        }

    }

    // scan for next cluster, retry if found, leave and cleanup otherwise

    free (files);

    return EXIT_SUCCESS;

}