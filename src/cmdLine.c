#define CMDLINE_C

#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include <extfat.h>

int initInstance (struct instance * inst) // Added from Phu
{
    setFunction(inst);
    inst->iflag = false;
    inst->oflag = false;
    inst->cflag = false;
    inst->vflag = false;
    inst->fflag = false;
    inst->mflag = false;
    inst->fdInput = -1;
    inst->fdOutput = -1;
    inst->opt = -1;
    inst->ivalue = NULL;
    inst->ovalue = NULL;
    bzero (&(inst->inFile), sizeof (struct stat));
    bzero (&(inst->outFile), sizeof (struct stat));
    inst->memInput = NULL;
    inst->memOutput = NULL;
    return EXIT_SUCCESS;
}

int fillInstance (struct instance * inst, int argc, char ** argv) // Added from Phu
{
    char * help[] = {
        "extfat utility",
        "    Usage: extfat -i inputFile -o outputFile -c -h ",
        "",
        "           -i xxx    where xxx is the input file name [This is optional, but -i test.image is implied if not specified]",
        "           -o xxx    where xxx is the output file number [This is optional, inputFile will be used if not specified]",
        "           -c        triggers the copying of input to output (This is optional)",
        "           -d        directory listing",
        "           -v        verify exfat image", // Added from Rency
        "           -h        is this help message",
        NULL
    };    
    int i = 0;
    while ((inst->opt = getopt (argc, argv, "i:co:hdfmv")) != -1)
    {
        switch (inst->opt)
        {
            case 'c':
                inst->cflag = true;
                break;
            case 'i':
                inst->iflag = true;
                inst->ivalue = optarg;
                break;
            case 'o':
                inst->oflag = true;
                inst->ovalue = optarg;
                break;
            case ':':
                if (optopt == 'i' || optopt == 'o')
                fprintf (stderr, "Option requires an argument.\n");
                return EXIT_FAILURE;
            case 'd':
                inst->dflag = true;
                break;
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
    if (isFalse(inst->iflag)) inst->ivalue = "test.image";
    if (isFalse(inst->oflag)) inst->ovalue = inst->ivalue;
    if (isFalse(inst->fflag) && isFalse(inst->mflag)) inst->mflag = true;
    if (isTrue(inst->fflag) && isTrue(inst->mflag)) // Added from Rency
    {
        fprintf (stderr, "-f and -m options are mutually exclusive\n");
        exit (EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}
