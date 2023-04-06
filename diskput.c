#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "helper.h"

/* ---------- Constants and global variables ---------- */

#define FAILED_EXIT -1
#define TRUE 1
#define FALSE 0
#define ENTRY_SIZE 32
#define SECTOR_SIZE 512
#define STR_BUFFER_SIZE 100
#define MAX_FAT_ENTRIES 3072   //9 sectors x 512 bytes x 8 bits / 12 bit entries


/* ---------- Helper functions ---------- */

/*
    used to determine if a file with the same name alreasy exists in the
    root directory
*/
int rootContainsFile(char* diskptr, char* filename, int physicalSector){
    char* dir = &diskptr[SECTOR_SIZE * physicalSector];
    dirEntry_t currentDir;

	while(dir[0] != 0x00) {	
        if (!(dir[11] & 0x08) && dir[0] != 0x2e) {
			extractDirectoryEntry(&currentDir, dir);
            char buffer[STR_BUFFER_SIZE];
            memset(buffer, 0, STR_BUFFER_SIZE);
            strcat(buffer, currentDir.filename);
            strcat(buffer, ".");
            strcat(buffer, currentDir.extension);
            strcat(buffer, "\0");
            if(strcmp(buffer, filename)==0){
                return TRUE;
            }
            memset(buffer, 0, STR_BUFFER_SIZE);
		}
        dir+=32;
	}
    // if file was not found in root directory 
    return FALSE;
}

int getNextFATEntry(char* diskptr){
    int value;
    for(int entry = 2; entry < MAX_FAT_ENTRIES; entry++){
        value = getFatEntry(entry, diskptr);
        if(value == 0x00){
            return entry;
        }
    }
    return FAILED_EXIT;
}

void addRootEntry(char* name, int fileSize, uint32_t firstLogicalSector, char* diskptr){
    char* dir = diskptr + (SECTOR_SIZE * 19);
    while(dir[0] != 0x00){
        dir += 32;
    }

    int b;
    int end = FALSE;
    for(b = 0; b < 8; b++){
        if(name[b] == '.'){
            end = b;
        }
        if(!end){
            dir[b] = name[b];
        }else{
            dir[b] = ' ';
        }
    }
    for(b = 0; b < 3; b++){
        dir[b+8] = name[end+b+1];
    }

    dir[11] = 0;

    //printf("fls: %08x b26: %08x b27: %08x\n", firstLogicalSector, dir[26], dir[27]);
    dir[26] = (uint32_t)(firstLogicalSector & 0x000000FF);
    //printf("fls: %08x b26: %08x b27: %08x\n", firstLogicalSector, dir[26], dir[27]);
	dir[27] = (firstLogicalSector & 0x0000FF00) >> 8;
    //printf("fls: %08x b26: %08x b27: %08x\n", firstLogicalSector, dir[26], dir[27]);

    dir[28] = (fileSize & 0x000000FF);
	dir[29] = (fileSize & 0x0000FF00) >> 8;
	dir[30] = (fileSize & 0x00FF0000) >> 16;
	dir[31] = (fileSize & 0xFF000000) >> 24;
}

void copyFileToRootDir(char* inputFileName, char* inputFileptr, int inputFileSize, char* diskptr){
    int bytesRemaining = inputFileSize;
    int currFatEntry = getNextFATEntry(diskptr);

    addRootEntry(inputFileName, inputFileSize, currFatEntry, diskptr);
    
    while(bytesRemaining > 0){
        printf("currentFATEntry: %d - bytesRemaining: %d\n", currFatEntry, bytesRemaining);
        int physicalSector = SECTOR_SIZE * (currFatEntry + 31);

        int byte;
        for(byte = 0; byte < SECTOR_SIZE; byte++){
            if(bytesRemaining == 0 ){
                printf("setting entry %d to 0xFFF\n", currFatEntry);
                setFatEntry(0xFFF, currFatEntry, diskptr);
                return;
            }
            diskptr[physicalSector + byte] = (inputFileptr[inputFileSize - bytesRemaining] & 0xFF);
            bytesRemaining--;
        }
        setFatEntry(0X123, currFatEntry, diskptr);
        int nextFatEntry = getNextFATEntry(diskptr);
        setFatEntry(nextFatEntry, currFatEntry, diskptr);
        currFatEntry = nextFatEntry;
    }
    
}

/* ---------- Main function ---------- */

int main(int argc, char* argv[]){
    // check input
    if(argc < 3){
        printf("Error: program should be invoked as follows:\n");
        printf("\t./diskput <diskfile> [destpath] <filename>\n");
        return FAILED_EXIT;
    }
    
    // open file
    int file = open(argv[1], O_RDWR);
    if(file == FAILED_EXIT){
        printf("Error: unable to open disk image\n");
        return FAILED_EXIT;
    }

    // create buffer and get size
    struct stat buffer;
    if(fstat(file, &buffer) == FAILED_EXIT){
        printf("Error: fstat() call failed\n");
        close(file);
        return FAILED_EXIT;
    }

    // load disk image into buffer
    char* diskptr = mmap(NULL, buffer.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, file, 0);
    if(diskptr == MAP_FAILED){
        printf("Error: mmap() call failed\n");
        close(file);
        return FAILED_EXIT;
    }

    // do the same for the file given
    char* inputFileName = argv[argc-1];

    // extract input filename
    char filename[STR_BUFFER_SIZE];
    int i;
    for(i = 0; inputFileName[i] != '\0'; i++){
        filename[i] = toupper(inputFileName[i]);
    }
    filename[i] = '\0';
    
    int inputFile = open(inputFileName, O_RDONLY);
    if(inputFile == FAILED_EXIT){
        printf("Error: input file not found\n");
        munmap(diskptr, buffer.st_size);
        close(file);
        return FAILED_EXIT;
    }

    struct stat inputFileBuffer;
    if(fstat(inputFile, &inputFileBuffer) == FAILED_EXIT){
        printf("Error: fstat() call failed\n");
        munmap(diskptr, buffer.st_size);
        close(file);
        return FAILED_EXIT;
    }

    int inputFileSize = inputFileBuffer.st_size;
    char* inputFileptr = mmap(NULL, inputFileSize, PROT_READ, MAP_SHARED, file, 0);
    if(inputFileptr == MAP_FAILED){
        printf("Error: mmap() call failed\n");
        close(inputFile);
        munmap(diskptr, buffer.st_size);
        close(file);
        return FAILED_EXIT;
    }

    int diskSize = getDiskSize(diskptr);
    int freeSize = getFreeSize(diskSize, diskptr);
    if(freeSize < inputFileSize){
        printf("Error: not enough space on the disk\n");
        munmap(inputFileptr, inputFileSize);
        close(inputFile);
        munmap(diskptr, buffer.st_size);
        close(file);
    }

    if(argc == 3){
        int existsInRoot = rootContainsFile(diskptr, filename, 19);
        if(existsInRoot){
            printf("Error: file already exists in root directory\n");
            munmap(inputFileptr, inputFileSize);
            close(inputFile);
            munmap(diskptr, buffer.st_size);
            close(file);
            return FAILED_EXIT;
        }
        printf("\tAdding file to root directory\n");
        copyFileToRootDir(filename, inputFileptr, inputFileSize, diskptr);
    }

    // clean
    munmap(inputFileptr, inputFileSize);
    close(inputFile);

    munmap(diskptr, buffer.st_size);
    close(file);
 
    return 0;
}