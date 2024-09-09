#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <math.h>
#include "fsLow.h"
#include "mfs.h"
#include "vcb.h"
#include "directoryEntry.h"



# define NUM_DIRECTORY_ENTRIES 50

// allows us to allocate the memory ofcwdString
int dirLocate;
int bytesNeeded = NUM_DIRECTORY_ENTRIES * sizeof(DirectoryEntry);
int blockCnt;   // How many blocks is a directory entry
int bytesUsed;
int trueNumEntries; // Total number of possible entires
int totalBytesNeeded;    // Total number of bytes in a directory entry

int locate(char *name, DirectoryEntry * Dir){

    printf("\nHIHIHSDOFIHSDFIOHSDF\n");
    blockCnt = (bytesNeeded + vcb->blkSize -1)/vcb->totalBlocks;
    bytesUsed = blockCnt * vcb->blkSize;
    trueNumEntries = bytesUsed / sizeof(Dir);
    bytesNeeded = trueNumEntries * sizeof(Dir);

    dirLocate = Dir[0].startBlock;
    for(int i = 0; i< trueNumEntries; i++){
        // Locate the location of the directroy in its parrent directory
        if(strcmp(Dir[i].fileName, name)== 0){
            return i;
        }
    }
    // If directory is not found return -1
    return -1;
}

DirectoryEntry *load(DirectoryEntry * Dir){
    Dir = malloc(bytesUsed);
    LBAread(Dir, blockCnt, Dir->startBlock);
    return Dir;
};

char* dilm = "/";
DirectoryEntry* cwdPointer;
int parsePath(char *path, PPINFO * res){
    char dirPath[sizeof(path)];
    strcpy(dirPath, path);
    int location;
    // Load the root directory
    blockCnt = (bytesNeeded + vcb->blkSize -1)/vcb->totalBlocks;
    bytesUsed = blockCnt * vcb->blkSize;
    trueNumEntries = bytesUsed / sizeof(DirectoryEntry);
    bytesNeeded = trueNumEntries * sizeof(DirectoryEntry);
    DirectoryEntry *Dir = malloc(bytesUsed);

    // Read the root directory if path is an absolute path
    if(strcmp(&(dirPath[0]), dilm) == 0){
    LBAread(Dir, blockCnt, vcb->strRootDir);
        printf("  call in parse \n");

    }else{
        // Get the current working directory
        Dir = cwdPointer;
    };
    char *token = strtok(dirPath, dilm);
    // Find the location of token in current directory
    while(token != NULL){
    location = locate(token, Dir);
    
    // Directory not found, exit parse path
    if(location == -1){
        return -1;
    };
    // item at location in parent DIR is not a directory.
    // Exit parse path
    if(Dir[location].type != Dir){
        return -1;
    };
    token = strtok(NULL, dilm);
    // IF token is NULL we have reached the last element 
    // Store the parrent directory and the idex into PPINFO struct and exit
    if(token == NULL){
        // Parent directory
        res ->parentDir = Dir;
        // Index in the parent directory
        res -> index = location;
        return 0;
    };

    // Load the directory at index obtained from locate in parent directory
    Dir = load(&Dir[location]);
    };



}


int fs_mkdir(const char *pathname, mode_t mode) {
    // Allocate memory for a new PPINFO struct
     
    char * path = malloc(sizeof(pathname));
    strcpy(path,pathname);
    PPINFO *info = malloc(sizeof(PPINFO));
    int status = parsePath(path, info);
    if(status == -1){
        free(path);
        free(info);
        return -1;
    };
    
    
}

DirectoryEntry * init_Dir(int minEntries, DirectoryEntry * parent){
    blockCnt = (bytesNeeded + vcb->blkSize -1)/vcb->totalBlocks;
    bytesUsed = blockCnt * vcb->blkSize;
    trueNumEntries = bytesUsed / sizeof(DirectoryEntry);
    bytesNeeded = trueNumEntries * sizeof(DirectoryEntry);
    DirectoryEntry *DE = malloc(bytesUsed);

    for(int i =2; i< trueNumEntries; i++){
        DE[i].used = false;
    };
    time_t t = time(NULL);
    strcpy(DE[0].fileName, ".");
    DE[0].startBlock = malloc(blockCnt);
    DE[0].created_time = t;
    DE[0].modified_time = t;
    DE[0].updated_time = t;
    
    DE[0].size = bytesNeeded;

    strcpy(DE[1].fileName, "..");
    if(parent == NULL){
        // Do root dir
    };
    DE[1].created_time = t;
    DE[1].modified_time = t;
    DE[1].updated_time = t;
    DE[1].startBlock = malloc(blockCnt);
    DE[1].size = bytesNeeded;

    return DE;

};

