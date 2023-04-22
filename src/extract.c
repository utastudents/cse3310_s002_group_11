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
    u_int32_t * fatMain; //acessing the FAT table
    unsigned int Cluster = getClusterSize(inst); 
    unsigned int FirstClusterOffset = getFirstCluster(inst);
    unsigned int CurrentCluster = inst->M_Boot->FirstClusterOfRootDirectory, file_count = 1, current_file = 0;
    exfile * files; 
    fatMain = getFATStart (inst);
    files = calloc (1, sizeof (exfile));
    decode_cluster ((void *)(inst->Data + FirstClusterOffset + Cluster * (CurrentCluster - 2)), Cluster, &files, &file_count, &current_file);
    while (fatMain[CurrentCluster] != (u_int32_t)(-1)) //check Fat table for next cluster 
    {
        CurrentCluster = fatMain[CurrentCluster]; //setup for scanning next cluster
        decode_cluster ((void *)(inst->Data + FirstClusterOffset + Cluster * (CurrentCluster - 2)), Cluster, &files, &file_count, &current_file);       
    }
   

    for (unsigned int i = 0; i < file_count - 1; i++)

    {
        
        if isZero(strcmp(inst->xvalue,files[i].filename))
        {
            FILE *fp;
            fp=fopen(inst->ovalue,"w+");
             if (fp == NULL){
              perror("fopen");
            }
        
            do{

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

              }
            }while(files[i].length>0);
            
            fclose(fp);
        }

    }


    free (files);

    return EXIT_SUCCESS;

}