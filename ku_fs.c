#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLOCKSIZE 4096
#define NUMBLOCK 64
#define INODESIZE 256

typedef struct INODE {
    unsigned int fSize;
    unsigned int blocks;
    unsigned int pointer[12];
    unsigned char trashData[200];
} iNode; // size 256byte

typedef struct DIRECTORYDATA {
    char inum;
    char fName[3];
} directoryData;

void *diskAddress;
void *superBlockAddress;
void *iMapAddress;
void *dMapAddress;
void *iNodeAddress;
void *dataBlockStartAddress;
directoryData *rootDirectory;
iNode *iNodeData;

void initBMaps() {
    *(unsigned char *) iMapAddress = 224;
    *(unsigned char *) dMapAddress = 128;
}//  이미 사용 중인 데이터값 할당

void initRootDirectory() {
    iNodeData = (iNode *) iNodeAddress;
    (iNodeData + 2)->fSize = 4 * 80;
    (iNodeData + 2)->blocks = 1;

    rootDirectory = (directoryData *) dataBlockStartAddress;
} // rootDirectory에 빈 데이터를 넣어줌

void initDisk() {
    diskAddress = calloc(NUMBLOCK, BLOCKSIZE);
    superBlockAddress = diskAddress;
    iMapAddress = diskAddress + BLOCKSIZE;
    dMapAddress = diskAddress + BLOCKSIZE * 2;
    iNodeAddress = diskAddress + BLOCKSIZE * 3;
    dataBlockStartAddress = diskAddress + BLOCKSIZE * 8;

    initRootDirectory();
    initBMaps();
}

void endOfFile() {
    for (int i = 0; i < BLOCKSIZE * NUMBLOCK; i++) {
        unsigned char tmp = *((unsigned char *) diskAddress + i);
        //if (tmp){
            printf("%.2x ", tmp);
        //} 
    }
}

int checkUsableDataBlock() {
    int usingDataBlock = 0;
    unsigned char dataMask = 128;
    unsigned char checkDataBlockEmpty;
    for (int i = 0; i < 56; i++) {
        checkDataBlockEmpty = (*(unsigned char *) (dMapAddress + i / 8)) & (dataMask >> i % 8);
        if (checkDataBlockEmpty == 0) {
            usingDataBlock++;
        } else {
            continue;
        }
        if (dataMask == 1) {
            dataMask = 128;
        }
    }
    return usingDataBlock;
}

int checkFileExist(char *fileName) {
    for (int i = 0; i < 80; i++) {
        if (*((rootDirectory + i)->fName) == *fileName && *((rootDirectory + i)->fName + 1) == *(fileName+1)) {
            if((rootDirectory + i)->inum == 0){
                return 0;
            }else{
                return 1;
            }
        } else {
            continue;
        }
    }
    return 0;
}

void inputData2Directory(int iNum, char *fileName) {
    for (int i = 0; i < 80; i++) {
        if ((rootDirectory + i)->inum == 0) {
            (rootDirectory + i)->inum = iNum;
            *((rootDirectory + i)->fName) = *fileName;
            *((rootDirectory + i)->fName+1) = *(fileName+1);
            break;
        }
    }
}

void inputData2DataBlock(iNode *inode, char *fileName, int byte) {
    char file_Name = fileName[0];
    int blockMapNum;
    for (int i = 0; i < byte; i++) {
        blockMapNum=inode->pointer[i/4096];
        if (blockMapNum != 0) {
            *((char *)dataBlockStartAddress + blockMapNum * BLOCKSIZE +(i % BLOCKSIZE)) = file_Name;
        }
    }
}

void writeFile(char *fileName, long int byte) {
    unsigned char iNodeMask = 128;
    unsigned char dataMask = 128;
    unsigned char checkDataBlockEmpty;
    unsigned char checkImapEmpty;
    int iNodeNum;
    int dataBlockNum;
    int count = 0;
    long int divideNum = 0;
    if(byte%BLOCKSIZE == 0){
        if(byte / BLOCKSIZE != 0){
            divideNum = byte / BLOCKSIZE;
        }
    }else{
            divideNum = byte / BLOCKSIZE + 1;
    }
    if (divideNum > 12) { // 1개의; 파일이  48kb를 넘어가면 입력 불가능
        printf("No space!\n");
    } else {
        if (divideNum > checkUsableDataBlock()) {// 사용 가능한 data block이 필요한 data block보다 작으면 입력 불가능
            printf("No space!\n");
        } else {
            if (!checkFileExist(fileName)) {
                for (int i = 0; i < 80; i++) {
                    checkImapEmpty = *(unsigned char *) (iMapAddress + i / 8) & (iNodeMask >> (i % 8));
                    if (checkImapEmpty == 0) {
                        iNodeNum = i;
                        *(unsigned char *) (iMapAddress + i / 8) = (*(unsigned char *) (iMapAddress + i / 8) | (iNodeMask >> (i % 8)));
                        break;
                    } else {
                        continue;
                    }
                }
                while (count < divideNum) {
                    for (int i = 0; i < 56; i++) {
                        checkDataBlockEmpty = (*(unsigned char *) (dMapAddress + i / 8) & (dataMask >> (i % 8)));
                        if (checkDataBlockEmpty == 0) {
                            dataBlockNum = i;;
                            *(unsigned char *) (dMapAddress + i / 8) = (*(unsigned char *) (dMapAddress + i / 8) | (dataMask >> (i % 8)));
                            break;
                        } else {
                            continue;
                        }
                        if (dataMask == 1) {
                            dataMask = 128;
                        }
                    }
                    (iNodeData + iNodeNum)->pointer[count] = dataBlockNum;
                    count++;
                }
                (iNodeData + iNodeNum)->fSize = byte;
                (iNodeData + iNodeNum)->blocks = divideNum;
                inputData2Directory(iNodeNum, fileName);
                inputData2DataBlock(iNodeData+iNodeNum, fileName, byte);
            }else {
                printf("Already exist!\n");
            }
        }
    }
}

