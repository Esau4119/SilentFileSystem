#include <stdlib.h>
#include <stdio.h>

#include "fsLow.h"
#include "vcb.h"



uint64_t FreeSpace(struct VCB *vcb) {
    //initializing our blocksize  
    int blocksize = vcb->blkSize;
    unsigned char bitArray[5*blocksize];
    int currentBlock = 0;

    //Uisng our first 6 bits
    for(int i =0; i < 6; i++){
        // Finding out bit
        int byteIndex = i/8;
       
        // Finding Location using 8 bits per byte
        int bitIndex = i % 8;
        
        //Setting the bit as used using bit opts & 1
        bitArray[byteIndex] = bitArray[byteIndex]|1 << bitIndex;

        //incrementing block
        currentBlock = currentBlock +1; 
    };

    // Initializing 5 blocks 
    LBAwrite(bitArray, 5, 1);

    // Setting location for freespace
    vcb->strFreeSpace = 1;
    // Return free space
    return vcb->strFreeSpace;
};


