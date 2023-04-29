#include <ctype.h>
#include <errno.h>
#include <extfat.h>
#include <fcntl.h>
#include <getopt.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>



int main(int argc, char ** argv)
{
    fileInfo exfat; 
    setFunction ((&exfat));
    initInstance (&exfat); 
    setFunction ((&exfat));
    fillInstance (&exfat, argc, argv); 
    if (isTrue(exfat.vflag && isFalse(exfat.iflag)))
    {
        fprintf (stderr, "%s: Verification requires an input file be specified\n", exfat.function);
        return EXIT_FAILURE;
    }
    if (isTrue(exfat.cflag && (isFalse(exfat.iflag) || isFalse(exfat.oflag))))
    {
        fprintf (stderr, "%s: Copy requires the input and output file be specified\n", exfat.function);
        return EXIT_FAILURE;
    }
    if (mapFile (&exfat) == EXIT_FAILURE)
    {
        unmapFile (&exfat);
        return EXIT_FAILURE;
    }
    if (isTrue(exfat.cflag)) mmapCopy (&exfat); 
    if(isTrue(exfat.xflag))extractfile(&exfat);
    if (isTrue(exfat.dflag)) directoryPrint(&exfat); // Added from Chris
    if (isTrue(exfat.Dflag)) deleteFile(&exfat);
    unmapFile (&exfat);
    return EXIT_SUCCESS;
}
