#define MMAP_C

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/mman.h>

#include <extfat.h>


// Map the files from the main memory structure
int mapFile (fileInfo * inst)
{
    int val = 0;
    if (isNull(inst)) return EXIT_FAILURE;
    setFunction(inst);
    inst->fd = open(inst->filename, O_RDWR);
    if (isFault(inst->fd))
    {
        fprintf (stderr, "%s: Unable to open input file [%s] - %s\n", inst->function, inst->filename, strerror(errno));
        return EXIT_FAILURE;
    }
    fstat (inst->fd, &(inst->inFile));
    inst->Data = mmap (NULL, inst->inFile.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, inst->fd, 0);
    if (inst->Data == MAP_FAILED)
    {
        printf ("%s: Cannot map input file to memory - %s\n", inst->function, strerror(errno));
        return EXIT_FAILURE;
    }
    inst->M_Boot = inst->Data;
    if (verifyExfat (inst) == EXIT_FAILURE)
    {
        fprintf (stderr, "%s: Unable to verify Exfat Image\n", inst->function);
        return EXIT_FAILURE;
    }
    setFunction (inst);
    val = (inst->SectorSize=1 << inst->M_Boot->BytesPerSectorShift) * 12;
    inst->B_Boot = ((void *)(inst->Data) + val);
    if (inst->vflag && (compareBootSec (inst) == EXIT_FAILURE))
    {
        msync (inst->Data, inst->inFile.st_size, MS_SYNC);
        munmap (inst->Data, inst->inFile.st_size);
        fprintf (stderr, "%s - Verification error, checksums of Main Boot and Back Boot do not match.\n", inst->function);
        return EXIT_FAILURE;
    }
    setFunction(inst);
    // Don't process output file unless we are copying
    if (inst->cflag != 1) return EXIT_SUCCESS;
    if (isNull(inst->ovalue)) 
    {
        fprintf (stderr, "%s - Specified a copy without an output file\n", inst->function);
        return EXIT_FAILURE;
    }
    if (isZero (strcmp (inst->filename, inst->ovalue))) 
    {
        inst->memOutput = inst->Data;
        return EXIT_SUCCESS;
    }
    if (isZero(stat (inst->ovalue, &(inst->outFile)))) 
    {
        fprintf (stderr, "%s: Overwrite not allowed, removing output file [%s] before copy\n", inst->function, inst->ovalue);
        remove (inst->ovalue);
    }
    inst->fdOutput = open(inst->ovalue, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH); 
    if (isFault(inst->fdOutput))
    {
        fprintf (stderr, "%s: Unable to open output file [%s] - %s\n", inst->function, inst->ovalue, strerror(errno));
        return EXIT_FAILURE;
    }

    ftruncate64 (inst->fdOutput, inst->inFile.st_size); 

    inst->memOutput = mmap (NULL, inst->inFile.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, inst->fdOutput, 0);  
    if (inst->memOutput == MAP_FAILED)
    {
        printf ("%s: Can not map output file to memory - %s\n", inst->function, strerror(errno));
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

// Unmap the files from the main memory structure
int unmapFile (fileInfo * inst)
{
    setFunction (inst);
    msync (inst->Data, inst->inFile.st_size, MS_SYNC); 
    munmap (inst->Data, inst->inFile.st_size); 
    close (inst->fd);
    if (inst->cflag != 1) return EXIT_SUCCESS;
    msync (inst->memOutput, inst->inFile.st_size, MS_SYNC); 
    munmap (inst->memOutput, inst->inFile.st_size); 
    close (inst->fdOutput); 
    return EXIT_SUCCESS;
}

// Use mmap to copy data from input to output files
int mmapCopy (fileInfo * inst)
{
    if (isNull(inst)) return EXIT_FAILURE;
    setFunction(inst);
    if (isZero(strcmp (inst->filename, inst->ovalue)))
    {
        fprintf (stderr, "%s: Can not copy input file [%s] to itself\n", inst->function, inst->filename);
        return EXIT_FAILURE;
    }
    // memcpy has no fault condition, but will throw a segfault if out of bounds
    // Trap SIGSEGV to more gracefully handle this
    memcpy (inst->memOutput, inst->Data, inst->inFile.st_size); 
    return EXIT_SUCCESS;
}
