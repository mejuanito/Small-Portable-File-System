#include <stdio.h>
#include <stdlib.h>
//including the fs.h file for this file
#include "fs.h"

fileSystem *createfileSystem(int numOfBlocks) 
{
    fileSystem *fs = malloc(sizeof(fileSystem));

    //If unable to give memory for fileSystem
    //then return null to be safe
    if(fs == NULL)
    {
        return NULL;
    }

    fs->numOfBlocks = numOfBlocks;
    fs->formatted = 0;
    fs->fnt = NULL;
    fs->dabpt = NULL;
    fs->bpt = NULL;
    fs->disk = malloc(numOfBlocks * BLOCK_SIZE);

    //check if disk null, and free the memory if it is
    if (fs->disk == NULL)
    {
        free(fs);
        return NULL;
    }

    fs->blockUsed = malloc(numOfBlocks * sizeof(int));

    if (fs->blockUsed == NULL)
    {
        free(fs->disk);
        free(fs);
        return NULL;
    }

    for (int i = 0; i < numOfBlocks; i++)
    {
        fs->blockUsed[i] = 0;
    }

    return fs;
}

int freeBlock(fileSystem *fs)
{
    for (int i = 0; i < fs->numOfBlocks; i++)
    {
        if (fs->blockUsed[i] == 0)
        {
            return i;
        }
    }
    return -1;
}
void destroyfileSystem(fileSystem *fs)
{
    free(fs->disk);
    free(fs->blockUsed);
    free(fs);
}
