#ifndef DIRECTORY_ENTRY_H
#define DIRECTORY_ENTRY_H
#define file 11
#define BUFFER_SIZE 50
#include <stdint.h>
#include <time.h>
#include <stdbool.h>

typedef struct{

    //Block Info
    int startBlock;
    char fileName[256];
    int size;
    int flag;
    bool used;
    int type;
    // Time Stamps
    time_t created_time;
    time_t updated_time;
    time_t modified_time;
    //need to know where are 
    int index; 
    char parent[20];
    char *parentDir;
} DirectoryEntry;

// Initialization for a directory
DirectoryEntry * Dir(int numEntries, DirectoryEntry *parent);

#endif 
