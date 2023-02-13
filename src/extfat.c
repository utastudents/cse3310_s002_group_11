#include <ctype.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

// Insert constants here

// Insert more structures here

// Main memory structure
struct instance
{
    bool iflag;
    bool oflag;
    bool cflag;
    int fd;
    int opt;
    char * ivalue;
    char * ovalue;
    struct stat infile;
    struct stat outfile;
    void * memInput;
    void * memOutput;
    char * function;
    void * bootSectorMain;
    void * bootSectorBackup;
};

// Function Declarations
int mapFile (struct instance *);
int readFile (struct instance *);
int writeFile (struct instance *);
int compareBootSec (struct instance *);

// Map the files from the main memory structure
int mapFile (struct instance * inst)
{
    return EXIT_SUCCESS;
}

// Read the file from the main memory structure, input only
int readFile (struct instance * inst)
{
    return EXIT_SUCCESS;
}

// Write the file from the main memory structure, output only
int writeFile (struct instance * inst)
{
    return EXIT_SUCCESS;
}

// Compare the boot sectors from the main memory structure
int compareBootSec (struct instance * inst)
{
    return EXIT_SUCCESS;
}

int main(int argc, char ** argv)
{
char * help[] = {
    "This is help line 1.",
    "This is help line 2.",
    "This is help line 3.",
    "This is help line 4.",
    "This is help line 5.",
    "This is help line 6.",
    NULL
};    
    struct instance inst;
    inst.iflag = false;
    inst.oflag = false;
    inst.cflag = false;
    inst.fd = -1;
    inst.opt = -1;
    inst.ivalue = NULL;
    inst.ovalue = NULL;
    bzero (&inst.infile, sizeof (struct stat));
    bzero (&inst.outfile, sizeof (struct stat));
    inst.memInput = NULL;
    inst.function = __func__;
    while ((inst.opt = getopt (argc, argv, "i:co:h")) != -1)
    {
        switch (inst.opt)
        {
            case 'c': inst.cflag = true; break;
            case 'i': inst.iflag = true; inst.ivalue = optarg; break;
            case 'o': inst.oflag = true; inst.ovalue = optarg; break;
            case ':':
                if (optopt == 'i' || optopt == 'o')
                fprintf (stderr, "Option requires an argument.\n");
                return EXIT_FAILURE;
            case 'h': while (help[0] != NULL) { fprintf (stderr, "%s\n", help[0]); help[0] = help[1]; } break;
            case '?': fprintf (stderr, "Option not recognized\n"); break;
            default:;
        }
    }
    return EXIT_SUCCESS;
}
