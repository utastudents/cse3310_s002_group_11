/*

   This is an example of using mmap to read an extFAT image file.

   To make the sample file:

    % # create the file system image
    % dd if=/dev/zero of=test.image count=1 bs=1G
    % sudo losetup /dev/loop2 test.image
    % sudo /usr/sbin/mkexfatfs /dev/loop2

    % # put something in the file system image
    % mkdir /tmp/d
    % sudo mount /dev/loop2 /tmp/d
    % cp examples/mmap.c /tmp/d

    % # clean up
    % sudo umount /tmp/d
    % sudo losetup -d /dev/loop2
    % rm -rf /tmp/d
    % rm test.image



   Written by Bud Davis, jimmie.davis@uta.edu
   (c) 2023, All Rights Reserved
   Provided to students of CSE3310, UTA. Any use
   other than this course is prohibited.

*/
#include<ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stddef.h>
#include <getopt.h>

#include "extfat.h" //strcuture defined in the header file


int main(int argc, char **argv)
{
   struct test B1={0,0,NULL,NULL,0}; //members of struct are initialize(NOTE:'-w' option for future use needs to be initialize)

   opterr = 0;

   while ((B1.c = getopt (argc, argv, "hi:o:cv")) != -1) //'-w' option for future use
    switch (B1.c)
      {
      case 'h': //help text
         printf("Need Help?\nAllowed options:\n1. '-i' option for the input file\n2. '-o' option for the output file.\n");
         printf("3. '-c' option denotes copy, where the input file is copied to the output file without change\n4. '-w' option not avaiable right now,but for future use\n5. '-v' option means verify for input file\n");
         return EXIT_SUCCESS; //exit

        break;
      case 'i':
         B1.ivalue=optarg;

        break;
      case 'o':
         B1.ovalue=optarg;

        break;
      case 'c':
         B1.cflag = 1;

        break;
      //case 'w': -for future use
      case 'v':
         B1.vflag = 1;

        break;
      case '?':
        if (optopt == 'i' || optopt == 'o')
          fprintf (stderr, "Option -%c requires an argument.\n", optopt);
        else if (isprint (optopt))
          fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        else
          fprintf (stderr,
                   "Unknown option character `\\x%x'.\n",
                   optopt);
        return 1;
      default:
        abort ();
      }

   if(B1.ovalue==NULL) //optional if output file is not specfied
   {
      B1.ovalue=B1.ivalue; //out file is same as input file
   }
printf ("cflag = %d, vflag = %d, ivalue = %s, ovalue = %s\n",B1.cflag,B1.vflag,B1.ivalue,B1.ovalue);

   // open the file system image file
   int fd = open("test.image", O_RDWR);
   if (fd == -1)
   {
      perror("file open: ");
      exit(0);
   }

   // Take the pointer returned from mmap() and turn it into
   // a structure that understands the layout of the data
   Main_Boot *MB = (Main_Boot *)mmap(NULL,
                                     sizeof(Main_Boot),
                                     PROT_READ,
                                     MAP_PRIVATE,
                                     fd,
                                     0); // note the offset

   if (MB == (Main_Boot *)-1)
   {
      perror("error from mmap:");
      exit(0);
   }

   // print out some things we care about

   printf("the pointer to MB is %p  \n", MB);

   printf("JumpBoot  %d %d %d \n", MB->JumpBoot[0], MB->JumpBoot[1], MB->JumpBoot[2]);
   printf("FileSystemName %s\n", MB->FileSystemName); // warning, not required to be terminated
   printf("PartitionOffset %ld\n", MB->PartitionOffset);
   printf("VolumeLength %ld\n", MB->VolumeLength);
   printf("FatOffset %d\n", MB->FatOffset);
   printf("FatLength %d\n", MB->FatLength);
   printf("ClusterHeapOffset %d\n", MB->ClusterHeapOffset);
   printf("ClusterCount %d\n", MB->ClusterCount);
   printf("FirstClusterofRootDirectory %d\n", MB->FirstClusterOfRootDirectory);
   printf("VolumeSerialNumber %x\n", MB->VolumeSerialNumber);
   printf("PercentInUse %d\n", MB->PercentInUse);
   

   // check that the offsets match the documentation
   assert(offsetof(Main_Boot, PercentInUse) == 112);
   //printf("the offset of PerCentInUse %ld\n", offsetof(Main_Boot, PercentInUse));

   printf("BytesPerSectorShift %d\n",MB->BytesPerSectorShift);
   printf("SectorsPerClusterShift %d\n",MB->SectorsPerClusterShift);
   printf("NumberOfFats %d\n",MB->NumberOfFats);

   int bytesPerSector = 2  << (MB->BytesPerSectorShift-1);
   int sectorsPerCluster = 2 << (MB->SectorsPerClusterShift-1);

   printf("-----> the bytesPerSector are %d\n",bytesPerSector);
   printf("-----> the sectorsPerCluster are %d\n",sectorsPerCluster);

   
   // unmap the file
   if (munmap(MB, sizeof(Main_Boot)) == -1)
   {
      perror("error from unmap:");
      exit(0);
   }

   // close the file
   if (close(fd))
   {
      perror("closeStat:");
   }
   fd = 0;

   return 0;
}