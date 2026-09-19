#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include "fs.h"
//To add some color to the console :)
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_RESET "\x1b[0m"
int main()
{
    char cmd[20];
    int numOfBlocks;
    fileSystem *fs = NULL;
    char choice[100];
    int numFN;
    int numDABPT;
    char filename[57];

    
    while (true)
    {
        //list all commands
        printf(ANSI_COLOR_GREEN"\n--Commands--\n");
        printf("Createfs\n");
        printf("Formatfs\n");
        printf("Savefs\n");
        printf("Openfs\n");
        printf("List\n");
        printf("Put\n");
        printf("Get\n");
        printf("Remove\n");
        printf("Rename\n");
        printf("User\n");
        printf("Link\n");
        printf("Unlink\n");
        printf("Exit\n"ANSI_COLOR_RESET);
        printf(ANSI_COLOR_BLUE"\nFS> "ANSI_COLOR_RESET);

        printf(ANSI_COLOR_BLUE);
        //So we can input the command, and then the info on the same line
        fgets(choice, sizeof(choice), stdin);
        choice[strcspn(choice, "\n")] = '\0';

        char *command = strtok(choice, " ");
        char *arg1 = strtok(NULL, " ");
        char *arg2 = strtok(NULL, " ");

        if (strcmp(command, "Exit") == 0)
        {
            break;
        }

        else if (strcmp(command, "Createfs") == 0) 
        {
            if (arg1 == NULL)
            {
                printf("\nIncorrect Usage: Createfs '#blocks' \n");
            }
            else 
            {
                numOfBlocks = atoi(arg1);
                if(numOfBlocks <= 0)
                {
                    printf("\nInvalid number of blocks entered.\n");
                }
                else
                {   
                    fs = createfileSystem(numOfBlocks);
                    if (fs == NULL)
                    {
                        printf("\nFailed to create the filesystem.\n");
                        //used for error in creation of files
                        return 1;
                    }
                    else 
                    {
                        printf("\nFilesystem created\n");
                    }   
                }    
            }   
        } 
        
        else if (strcmp(command, "Formatfs") == 0)
        {
            if (arg1 == NULL)
            {
                printf("\nIncorrect Usage: Formatfs '#filenames' '#DABPTEntries' \n");
            }
            else
            {

                if (fs == NULL)
                {
                    printf("\nFilesystem does not exist. Use Createfs\n");
                }
                else 
                {
                    //parse both arguments into the variables
                    numFN = atoi(arg1);
                    numDABPT = atoi(arg2);

                    //equation plugged in
                    int FNTBlocks = (numFN + 3) / 4;
                    int DABPTBlocks = (numDABPT +3) / 4;

                    if (1 + FNTBlocks + DABPTBlocks > fs->numOfBlocks)
                    {
                        printf("\nNot enough blocks for DABPT and FNT\n");
                        return 1;
                    }
                    
                    //set values
                    fs->numFN = numFN;
                    fs->numDABPT = numDABPT;
                    fs->FNTStart = 1;
                    fs->DABPTStart = fs->FNTStart + FNTBlocks;

                    //Makes all blocks free
                    for (int i = 0; i < fs->numOfBlocks; i++)
                    {
                        fs->blockUsed[i] = 0;
                    }
                    //Reserves
                    fs->blockUsed[0] = 1;

                    //Makes FNT blocks used
                    for (int i = fs->FNTStart; i < fs->FNTStart + FNTBlocks; i++)
                    {
                        fs->blockUsed[i] = 1;
                    }

                    //Makes DABPT blocks used
                    for (int i = fs->DABPTStart; i < fs->DABPTStart + DABPTBlocks; i++)
                    {
                        fs->blockUsed[i] = 1;
                    }
                    
                    fs->fnt = malloc(numFN * sizeof(FNTEntry));

                    //check if allocation of memory failed
                    if (fs->fnt == NULL)
                    {
                        printf("\nFailed to allocate memory for FNT\n");
                        return 1;
                    }
                
                    for (int i = 0; i < numFN; i++)
                    {
                        fs->fnt[i].inode = -1;
                        fs->fnt[i].filename[0] ='\0';
                    }
                 
                    memcpy(fs->disk + (fs->FNTStart * BLOCK_SIZE), fs->fnt, numFN * sizeof(FNTEntry));

                    fs->dabpt = malloc(numDABPT * sizeof(DABPTEntry));
                
                    if(fs->dabpt == NULL)
                    {
                        printf("\nFailed to allocate memory for DABPT\n");
                        return 1;
                    }

                    for(int i = 0; i < numDABPT; i++)
                    {
                        fs->dabpt[i].bptPointer = -1;
                        fs->dabpt[i].fileSize = 0;
                        fs->dabpt[i].lastTime = 0;
                        fs->dabpt[i].usrname[0] = '\0';
                    }
                  
                    memcpy(fs->disk + (fs->DABPTStart * BLOCK_SIZE), fs->dabpt, numDABPT * sizeof(DABPTEntry));
                    
                    //metadata
                    FSHeader header;
                    header.numOfBlocks = fs->numOfBlocks;
                    header.numFN = fs->numFN;
                    header.numDABPT = fs->numDABPT;
                    header.FNTStart = fs->FNTStart;
                    header.DABPTStart = fs->DABPTStart;
                    header.formatted = 1;
                    memcpy(fs->disk, &header, sizeof(FSHeader));

                    fs->formatted = 1;
                    printf("\nFilesystem was succesfully formatted\n");
                }   
            }

        } else if(strcmp(command, "Savefs") == 0)
        {
            if (arg1 == NULL)
            {
                printf("\nIncorrect Usage: Savefs 'filename' \n");
            }
            else
            {
                if (fs == NULL)
                {
                    printf("\nFilesystem does not exist. Use Createfs\n");
                }
                else if (fs->formatted == 0)
                {
                    printf("\nFilesystem has not been formatted\n");
                }
                else 
                {
                    strcpy(filename, arg1);

                    //write in binary mode so it contains raw bytes
                    FILE *file = fopen(filename, "wb");
                    if (file == NULL)
                    {
                        printf("\nFilesystem not opened\n");
                    }
                    else 
                    {
                        fwrite(fs->disk, BLOCK_SIZE, fs->numOfBlocks, file);
                        fclose(file);
                        printf("\nSaved filesystem\n");
                    }
                }
            }
            
        } 
        else if (strcmp(command, "Openfs") == 0)
        {            
            if (arg1 == NULL)
            {
                printf("\nIncorrect Usage: Openfs 'filename' \n");
            }
            else
            {
                strcpy(filename, arg1);

                //read binary
                FILE *file = fopen(filename, "rb");
                if (file == NULL)
                {
                    printf("\nFilesystem not opened\n");
                }
                else
                {
                    //moves to end of file
                    fseek(file, 0, SEEK_END);
                    //tells us how many bytes is file we are
                    long filesize = ftell(file);
                    //moves back to beginning
                    rewind(file);
                    if (filesize % BLOCK_SIZE != 0)
                    {
                        printf("\nInvalid file\n");
                        fclose(file);
                    }
                    else
                    {
                        //if file exists
                        numOfBlocks = filesize / BLOCK_SIZE;
                        printf("\nFilesystem has %d blocks.\n", numOfBlocks);
                        
                        fs = createfileSystem(numOfBlocks);
                        if (fs == NULL)
                        {
                            printf("\nFailed to create filesystem\n");
                            fclose(file);
                        }
                        else 
                        {
                            //reads file
                            fread(fs->disk, BLOCK_SIZE, fs->numOfBlocks, file);
                            fclose(file);

                            FSHeader header;
                            memcpy(&header, fs->disk, sizeof(FSHeader));
                            
                            fs->numFN = header.numFN;
                            fs->numDABPT = header.numDABPT;
                            fs->FNTStart = header.FNTStart;
                            fs->DABPTStart = header.DABPTStart;
                            fs->formatted = header.formatted;

                            fs->blockUsed[0] = 1;

                            int FNTBlocks = (fs->numFN + 3) / 4;
                            for (int i = fs->FNTStart; i < fs->FNTStart + FNTBlocks; i++)
                            {
                                fs->blockUsed[i] = 1;
                            }

                            int DABPTBlocks = (fs->numDABPT + 3) / 4;
                            for (int i = fs->DABPTStart; i < fs->DABPTStart + DABPTBlocks; i++)
                            {
                                fs->blockUsed[i] = 1;
                            }

                            }
                            fs->fnt = malloc(fs->numFN * sizeof(FNTEntry));
                            fs->dabpt = malloc(fs->numDABPT * sizeof(DABPTEntry));
                            
                            if (fs->fnt == NULL || fs->dabpt == NULL)
                            {
                                printf("\nFailed to allocate memory for filesystem\n");
                            }
                            else
                            {
                                memcpy(fs->fnt, fs->disk + (fs->FNTStart * BLOCK_SIZE), fs->numFN * sizeof(FNTEntry));
                                memcpy(fs->dabpt, fs->disk + (fs->DABPTStart * BLOCK_SIZE), fs->numDABPT * sizeof(DABPTEntry));

                                for (int i = 0; i < fs->numDABPT; i++)
                                {
                                    if (fs->dabpt[i].bptPointer != -1)
                                    {
                                        int currentBPTBlock = fs->dabpt[i].bptPointer;

                                        while (currentBPTBlock != -1)
                                        {
                                            BPTEntry bpt;

                                            memcpy(&bpt, fs->disk + (currentBPTBlock * BLOCK_SIZE), sizeof(BPTEntry));

                                            fs->blockUsed[currentBPTBlock] = 1;

                                            for(int j = 0; j < 7; j++)
                                            {
                                                if (bpt.pointers[j] != -1)
                                                {
                                                    fs->blockUsed[bpt.pointers[j]] = 1;
                                                }
                                            }
                                            currentBPTBlock = bpt.pointers[7];
                                        }
                                    }
                                }
                            printf("\nFilesystem opened\n");
                        }
                    }
                }
            }
        } 
        else if (strcmp(command, "List") == 0)
        {
            if (fs == NULL || fs->formatted == 0)
            {
                printf("\nFilesystem does not exist or has not been formatted\n");
            }
            else 
            {
                for (int i = 0; i < fs->numFN; i++)
                {
                    if (fs->fnt[i].inode != -1)
                    {
                        printf("%s\n", fs->fnt[i].filename);
                    }
                }
            }
        }
        else if (strcmp(command, "Put") == 0)
        {
            if (arg1 == NULL)
            {
                printf("\nIncorrect Usage: Put 'filename'\n");
            }
            else if (fs == NULL || fs->formatted == 0)
            {
                printf("\nFilesystem does not exist or has not been formatted\n");
            }
            else if (strlen(arg1) > 55)
            {
                printf("\nFilename too long\n");
            }
            else
            {
                //prevents same file name
                int duplicate = 0;

                for (int i = 0; i < fs->numFN; i++)
                {
                    if (fs->fnt[i].inode != -1 &&
                        strcmp(fs->fnt[i].filename, arg1) == 0)
                    {
                        duplicate = 1;
                        break;
                    }
                }

                if (duplicate)
                {
                    printf("\nFile already exists in the filesystem\n");
                }
                else
                {
                    strcpy(filename, arg1);
                    FILE *file = fopen(filename, "rb");

                    if (file == NULL)
                    {
                        printf("\nFile could not open\n");
                    }
                    else
                    {
                        if (fseek(file, 0, SEEK_END) != 0)
                        {
                            printf("\nCould not determine file size\n");
                            fclose(file);
                        }
                        else
                        {
                            //almost everything below is checks incase of failure
                            long filesize = ftell(file);

                            if (filesize < 0)
                            {
                                printf("\nCould not determine file size\n");
                                fclose(file);
                            }
                            else if (filesize == 0)
                            {
                                printf("\nCannot Put an empty file\n");
                                fclose(file);
                            }
                            else
                            {
                                rewind(file);
                                int dataBlcks = (filesize + BLOCK_SIZE - 1) / BLOCK_SIZE;
                                int fntI = -1;

                                for (int i = 0; i < fs->numFN; i++)
                                {
                                    if (fs->fnt[i].inode == -1)
                                    {
                                        fntI = i;
                                        break;
                                    }
                                }

                                if (fntI == -1)
                                {
                                    printf("\nNo free filename entries\n");
                                    fclose(file);
                                }
                                else
                                {
                                    int dabptI = -1;

                                    for (int i = 0; i < fs->numDABPT; i++)
                                    {
                                        if (fs->dabpt[i].bptPointer == -1)
                                        {
                                            dabptI = i;
                                            break;
                                        }
                                    }

                                    if (dabptI == -1)
                                    {
                                        printf("\nNo free DABPT entries\n");
                                        fclose(file);
                                    }
                                    else
                                    {
                                        int bptEntriesNeeded = (dataBlcks + 6) / 7;
                                        BPTEntry *bptList = malloc(bptEntriesNeeded * sizeof(BPTEntry));
                                        int *allocBlocks = malloc((bptEntriesNeeded * 8) * sizeof(int));
                                        int allocCount = 0;

                                        if (bptList == NULL || allocBlocks == NULL)
                                        {
                                            printf("\nFailed to allocate memory to BPT\n");
                                            free(bptList);
                                            free(allocBlocks);
                                            fclose(file);
                                        }
                                        else
                                        {
                                            for (int i = 0; i < bptEntriesNeeded; i++)    
                                            {
                                                for (int j = 0; j < 8; j++)
                                                {
                                                    bptList[i].pointers[j] = -1;
                                                }
                                            }

                                            int allocationFailed = 0;

                                            for (int i = 0; i < bptEntriesNeeded; i++)
                                            {
                                                int bptBlock = freeBlock(fs);

                                                if (bptBlock == -1)
                                                {
                                                    allocationFailed = 1;
                                                    break;
                                                }

                                                fs->blockUsed[bptBlock] = 1;
                                                allocBlocks[allocCount] = bptBlock;
                                                allocCount++;

                                                if (i == 0)
                                                {
                                                    fs->dabpt[dabptI].bptPointer = bptBlock;
                                                }
                                                else
                                                {
                                                    bptList[i - 1].pointers[7] =  bptBlock;
                                                }

                                                for (int j = 0; j < 7 && (i * 7 + j) < dataBlcks; j++)
                                                {
                                                    int dataBlock = freeBlock(fs);

                                                    if (dataBlock == -1)
                                                    {
                                                        allocationFailed = 1;
                                                        break;
                                                    }
                                                    fs->blockUsed[dataBlock] = 1;
                                                    allocBlocks[allocCount] = dataBlock;
                                                    allocCount++;
                                                    bptList[i].pointers[j] = dataBlock;   
                                                }

                                                if (allocationFailed)
                                                {
                                                    break;
                                                }
                                            }

                                            if (allocationFailed)
                                            {
                                                printf("\nNot enough space for the file\n");

                                                for (int i = 0; i < allocCount; i++)   
                                                {
                                                    fs->blockUsed[allocBlocks[i]] = 0;
                                                }

                                                fs->dabpt[dabptI].bptPointer = -1;
                                                fs->dabpt[dabptI].fileSize = 0;
                                                fs->dabpt[dabptI].lastTime = 0;
                                                fs->dabpt[dabptI].usrname[0] = '\0';

                                                free(allocBlocks);
                                                free(bptList);
                                                fclose(file);
                                            }
                                            else
                                            {
                                                for (int i = 0; i < bptEntriesNeeded;i++)
                                                {
                                                    int bptBlock;

                                                    if (i == 0)
                                                    {
                                                        bptBlock = fs->dabpt[dabptI].bptPointer; 
                                                    }
                                                    else
                                                    {
                                                        bptBlock = bptList[i - 1].pointers[7];
                                                    }

                                                    memcpy(fs->disk + (bptBlock * BLOCK_SIZE), &bptList[i], sizeof(BPTEntry));  
                                                }

                                                int bytesRemaining = filesize;

                                                for (int i = 0; i < bptEntriesNeeded; i++)
                                                {
                                                    for (int j = 0;j < 7 && bytesRemaining > 0; j++)  
                                                    {
                                                        int dataBlock = bptList[i].pointers[j];
                                                        int bytesToWrite;

                                                        if (bytesRemaining > BLOCK_SIZE)
                                                        {
                                                            bytesToWrite = BLOCK_SIZE;
                                                        }
                                                        else
                                                        {
                                                            bytesToWrite = bytesRemaining;   
                                                        }

                                                        size_t bytesRead = fread(fs->disk + (dataBlock * BLOCK_SIZE), 1, bytesToWrite, file);
                                       
                                                        if (bytesRead != (size_t)bytesToWrite)
                                                        {
                                                            printf("\nError reading file\n");

                                                            for (int k = 0;k < allocCount;k++)
                                                            {
                                                                fs->blockUsed[allocBlocks[k]] = 0;
                                                            }

                                                            fs->dabpt[dabptI].bptPointer = -1;
                                                            fs->dabpt[dabptI].fileSize = 0;
                                                            fs->dabpt[dabptI].lastTime = 0;
                                                            fs->dabpt[dabptI].usrname[0] = '\0';
                                                                

                                                            free(allocBlocks);
                                                            free(bptList);
                                                            fclose(file);

                                                            allocationFailed = 1;
                                                            break;
                                                        }

                                                        bytesRemaining -= bytesToWrite;
                                                    }

                                                    if (allocationFailed)
                                                    {
                                                        break;
                                                    }
                                                }

                                                if (!allocationFailed)
                                                {
                                                    fclose(file);

                                                    fs->fnt[fntI].inode = dabptI;

                                                    strcpy(fs->fnt[fntI].filename, filename);

                                                    fs->dabpt[dabptI].fileSize = filesize;
                                                    fs->dabpt[dabptI].lastTime = time(NULL);
                                                        
                                                    memcpy(fs->disk + (fs->FNTStart *BLOCK_SIZE), fs->fnt, fs->numFN * sizeof(FNTEntry));
                                        
                                                    memcpy(fs->disk + (fs->DABPTStart * BLOCK_SIZE), fs->dabpt, fs->numDABPT *  sizeof(DABPTEntry));
                              
                                                    printf("\nFile successfully added\n");    
                                                    printf("Data blocks needed: %d\n", dataBlcks);
                                                    printf("BPT entries needed: %d\n", bptEntriesNeeded);
                                                    printf("Free DABPT entry found at index %d\n", dabptI);
                                                    printf("Free FNT entry found at index %d\n", fntI);
                                                    printf("File size: %ld bytes\n", filesize);
                                                }

                                                free(allocBlocks);
                                                free(bptList);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        else if (strcmp(command, "Get") == 0)
        {
            if (arg1 == NULL)
            {
                printf("\nIncorrect Usage: Get 'filename\n");
            }
            else if (fs == NULL || fs->formatted == 0)
            {
                printf("\nFilesystem does not exist or has not been formatted\n");
            }
            else
            {
                int fntI = -1;

                for (int i = 0; i < fs->numFN; i++)
                {
                    if(strcmp(fs->fnt[i].filename, arg1) == 0)
                    {
                        fntI = i;
                        break;
                    }
                }
                if (fntI == -1)
                {
                    printf("\nFile not found\n");
                }
                else
                {
                    int dabptI = fs->fnt[fntI].inode;

                    printf("\nFile found\n");
                    printf("FNT index: %d\n", fntI);
                    printf("DABPT index: %d\n", dabptI);

                    int bptBlock = fs->dabpt[dabptI].bptPointer;

                    printf("BPT block: %d\n", bptBlock);

                    BPTEntry bpt;

                    memcpy(&bpt, fs->disk + (bptBlock * BLOCK_SIZE), sizeof(BPTEntry));

                    FILE *output = fopen(arg1, "wb");

                    if (output == NULL)
                    {
                        printf("\nCould not create an external file\n");
                    }
                    else
                    {
                        int bytesRemain = fs->dabpt[dabptI].fileSize;
                        int currentBptBlock = bptBlock;

                        while (currentBptBlock != -1 && bytesRemain > 0)
                        {
                            BPTEntry currentBPT;

                            memcpy(&currentBPT, fs->disk + (currentBptBlock * BLOCK_SIZE), sizeof(BPTEntry));

                            for (int i = 0; i < 7 && bytesRemain > 0; i++)
                            {
                                if (currentBPT.pointers[i] != -1)
                                {
                                    int bytesToRead;

                                    if(bytesRemain > BLOCK_SIZE)
                                    {
                                        bytesToRead = BLOCK_SIZE;
                                    }
                                    else
                                    {
                                        bytesToRead = bytesRemain;
                                    }
                                    fwrite (fs->disk + (currentBPT.pointers[i] * BLOCK_SIZE), 1, bytesToRead, output);
                                    bytesRemain -= bytesToRead;
                                }
                            }
                            currentBptBlock = currentBPT.pointers[7];
                        }
                        fclose(output);
                        printf("\nFile was retrieved\n");
                    }
                }
            }
        }
        else if (strcmp(command, "Remove") == 0)
        {
            if (arg1 == NULL)
            {
                printf("\nIncorrect Usage: Remove 'filename'\n");
            }
            else if (fs == NULL || fs->formatted == 0)
            {
                printf("\nFilesystem does not exist or has not been formatted\n");
            }
            else
            {
                int fntI = -1;

                for (int i = 0; i < fs->numFN; i++)
                {
                    if (strcmp(fs->fnt[i].filename, arg1) == 0)
                    {
                        fntI = i;
                        break;
                    }
                }
                if (fntI == -1)
                {
                    printf("\nFile was not found\n");
                }
                else
                {
                    int dabptI = fs->fnt[fntI].inode;

                    printf("\nFile found\n");
                    printf("FNT index: %d\n", fntI);
                    printf("DABPT index: %d\n", dabptI);

                    int otherLinks = 0;

                    for (int i = 0; i < fs->numFN; i++)
                    {
                        if (i != fntI && fs->fnt[i].inode == dabptI)
                        {
                            otherLinks = 1;
                            break;
                        }
                    }
                    int currentBPTBlock = fs->dabpt[dabptI].bptPointer;

                    if (otherLinks == 0) 
                    {
                        while (currentBPTBlock != -1)
                        {
                            BPTEntry bpt;

                            memcpy(&bpt, fs->disk + (currentBPTBlock * BLOCK_SIZE), sizeof(BPTEntry));
                            
                            for (int i = 0; i < 7; i++)
                            {
                                if (bpt.pointers[i] != -1)
                                {
                                    fs->blockUsed[bpt.pointers[i]] = 0;
                                }
                            }
                            int nextBPTBlock = bpt.pointers[7];
                            fs->blockUsed[currentBPTBlock] = 0;
                            currentBPTBlock = nextBPTBlock;
                        }

                        fs->dabpt[dabptI].bptPointer = -1;
                        fs->dabpt[dabptI].fileSize = 0;
                        fs->dabpt[dabptI].lastTime = 0;
                        fs->dabpt[dabptI].usrname[0] = '\0';
                        
                    }
                    
                    fs->fnt[fntI].inode = -1;
                    fs->fnt[fntI].filename[0] = '\0';

                    memcpy(fs->disk + (fs->FNTStart * BLOCK_SIZE), fs->fnt, fs->numFN * sizeof(FNTEntry));

                    memcpy(fs->disk + (fs->DABPTStart * BLOCK_SIZE), fs->dabpt, fs->numDABPT * sizeof(DABPTEntry));

                    printf("\nFile removed successfully\n");
                }
            }
        }
        else if (strcmp(command, "Rename") == 0)
        {
            if (arg1 == NULL || arg2 == NULL)
            {
                printf("\nIncorrect Usage: Rename 'oldname' 'newname'\n");
            }
            else if (fs == NULL || fs->formatted == 0)
            {
                printf("\nFilesystem does not exist or has not been formatted\n");
            }
            else
            {
                int fntI = -1;

                for (int i = 0; i < fs->numFN; i++)
                {
                    if (strcmp(fs->fnt[i].filename, arg1) == 0)
                    {
                        fntI = i;
                        break;
                    }
                }
                if (fntI == -1)
                {
                    printf("\nFile was not found\n");
                }
                else
                {
                    int duplicate = 0;

                    for (int i = 0; i < fs->numFN; i++)
                    {
                        if (i != fntI && strcmp(fs->fnt[i].filename, arg2) == 0)
                        {
                            duplicate = 1;
                            break;
                        }
                    }

                    if (duplicate)
                    {
                        printf("\nCan't enter duplicate files\n");
                    }
                    else
                    {
                        strcpy(fs->fnt[fntI].filename, arg2);
                        memcpy(fs->disk + (fs->FNTStart * BLOCK_SIZE),
                            fs->fnt,
                            fs->numFN * sizeof(FNTEntry));

                        printf("\nFile renamed\n");
                    }
                }
            }
        }
        else if (strcmp(command, "User") == 0)
        {
            if (arg1 == NULL || arg2 == NULL)
            {
                printf("\nIncorrect Usage: User 'file' 'name'\n");
            }
            else if (fs == NULL || fs->formatted == 0)
            {
                printf("\nFilesystem does not exist or has not been formatted\n");
            }
            else
            {
                int fntI = -1;

                for (int i = 0; i < fs->numFN; i++)
                {
                    if (strcmp(fs->fnt[i].filename, arg1) == 0)
                    {
                        fntI = i;
                        break;
                    }
                }
                if (fntI == -1)
                {
                    printf("\nFile was not found\n");
                }
                else
                {
                    int dabptI = fs->fnt[fntI].inode;

                    strcpy(fs->dabpt[dabptI].usrname, arg2);
                    memcpy(fs->disk + (fs->DABPTStart * BLOCK_SIZE), fs->dabpt, fs->numDABPT * sizeof(DABPTEntry));
                    printf("\nUsername set to: %s\n", arg2);
                }
            }
        }
        else if (strcmp(command, "Link") == 0)
        {
            if (arg1 == NULL || arg2 == NULL)
            {
                printf("\nIncorrect Usage: Link 'oldname' 'newname'\n");
            }
            else if (fs == NULL || fs->formatted == 0)
            {
                printf("\nFilesystem does not exist or has not been formatted\n");
            }
            else
            {
                int fntI = -1;

                for (int i = 0; i < fs->numFN; i++)
                {
                    if (strcmp(fs->fnt[i].filename, arg1) == 0)
                    {
                        fntI = i;
                        break;
                    }
                }

                if (fntI == -1)
                {
                    printf("\nFile was not found\n");
                }
                else
                {
                    int existingFntI = -1;

                    for (int i = 0; i < fs->numFN; i++)
                    {
                        if (strcmp(fs->fnt[i].filename, arg2) == 0)
                        {
                            existingFntI = i;
                            break;
                        }
                    }

                    if (existingFntI != -1)
                    {
                        printf("\nCan't enter duplicate files\n");
                    }
                    else
                    {
                        int newFntI = -1;

                        for (int i = 0; i < fs->numFN; i++)
                        {
                            if (fs->fnt[i].inode == -1)
                            {
                                newFntI = i;
                                break;
                            }
                        }
                        if (newFntI == -1)
                        {
                            printf("\nNo free filename entries\n");
                        }
                        else
                        {
                            fs->fnt[newFntI].inode = fs->fnt[fntI].inode;
                            strcpy(fs->fnt[newFntI].filename, arg2);

                            memcpy(fs->disk + (fs->FNTStart * BLOCK_SIZE), fs->fnt, fs->numFN * sizeof(FNTEntry));

                            printf("\nLink created\n");
                        }
                    }
                    
                }
            }
        }
        else if (strcmp(command, "Unlink") == 0)
        {
            if (arg1 == NULL)
            {
                printf("Incorrect Usage: Unlink 'filename' \n");
            }
            else if (fs == NULL || fs->formatted == 0)
            {
                printf("\nFilesystem does not exist or has not been formatted\n");
            }
            else
            {
                int fntI = -1;

                for (int i = 0; i < fs->numFN; i++)
                {
                    if (strcmp(fs->fnt[i].filename, arg1) == 0)
                    {
                        fntI = i;
                        break;
                    }
                }

                if (fntI == -1)
                {
                    printf("\nFile was not found\n");
                }
                else
                {
                    int dabptI = fs->fnt[fntI].inode;

                    int otherLinks = 0;

                    for (int i = 0; i < fs->numFN; i++)
                    {
                        if (i != fntI && fs->fnt[i].inode == dabptI)
                        {
                            otherLinks = 1;
                            break;
                        }
                    }

                    if (otherLinks == 0)
                    {
                        printf("\nThis is the last link\n");

                        int currentBPTBlock = fs->dabpt[dabptI].bptPointer;

                        while (currentBPTBlock != -1)
                        {
                            BPTEntry bpt;

                            memcpy(&bpt,
                                fs->disk + (currentBPTBlock * BLOCK_SIZE),
                                sizeof(BPTEntry));

                            for (int i = 0; i < 7; i++)
                            {
                                if (bpt.pointers[i] != -1)
                                {
                                    fs->blockUsed[bpt.pointers[i]] = 0;
                                }
                            }

                            int nextBPTBlock = bpt.pointers[7];

                            fs->blockUsed[currentBPTBlock] = 0;

                            currentBPTBlock = nextBPTBlock;
                        }

                        fs->dabpt[dabptI].bptPointer = -1;
                        fs->dabpt[dabptI].fileSize = 0;
                        fs->dabpt[dabptI].lastTime = 0;
                        fs->dabpt[dabptI].usrname[0] = '\0';
                    }

                    fs->fnt[fntI].inode = -1;
                    fs->fnt[fntI].filename[0] = '\0';

                    memcpy(fs->disk + (fs->FNTStart * BLOCK_SIZE),
                        fs->fnt,
                        fs->numFN * sizeof(FNTEntry));

                    printf("\nLink removed successfully\n");
                }
            }    
        }
    }
    return 0;
} 