#define CMDLINE_C

#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include <extfat.h>

int initInstance (fileInfo * inst) // Added from Phu
{
    setFunction(inst);
    inst->iflag = false;
    inst->oflag = false;
    inst->cflag = false;
    inst->vflag = false;
    inst->fflag = false;
    inst->mflag = false;
    inst->fd = -1;
    inst->fdOutput = -1;
    inst->opt = -1;
    inst->filename = NULL;
    inst->ovalue = NULL;
    int SectorSize=0;
    int FileLength=0;
    bzero (&(inst->inFile), sizeof (struct stat));
    bzero (&(inst->outFile), sizeof (struct stat));
    inst->Data = NULL;
    inst->memOutput = NULL;
    return EXIT_SUCCESS;
}

int fillInstance (fileInfo * inst, int argc, char ** argv) // Added from Phu
{
    char * help[] = {
        "extfat utility",
        "    Usage: extfat -i inputFile -o outputFile -c -h ",
        "",
        "           -i xxx    where xxx is the input file name [This is optional, but -i test.image is implied if not specified]",
        "           -o xxx    where xxx is the output file number [This is optional, inputFile will be used if not specified]",
        "           -c        triggers the copying of input to output (This is optional)",
        "           -m        use mmap for file access. [implied if -f and -m not specified]", // Added from Rency
        "           -f        use fread for file access", // Addded from Rency
        "           -v        verify exfat image", // Added from Rency
        "           -h        is this help message",
        NULL
    };    
    int i = 0;
    while ((inst->opt = getopt (argc, argv, "i:co:hfmv")) != -1)
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
            case ':':
                if (optopt == 'i' || optopt == 'o')
                fprintf (stderr, "Option requires an argument.\n");
                return EXIT_FAILURE;
            case 'm': // Added from Rency
                inst->mflag = true;
                break;
            case 'f': // Added from Rency
                inst->fflag = true;
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
    if (isFalse(inst->fflag) && isFalse(inst->mflag)) inst->mflag = true;
    if (isTrue(inst->fflag) && isTrue(inst->mflag)) // Added from Rency
    {
        fprintf (stderr, "-f and -m options are mutually exclusive\n");
        exit (EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}