fdDir * fs_opendir(const char *pathname){

}

struct fs_diriteminfo *fs_readdir(fdDir *dirp) {
    // Check if the directory pointer is NULL 
    if (dirp == NULL) {
        printf("The 'Item' directory cannot be found or accessed.");
        return NULL;
    }
    // Allocate memory for the fs_diriteminfo struct
    struct fs_diriteminfo *data = malloc(250);
    static int tracker = 0;
    // Allocate memory for a DirectoryEntry array 
    DirectoryEntry *Dir = malloc(60 * 52);
    if (dirp[tracker].d_reclen == 0) {
        // Reset the tracker variable and return NULL
        tracker = 0;
    return NULL;
    }
    // Read the directory entries from the disk into the DirectoryEntry array
    LBAread(Dir, 6, dirp[0].dirEntryPosition);
    // Get the index of the next entry to be read from the directory pointer
    int index = dirp[tracker].dirEntryPosition;
    // Check if the index is out of bounds 
    if (index > 52) {
    // Return NULL
    return NULL;
    }
    // Copy the name of the entry to the fs_diriteminfo struct's d_name field
    strcpy(data->d_name, Dir[index].fileName);
    data->d_reclen = dirp[tracker].d_reclen;
    data->fileType = 1;
    tracker++;
    // Return the fs_diriteminfo struct
    return data;

}

int fs_stat(const char *path, struct fs_stat *buf){
   // Allocate memory for a new PPINFO struct and store a pointer to it in info.
    PPINFO *info = malloc(sizeof(PPINFO));
    int index = info->index;
    DirectoryEntry *parent = info->parentDir;
    DirectoryEntry *Dir = load(&parent[index]);
    // Read the contents of block 6 into the Dir structure.
    LBAread(Dir, 6, vcb->blkSize);
    // Copy the contents of the path variable into the num variable.
    char num[150];
    strcpy(num, path);

    printf("copy statement \n");
    // Parse the num variable using the parsePath function and store the result in spot.
    int spot = parsePath(num, info);
        printf("parse path\n ");

    /*If the spot variable is equal to -1, 
    then print an error message and return -1.*/
    if (spot == -1) {
        printf("location path not found\n");
        return -1;
    }
    // Read the contents of block 6 at the given spot index into the Dir structure.
    LBAread(Dir, 6, spot);
    buf->st_blksiSze = vcb->blkSize;
    buf->st_blocks = ceil(((double)60 * 51.0) / vcb->blkSize);
    buf->st_createtime = Dir->created_time;
    buf->st_size = Dir->size;
    // Return 0 to indicate success.
    return 0;
}

// Return 1 if file else return 0
int fs_isFile(char * filename){
// Use parse path to get file location and parrent directory
PPINFO *info = malloc(sizeof(PPINFO));
int valid = parsePath(filename, info);
if(valid == -1){
    // Directory not found
    return -1;
}
// Check if index at postion in the parent directroy is a file
if(info->parentDir[info->index].type == file){
    free(info);
    return 1;
}else{
    free(info);
    return 0;
};
};

// Retrun 1 if dir, return 0 otherwise
int fs_isDir(char * pathname){
    DirectoryEntry dir;

    PPINFO *info = malloc(sizeof(PPINFO));
    int valid = parsePath(pathname, info);
    if(valid == -1){
        // Directory not found
        return -1;
    }
    // Check if item in the parent directory at index is a directory
    if(info->parentDir[info->index].type == dir.type){
        free(info);
        return 1;
    }else{
        free(info);
        return 0;
    };
};

//this function allows to close the directory
int fs_closedir(fdDir *dirp){
    dirp = NULL;
    free(dirp);
    return 1;
}

char * fs_getcwd(char *pathname, size_t size) {
    char * path;
    //parsepath to get what we're copying into buffer maybe?
    if (cwdPointer == NULL) {
        path = NULL; //replace with root directory
    }
    else {
        //*path = *cwdPointer.directory.name;
        memcpy(path, cwdPointer->fileName, sizeof(*cwdPointer->fileName));
    }
    if (size == 0 || size < strlen(path)) { //or if path length is too big
        return NULL;
    }
    memcpy(pathname, path, size); 
    return pathname;

}

int fs_setcwd(char *pathname) {
    PPINFO * info = malloc(sizeof(PPINFO));
    parsePath(pathname, info);
    //if parent index > 0
    /*DirectoryEntry * newDir = loadDir(parent[index]);*/
    /*free prior global cwdDir
    set cwdDir = newDir (parsepath starts from here next time)
    strcpy(cwdname, pathname)*/
} 