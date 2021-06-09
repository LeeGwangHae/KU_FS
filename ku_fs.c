#include <stdio.h>
#define blockSize 4096
#define numBlock 64

typedef struct INODE{
    unsigned int fSize;
    unsigned int blocks;
    unsigned int pointer[12];
    char trashData[200];
}iNode; // size 256byte

typedef struct ROOTDIRECTORY{
    char inum;
    char fName[3];
}directory;

void *diskAddress;
void *superBlockAddress;
void *iMapAddress;
void *dMapAddress;
void *iNodeAddress;
void *dataBlockAddress;
directory rootDirectory;

void reservedInode(){
    
}

void initRootDirectory(){
    rootDirectory.inum = 2;

}

void initDisk(){
    diskAddress = malloc(numBlock * blockSize);
    superBlockAddress = diskAddress;
    iMapAddress = superBlockAddress + blockSize * 1;
    dMapAddress = iMapAddress + blockSize * 1;
    iNodeAddress = dMapAddress + blockSize * 1;
    dataBlockAddress = iNodeAddress + blockSize * 5;
    memset(diskAddress, 0, sizeof(numBlock * blockSize));



}

int main(int argc, char *argv[]){

}