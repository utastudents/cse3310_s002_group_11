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
int mapFile (struct instance * inst)
{
    int val = 0;
    if (isNull(inst)) return EXIT_FAILURE;
    setFunction(inst);
    inst->fdInput = open(inst->ivalue, O_RDWR);
    if (isFault(inst->fdInput))
    {
        fprintf (stderr, "%s: Unable to open input file [%s] - %s\n", inst->function, inst->ivalue, strerror(errno));
        return EXIT_FAILURE;
    }
    fstat (inst->fdInput, &(inst->inFile));
    inst->memInput = mmap (NULL, inst->inFile.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, inst->fdInput, 0);
    if (inst->memInput == MAP_FAILED)
    {
        printf ("%s: Cannot map input file to memory - %s\n", inst->function, strerror(errno));
        return EXIT_FAILURE;
    }
    inst->bootSectorMain = inst->memInput;
    if (verifyExfat (inst) == EXIT_FAILURE)
    {
        fprintf (stderr, "%s: Unable to verify Exfat Image\n", inst->function);
        return EXIT_FAILURE;
    }
    setFunction (inst);
    val = (1 << inst->bootSectorMain->bytesPerSectorShift) * 12;
    inst->bootSectorBackup = ((void *)(inst->memInput) + val);
    if (compareBootSec (inst) == EXIT_FAILURE)
    {
        msync (inst->memInput, inst->inFile.st_size, MS_SYNC);
        munmap (inst->memInput, inst->inFile.st_size);
        return EXIT_FAILURE;
    }
    setFunction(inst);
    // Don't process output file unless we are copying
    if (inst->cflag != 1) return EXIT_SUCCESS;
    if (isNull(inst->ovalue)) // Merged from Rency
    {
        fprintf (stderr, "%s - Specified a copy without an output file\n", inst->function);
        return EXIT_FAILURE;
    }
    if (isZero (strcmp (inst->ivalue, inst->ovalue))) // Merged from Rency
    {
        inst->memOutput = inst->memInput;
        return EXIT_SUCCESS;
    }
    if (isZero(stat (inst->ovalue, &(inst->outFile)))) // Modified from Phu
    {
        fprintf (stderr, "%s: Overwrite not allowed, removing output file [%s] before copy\n", inst->function, inst->ovalue);
        remove (inst->ovalue);
    }
    inst->fdOutput = open(inst->ovalue, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH); // Merged from Phu
    if (isFault(inst->fdOutput))
    {
        fprintf (stderr, "%s: Unable to open output file [%s] - %s\n", inst->function, inst->ovalue, strerror(errno));
        return EXIT_FAILURE;
    }

    ftruncate64 (inst->fdOutput, inst->inFile.st_size); // Modified from Phu

    inst->memOutput = mmap (NULL, inst->inFile.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, inst->fdOutput, 0);  // Merged from Phu
    if (inst->memOutput == MAP_FAILED)
    {
        printf ("%s: Can not map output file to memory - %s\n", inst->function, strerror(errno));
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

// Unmap the files from the main memory structure
int unmapFile (struct instance * inst)
{
    setFunction (inst);
    msync (inst->memInput, inst->inFile.st_size, MS_SYNC); // Merged from Phu
    munmap (inst->memInput, inst->inFile.st_size); // Merged from Phu
    close (inst->fdInput); // Merged from Phu
    msync (inst->memOutput, inst->inFile.st_size, MS_SYNC); // Merged from Phu
    munmap (inst->memOutput, inst->inFile.st_size); // Merged from Phu
    close (inst->fdOutput); // Merged from Phu
    return EXIT_SUCCESS;
}

// Use mmap to copy data from input to output files
int mmapCopy (struct instance * inst)
{
    if (isNull(inst)) return EXIT_FAILURE;
    setFunction(inst);
    if (isZero(strcmp (inst->ivalue, inst->ovalue)))
    {
        fprintf (stderr, "%s: Can not copy input file [%s] to itself\n", inst->function, inst->ivalue);
        return EXIT_FAILURE;
    }
    // memcpy has no fault condition, but will throw a segfault if out of bounds
    // Trap SIGSEGV to more gracefully handle this
    memcpy (inst->memOutput, inst->memInput, inst->inFile.st_size); // Merged from Phu
    return EXIT_SUCCESS;
}