void deleteFile(char* fileName) {
    int getInodeNum = 0;
    unsigned int deleteDataSize;
    unsigned int deleteDataBlockNum;
    char dataMapMask = 1;
    char iMapMask = 1;
    int divideNum = 0;
    for (int i = 0; i < 80; i++) {
        if ((*((rootDirectory + i)->fName) == *fileName) && *((rootDirectory + i)->fName + 1) == *(fileName+1)) {
            getInodeNum = (rootDirectory + i)->inum;
            if(getInodeNum == 0){
                continue;
            }else{
                (rootDirectory + i)->inum = 0;
                break;
            }
        }else {
            continue;
        }
    }
    if(getInodeNum == 0){
        printf("No such file\n");
    }else{
        iMapMask = 1 << 7 - (getInodeNum%8);
        *(unsigned char *)(iMapAddress + getInodeNum/8) = (*(unsigned char *)(iMapAddress + getInodeNum/8)) - iMapMask; 
        deleteDataSize = (iNodeData + getInodeNum)->fSize;
        if(deleteDataSize%BLOCKSIZE == 0){
            if(deleteDataSize / BLOCKSIZE != 0){
                divideNum = deleteDataSize / BLOCKSIZE;
            }
        }else{
            divideNum = deleteDataSize / BLOCKSIZE + 1;
        }

        for(int i = 0; i < divideNum; i++){
            deleteDataBlockNum = *((iNodeData + getInodeNum)->pointer + i);
            dataMapMask = 1 << 7 - (deleteDataBlockNum%8);
            *(char *)(dMapAddress + deleteDataBlockNum/8) = (*(unsigned char *)(dMapAddress + deleteDataBlockNum/8)) - dataMapMask;
        }
    }
}

void readFile(char *fileName, unsigned int byte) {
    int iNodeNum=0;
    unsigned int size = byte;
    unsigned int min = size;
    for (int i = 0; i < 80; i++) {
        if(strcmp((rootDirectory+i)->fName, fileName)==0){ 
            if((rootDirectory+i)->inum!=0){
                iNodeNum=(rootDirectory+i)->inum;
                break;
            }
            if ((rootDirectory+i)->inum==0){
                printf("No such file\n");
                return;
            }
        }
    }
    if(iNodeNum == 0){
        printf("No such file\n");
        return;
    }
    if(size>(iNodeData+iNodeNum)->fSize){
        min =(iNodeData+iNodeNum)->fSize;
        
    }
    unsigned int blocks;
    if(min % BLOCKSIZE == 0){
        blocks = min / BLOCKSIZE;
    }else{
        blocks = min / BLOCKSIZE + 1; 
    }

    unsigned int reminder = min % BLOCKSIZE;
    int *pointers = (int *)malloc(sizeof(int)*blocks);
    for(int i = 0; i < blocks ; i++){
        *(pointers + i) = (iNodeData + iNodeNum)->pointer[i];
    }
    if(reminder == 0){
        for(int i = 0; i < blocks; i++){
            for(int j = 0; j < BLOCKSIZE; j++){
                printf("%c", *((unsigned char *)(dataBlockStartAddress +BLOCKSIZE * pointers[i]) + j));
            }
        }
    }

    else {
        int i;
        for(i = 0; i < blocks - 1; i++){
            for(int j = 0; j < BLOCKSIZE; j++){
                printf("%c", *((unsigned char *)(dataBlockStartAddress + BLOCKSIZE * pointers[i]) + j));
            }
        }
        for(int j = 0; j < reminder; j++){
                printf("%c", *((unsigned char *)(dataBlockStartAddress + BLOCKSIZE * pointers[i]) + j));
            }
    }
    printf("\n");  
}

int main(int argc, char *argv[]) {
    FILE *file;
    char line[30];
    char *fileName;
    char *fileMode;
    long int byte;
    int count = 0;
    char *cutLine;
    

    if (argc < 2) {
        printf("need input file. Try it again.\n");
        return 1;
    }
    initDisk();
    file = fopen(argv[1], "r");

    while (fgets(line, sizeof(line), file) != NULL) {
        cutLine = strtok(line, " ");
        fileName = cutLine;  // getFileName from input.txt
        cutLine = strtok(NULL, " ");
        fileMode = cutLine; // getFileMode from input.txt
        if (*fileMode == 100) {// 아스키코드 100 = "d" , 119 = "w", 114 = "r"
            deleteFile(fileName);
            continue;
        } else {
            cutLine = strtok(NULL, " ");
            byte = atol(cutLine);
            if (*fileMode == 119) {
                writeFile(fileName, byte);
            } else if (*fileMode == 114) {
                readFile(fileName, byte);
            } else {
                printf("Wrong fileMode!\n");
            }
        }
    }
    endOfFile();
    fclose(file);
    free(diskAddress);
    return 0;
}