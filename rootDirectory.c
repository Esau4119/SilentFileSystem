#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#include "fsLow.h"
#include "vcb.h"
#include "directoryEntry.h"


#define directoryEntries 50
#define DIRECTORY_TYPE "directory"

uint64_t rootDirectory(VCB *vcb) {

    //Grabbing the size of root directory
    uint64_t rootSize = sizeof(DirectoryEntry) *directoryEntries;
    //Blocks need to store root directory
    uint64_t blockNeeded = (rootSize + vcb->blkSize - 1) /vcb->totalBlocks;
    //Allocating memory for our root 
    DirectoryEntry *root = malloc(blockNeeded * vcb->blkSize);

    if (root == NULL) {
        printf("ERROR: MEMORY FOR ROOT DIRECTORY NOT ALLOCATED\n");
        return 0;
    }

    //Initializing entries to be ready for use
    for (uint64_t i = 0; i < directoryEntries; i++) {
        // Setting everything to zero
        root[i].startBlock = 0;
        root[i].size = 0;
        root[i].created_time = 0;
        root[i].updated_time = 0;
        strcpy(root[i].parent, "");

    }
  //Directory entries: "." & ".."
    for (uint64_t i = 0; i < 1; i++) {
        if( i == 0){
            strcpy(root[i].fileName, ".");
        }else{
            strcpy(root[i].fileName, "..");
        }
        // setting up our root attributes
        root[i].size =rootSize;
        root[i].startBlock= vcb->strRootDir;
        root[i].created_time= time(NULL);
        root[i].updated_time= time(NULL);
        snprintf(root[i].parent, sizeof(root[i].parent),"%d", getuid());
    }

    // Writing to disk
    LBAwrite(root, blockNeeded,vcb->strRootDir);

    // Freeing malloc 
    free(root);

    return vcb->strRootDir;
}
