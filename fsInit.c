/**************************************************************
* Class:  CSC-415-0# Fall 2021
* Names: 
* GitHub Name:
* Group Name:
* Project: Basic File System
*
* File: fsInit.c
*
* Description: Main driver for file system assignment.
*
* This file is where you will start and initialize your system
*
**************************************************************/


#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>

#include "fsLow.h"
#include "mfs.h"
#include "vcb.h"
#include "directoryEntry.h"
#include "freeSpace.c"
#include "rootDirectory.c"
//Initializing ouir VCB structure

struct VCB *vcb;

int initFileSystem (uint64_t numberOfBlocks, uint64_t blockSize)
	{
	printf ("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);
	/* TODO: Add any code you need to initialize your file system. */

	// Allocating memory for VCB
	vcb = malloc(blockSize);
	memset(vcb, 1, blockSize);

	//Setting VCB data
    vcb->blkSize = blockSize;
    vcb->totalBlocks = numberOfBlocks;
	LBAread(vcb, 1, 1);

	//Making sure our magic numbers match
	if(vcb->magicNumber != magNum  ){
		printf("Formatting required.\n");
		// Didnt match, so now we free vcb
		free(vcb);

		//reallocation memory with blocksize
		vcb = malloc(blockSize);
		memset(vcb, 1, blockSize);
    	vcb->blkSize = blockSize;
    	vcb->totalBlocks = numberOfBlocks;

		// matching numbers
		vcb->magicNumber = magNum;
	}

	// Error handling for vcb writing
	if(LBAwrite(vcb, 1 , 1) != 1){
		printf("An error has occurred while writing to the VCB.\n");
		return -1;
	}

	// Initialize free space
    uint64_t freeSpace = FreeSpace(vcb);
	// Storing freespace loation in vcb
	vcb->free_space = freeSpace;
	if(vcb->free_space  != 1){
		printf("Failed to initialize the free space\n");
		free(vcb);
		return -1;
	}
	
	//initialize the root directory
	uint64_t rootdir = rootDirectory(vcb);
	vcb->root_dir = rootdir;

	// Error handling for vcb writing
	if(vcb->root_dir == 0){
		printf("Failed to initialize the root directory\n");
		free(vcb);
		return -1;
	}

	//frees the allocated memory of vcb
	free(vcb);
	return 0;
	}
	
	
void exitFileSystem ()
	{
	printf ("System exiting\n");
	}