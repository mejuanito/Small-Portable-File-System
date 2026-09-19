#ifndef FS_H
#define FS_H

#define BLOCK_SIZE 256

//struct for BPT
typedef struct {
    int pointers[8];
} BPTEntry;

//struct for the FNT
//assignment didn't specifiy, so I created a reasonable struct for it
typedef struct {
    char filename[56];
    int inode;
} FNTEntry;

//struct for DABPT
//assignment didn't specifiy, so I created a reasonable struct for it
typedef struct {
    int fileSize;
    long lastTime;
    int bptPointer;
    char usrname[40];
} DABPTEntry;

//metadata struct for FS
typedef struct {
    int numOfBlocks;
    int numFN;
    int numDABPT;
    int FNTStart;
    int DABPTStart;
    int formatted;
} FSHeader;

//struct for the filesystem
typedef struct {
    int numOfBlocks;
    int numFN;
    int numDABPT;
    int formatted;
    int FNTStart;
    int DABPTStart;
    int *blockUsed;
    unsigned char *disk;
    FNTEntry *fnt;
    DABPTEntry *dabpt;
    BPTEntry *bpt;
} fileSystem;

//prototypes
int freeBlock(fileSystem *fs);
fileSystem *createfileSystem(int numOfBlocks);
void destroyfileSystem(fileSystem *fs);

#endif