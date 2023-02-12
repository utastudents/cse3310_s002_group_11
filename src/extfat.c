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

char * help[] = {
    "This is help line 1.",
    "This is help line 2.",
    "This is help line 3.",
    "This is help line 4.",
    "This is help line 5.",
    "This is help line 6.",
    NULL
};

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
    void * mem;
    char * function;
};

int main(int argc, char ** argv)
{
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
    inst.mem = NULL;
    inst.function = "main";
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