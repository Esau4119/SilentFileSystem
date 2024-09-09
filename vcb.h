#ifndef VCB_H
#define VCB_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fsLow.h"
#include <stdint.h>
#define magNum 101

typedef struct VCB{
    char name[20];          // Storing name 
    uint64_t blkSize;       // BLockSize in bytes
    uint64_t totalBlocks;   // Number of blocks in our volume
    uint64_t freeBlocks;    // Number of freeblocks
    uint32_t magicNumber;   // our magic number
    uint64_t numFiles;      // number of files we have
    uint64_t maxSize;       // Biggest size we can reach
    uint64_t totalSize;     // How big we are
    uint64_t strFreeSpace;  // Location for start of freespace
    uint64_t strRootDir;    // Location for start of root
    uint64_t root_dir;      // storing root
    uint64_t free_space;    // storing freespace
    uint64_t allocateUnit;  // Size of file/dir units 
} VCB;
extern struct VCB *vcb;

#endif 
