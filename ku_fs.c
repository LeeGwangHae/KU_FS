#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLOCKSIZE 4096
#define NUMBLOCK 64
#define INODESIZE 256

typedef struct INODE{
    unsigned int fSize;
    unsigned int blocks;
    unsigned int pointer[12];
    unsigned char trashData[200];
}iNode; // size 256byte

typedef struct DIRECTORYDATA{
    char inum;
    char fName[3];
}directoryData;

void *diskAddress;
void *superBlockAddress;
void *iMapAddress; 
void *dMapAddress;
void *iNodeAddress;
void *dataBlockStartAddress;
directoryData *rootDirectory;
iNode *iNodeArray;

void initBMaps(){
    *(unsigned char *)iMapAddress = 224;
    *(unsigned char *)dMapAddress = 128;
}//  이미 사용 중인 데이터값 할당

void initRootDirectory(){
    iNodeArray = (iNode *)iNodeAddress;
    (iNodeArray + 2)->fSize = 4 * 80;
    (iNodeArray + 2)->blocks = 1;

     rootDirectory= (directoryData *)dataBlockStartAddress;
} // rootDirectory에 빈 데이터를 넣어줌

void initDisk(){
    diskAddress = calloc(NUMBLOCK, BLOCKSIZE);
    superBlockAddress = diskAddress;
    iMapAddress = diskAddress + BLOCKSIZE;
    dMapAddress = diskAddress + BLOCKSIZE * 2;
    iNodeAddress = diskAddress + BLOCKSIZE * 3;
    dataBlockStartAddress = diskAddress + BLOCKSIZE * 8;

    initRootDirectory();
    initBMaps();
}

void endOfFile(){
    for(int i = 0; i < BLOCKSIZE * NUMBLOCK; i++){
        printf("%.2x ", *((unsigned char *)diskAddress + i));
    }
}

void deleteFile(){

}

void writeFile(){

}

void readFile(){

}

int main(int argc, char *argv[]){
    FILE *file;
    char line[30];
    char *fileName;
    char *fileMode;
    long int byte;
    int count = 0;
    char *cutLine;
    /*
    FILE *fp, *fp2, *fp3, *fp4;
    fp = fopen("imap.txt", "w");
    fp2 = fopen("dmap.txt", "w");
    fp3 = fopen("dblock0.txt", "w");
    fp4 = fopen("iblock0.txt", "w");
    for(int i = 0; i < 4096; i++){
        fprintf(fp, "%.2x ", *((unsigned char *)iMapAddress + i));
    }

    for(int i = 0; i < 4096; i++){
        fprintf(fp2, "%.2x ", *((unsigned char *)dMapAddress + i));
    }

    for(int i = 0; i < 4096; i++){
        fprintf(fp3, "%.2x ", *((unsigned char *)dataBlockStartAddress + i));
    }

    for(int i = 512; i < 256 * 3; i++){
        fprintf(fp4, "%.2x ", *((unsigned char *)iNodeAddress + i));
    }
    fclose(fp);
    fclose(fp2);
    fclose(fp3);
    fclose(fp4);
    */
   //  비트 출력이 올바르게 되는지 확인하기 위한 output file t생성 코드
    if(argc < 2){
        printf("need input file. Try it again.\n");
        return 1;
    }
    initDisk();
    file = fopen(argv[1], "r");

    while(fgets(line, sizeof(line), file) != NULL){
        cutLine = strtok(line, " ");
        fileName = cutLine;  // getFileName from input.txt
        //printf("fileName : %s\n", fileName);
        cutLine = strtok(NULL, " ");
        fileMode = cutLine; // getFileMode from input.txt
        //printf("fileMode : %s\n",fileMode);
        if(*fileMode == 100){// 아스키코드 100 = "d" , 119 = "w", 114 = "r"
            cutLine = strtok(NULL, " ");
            //printf("%s, %s", fileName, fileMode);
            continue;
        }else{
            cutLine = strtok(NULL, " ");
            byte = atoi(cutLine);
            if(*fileMode == 119){
                writeFile();
            }else if(*fileMode == 114){
                readFile();
            }else{
                printf("Wrong fileMode!\n");
            }
            //printf("%s, %s, %ld", fileName, fileMode, byte);
        }
        //printf("\n");
    }
    free(diskAddress);

    return 0;
}