/**************************************************************
* Class:  CSC-415-0# Fall 2021
* Names: Esau Bojorquez Medina
* GitHub Name: Esau4119
* Group Name: The Silent Group
* Project: Basic File System
*
* File: b_io.c
*
* Description: Basic File System - Key File I/O Operations
*
**************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>			// for malloc
#include <string.h>			// for memcpy
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "b_io.h"

#include "directoryEntry.h"
#include "fsLow.h"
#include "mfs.h"

#define MAXFCBS 20
#define B_CHUNK_SIZE 512

typedef struct b_fcb
	{
	/** TODO add al the information you need in the file control block **/
	char * buf;		//holds the open file buffer
	int index;		//holds the current position in the buffer
	int buflen;		//holds how many valid bytes are in the buffer
	int bufferPos;
	DirectoryEntry * dirEnt; 
	} b_fcb;
	
b_fcb fcbArray[MAXFCBS];

int startup = 0;	//Indicates that this has not been initialized

//Method to initialize our file system
void b_init ()
	{
		
	//init fcbArray to all free
	for (int i = 0; i < MAXFCBS; i++)
		{	


		fcbArray[i].buf = NULL; //indicates a free fcbArray
		fcbArray[i].dirEnt = NULL; 
		//Marking start of file
        fcbArray[i].bufferPos = 0; 
		}
		
	startup = 1;
	}

//Method to get a free FCB element
b_io_fd b_getFCB ()
	{
	for (int i = 0; i < MAXFCBS; i++)
		{
		if (fcbArray[i].buf == NULL)
			{
			return i;		//Not thread safe (But do not worry about it for this assignment)
			}
		}
	return (-1);  //all in use
	}
	
// Interface to open a buffered file
// Modification of interface for this assignment, flags match the Linux flags for open
// O_RDONLY, O_WRONLY, or O_RDWR
b_io_fd b_open (char * filename, int flags){
	
	//Initialize our system
	if (startup == 0) b_init();  
		
	// This is where you are going to want to call GetFileInfo and b_getFCB
    b_io_fd returnFd = b_getFCB();
	fcbArray[returnFd].dirEnt = GetFileInfo(filename);

	//allocate memory for our buffer  
	
	fcbArray[returnFd].buf = (char*) malloc(B_CHUNK_SIZE * sizeof(char));
	
	
	//reset the current buffer's position
	fcbArray[returnFd].bufferPos = 0; 
    if (returnFd < 0) {
        return -1; 
    }

	//if the file or buffer is not found not found 
    if (fcbArray[returnFd].bufferPos == NULL ||fcbArray[returnFd].buf == NULL) {
        return -1; 
    }
	if(returnFd == -1){
		return -1;
	};
		// Matching flags to linux flags
	if (flags == O_RDONLY) {; 
		// Read only
		fcbArray[returnFd].dirEnt->flag = O_RDONLY;
	} else if (flags == O_WRONLY) {
		// Write only
		fcbArray[returnFd].dirEnt->flag = O_WRONLY; 
		
	} else if (flags == O_RDWR) {
		// Read and write
		fcbArray[returnFd].dirEnt->flag = O_RDWR; 
		
	} else if (flags == (O_WRONLY | O_TRUNC)) {
		// Truncate: delete the contents of the file without deleting the file
		fcbArray[returnFd].dirEnt->size = 0;
		fcbArray[returnFd].bufferPos = 0;
		fcbArray[returnFd].index = 0;
	}
	return returnFd;

}



// Interface to seek function	
int b_seek (b_io_fd fd, off_t offset, int whence)
	{
	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}

	//Marking original Starting position
	int start = fcbArray[fd].bufferPos;
	if (whence == SEEK_END) {
		//End of the file, offset != 0 probably causes an overload
		fcbArray[fd].bufferPos = fcbArray[fd].bufferPos + fcbArray[fd].dirEnt->size;
	} else if (whence == SEEK_SET) {
		// Start of the file is with offset
		fcbArray[fd].bufferPos = 0 + offset;
	} else if (whence == SEEK_CUR) {
		// No change to file position before offset
		fcbArray[fd].bufferPos += offset;
	}
	int bytes= abs(fcbArray[fd].bufferPos - start);
	return bytes; 
	}



// Interface to write function	
int b_write (b_io_fd fd, char * buffer, int count)
	{
	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}
		
		
	return (0); //Change this
	}



// Interface to read a buffer

// Filling the callers request is broken into three parts
// Part 1 is what can be filled from the current buffer, which may or may not be enough
// Part 2 is after using what was left in our buffer there is still 1 or more block
//        size chunks needed to fill the callers request.  This represents the number of
//        bytes in multiples of the blocksize.
// Part 3 is a value less than blocksize which is what remains to copy to the callers buffer
//        after fulfilling part 1 and part 2.  This would always be filled from a refill 
//        of our buffer.
//  +-------------+------------------------------------------------+--------+
//  |             |                                                |        |
//  | filled from |  filled direct in multiples of the block size  | filled |
//  | existing    |                                                | from   |
//  | buffer      |                                                |refilled|
//  |             |                                                | buffer |
//  |             |                                                |        |
//  | Part1       |  Part 2                                        | Part3  |
//  +-------------+------------------------------------------------+--------+
int b_read (b_io_fd fd, char * buffer, int count)
	{

	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}

	if (fcbArray[fd].dirEnt == NULL){		//File not open for this descriptor

		return -1;
	}	

	int bRead = 0; //track the number of bytes read so far
	
	//Read until the end of the file
	while (fcbArray[fd].bufferPos < fcbArray[fd].dirEnt->size 
			&& bRead < count  ) {
		// calculate the bytes left in the current buffer
		int leftOver = 
			B_CHUNK_SIZE - fcbArray[fd].bufferPos % B_CHUNK_SIZE;
		// track how many bytes are left to be read,
		int bytesToRead = count - bRead;

		//Loading the next chunck if we are at the end. 
		// If not at the end, we track the remaining bytes to read. 
		if (leftOver == 0) {
			//the only function you call to get data is LBAread.
		  LBAread(fcbArray[fd].buf,
		   fcbArray[fd].dirEnt->index + fcbArray[fd].bufferPos / B_CHUNK_SIZE,1);
		}else if(bytesToRead > leftOver){
			bytesToRead = leftOver;
		}

	  // memcopy data that is in our buffer to provided buffer
		memcpy(buffer + bRead,
		 fcbArray[fd].buf + fcbArray[fd].bufferPos % B_CHUNK_SIZE,
		 bytesToRead);
		fcbArray[fd].bufferPos= fcbArray[fd].bufferPos + bytesToRead;
		bRead = bRead + bytesToRead; 
	}

	// if we've read to the end of the file, return 0
	if (fcbArray[fd].bufferPos == fcbArray[fd].dirEnt->size)return 0;
	return bRead;
	}
	
// Interface to Close the file	
int b_close (b_io_fd fd)
	{
		if (startup == 0) {
		b_init(); //Initialize our system if it's not initialized
	}

	if (fd < 0 || fd >= MAXFCBS) {
		return -1;
	}

	// Reset Buffer
	fcbArray[fd].buf = NULL;
	// Freeing buffer
	free(fcbArray[fd].buf);
	// resetting buffer position 
	fcbArray[fd].bufferPos = 0;

	}
