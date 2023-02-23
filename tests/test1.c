//Test code for Iteration 1 group
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include<stdbool.h>

#define isTrue(x) (x == true) // Boolean test for being true
#define isFalse(x) (x == false) // Boolean test for being false

struct instance
{
    bool iflag;
    bool oflag;
    bool cflag;
    bool vflag;
    bool fflag;
    bool mflag;
    int opt;
    char * ivalue;
    char * ovalue;

};

int main (int argc, char **argv)
{
  char * help[] = {
        "extfat utility",
        "    Usage: extfat -i inputFile -o outputFile -c -h ",
        "",
        "           -i xxx    where xxx is the input file name [This is optional, but -i test.image is implied if not specified]",
        "           -o xxx    where xxx is the output file number [This is optional, inputFile will be used if not specified]",
        "           -c        triggers the copying of input to output (This is optional)",
        "           -m        use mmap for file access. [implied if -f and -m not specified]",
        "           -f        use fread for file access", 
        "           -v        verify exfat image", 
        "           -h        is this help message",
        NULL
    }; 
   struct instance B1={false,false,false,false,false,false,-1,NULL,NULL};
    
    int i;
  
   while ((B1.opt = getopt (argc, argv, "i:co:hfmv")) != -1)
    {
        switch (B1.opt)
        {
            case 'c':
                B1.cflag = true;
                break;
            case 'i':
                B1.iflag = true;
                B1.ivalue = optarg;
                break;
            case 'o':
                B1.oflag = true;
                B1.ovalue = optarg;
                break;
            case ':':
                if (optopt == 'i' || optopt == 'o')
                fprintf (stderr, "Option requires an argument.\n");
                return EXIT_FAILURE;
            case 'm':
                B1.mflag = true;
                break;
            case 'f':
                B1.fflag = true;
                break;
            case 'v': 
                B1.vflag = true;
                break;
            case 'h':
                while (help[i] != NULL)
                {
                    fprintf (stderr, "%s\n", help[i]);
                    i++;
                }
                return EXIT_SUCCESS;
            case '?':
                fprintf (stderr, "Option not recognized\n");
                return EXIT_FAILURE;
            default:;
        }
    }
    if (isFalse(B1.iflag)) B1.ivalue = "test.image";
    if (isFalse(B1.oflag)) B1.ovalue = B1.ivalue;
    if (isFalse(B1.fflag) && isFalse(B1.mflag)) B1.mflag = true;
    if (isTrue(B1.fflag) && isTrue(B1.mflag))
    {
        fprintf (stderr, "-f and -m options are mutually exclusive");
        return EXIT_FAILURE;
    }
    printf ("cflag = %d, vflag = %d,fflag= %d,mflag=%d,ivalue = %s, ovalue = %s\n",B1.cflag,B1.vflag,B1.fflag,B1.fflag,B1.ivalue,B1.ovalue);
  return 0;
}


















