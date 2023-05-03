#define CMDLINE_C

#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include <extfat.h>

int initInstance (fileInfo * inst) 
{
    setFunction(inst);
    inst->iflag = false;
    inst->oflag = false;
    inst->cflag = false;
    inst->vflag = false;
    inst->dflag = false; // Added from Chris
    inst->xflag = false;
    inst->Dflag = false;
    inst->fd = -1;
    inst->fdOutput = -1;
    inst->opt = -1;
    inst->filename = NULL;
    inst->ovalue = NULL;
    inst->Dvalue = NULL;
    inst-> SectorSize = 0;
    inst-> FileLength = 0;
    bzero (&(inst->inFile), sizeof (struct stat));
    bzero (&(inst->outFile), sizeof (struct stat));
    inst->Data = NULL;
    inst->memOutput = NULL;
    inst->allocationBitmap = 0;
    return EXIT_SUCCESS;
}

int fillInstance (fileInfo * inst, int argc, char ** argv) 
{
    char * help[] = {
        "extfat utility",
        "    Usage: extfat -i inputFile -o outputFile -c -h ",
        "",
        "           -i xxx    where xxx is the input file name [This is optional, but -i test.image is implied if not specified]",
        "           -o xxx    where xxx is the output file number [This is optional, inputFile will be used if not specified]",
        "           -c        copy input ",
        "           -d        directory listing",// Added from Chris
        "           -D        delete file",// Added from Chris
        "           -x xxx    where xxx is the file to be extracted"
        "           -v        verify exfat image", // Added from Rency
        "           -h        is this help message",
        NULL
    };    
    int i = 0;
    while ((inst->opt = getopt (argc, argv, "i:co:x:hdD:v")) != -1)
    {
        switch (inst->opt)
        {
            case 'c':
                inst->cflag = true;
                break;
            case 'i':
                inst->iflag = true;
                inst->filename = optarg;
                break;
            case 'o':
                inst->oflag = true;
                inst->ovalue = optarg;
                break;
            case 'x':
                inst->xflag = true;
                inst->xvalue = optarg;
                break;
            case 'D':
                inst->Dflag = true;
                inst->Dvalue = optarg;
                break;
            case ':':
                if (optopt == 'i' || optopt == 'o'|| optopt == 'x' )
                fprintf (stderr, "Option requires an argument.\n");
                return EXIT_FAILURE;
            case 'd': // Added from Chris
                inst->dflag = true;
                break;
            case 'v': // Added from Rency
                inst->vflag = true;
                break;
            case 'h':
                while (help[i] != NULL) printf ("%s\n", help[i++]);
                exit (EXIT_SUCCESS);
            case '?':
                fprintf (stderr, "Option not recognized\n");
                return EXIT_FAILURE;
            default:;
        }
    }
    if (isFalse(inst->iflag)) inst->filename = "test.image";
    if (isFalse(inst->oflag)) inst->ovalue = inst->filename;
    return EXIT_SUCCESS;
}