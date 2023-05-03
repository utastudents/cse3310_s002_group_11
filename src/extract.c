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
    unsigned int CurrentCluster = inst->M_Boot->FirstClusterOfRootDirectory - 2, file_count = 1, current_file = 0;
    exfile * files; 
    fatMain = getFATStart (inst);
    files = calloc (1, sizeof (exfile));
    
    while (true)
    {
        decode_cluster ((void *)(inst->Data + FirstClusterOffset + Cluster * CurrentCluster), Cluster, &files, &file_count, &current_file, inst);           
        CurrentCluster = nextCluster (fatMain, CurrentCluster, false, false); // Root directory always uses FAT table
        if (CurrentCluster == (u_int32_t)(-1)) break;
        if (isZero(CurrentCluster)) break;
        CurrentCluster -= 2;
    }
   
   
    for (unsigned int i = 0; i < file_count - 1; i++)

    { 
        if isZero(strcmp(inst->xvalue,files[i].filename))
        {
            FILE *fp;
            fp=fopen(inst->ovalue,"w+");
            if (fp == NULL) perror("fopen");
            CurrentCluster = files[i].cluster - 2;
            do
            {
                fwrite((void *)(inst->Data + FirstClusterOffset + Cluster * CurrentCluster),(files[i].length>Cluster)?Cluster:files[i].length,1,fp);
                CurrentCluster = nextCluster (fatMain, CurrentCluster, files[i].allocationPossible, files[i].noFatChain);
                if (CurrentCluster == (u_int32_t)(-1)) break;
                if (isZero(CurrentCluster)) break;
                if (isFalse(files[i].noFatChain)) CurrentCluster -= 2;
                if (files[i].length <= Cluster) break;
                if(files[i].length>Cluster) files[i].length -= Cluster;
            } while(files[i].length>0);
            fclose(fp);
        }
    }
    free (files);
    return EXIT_SUCCESS;

}

