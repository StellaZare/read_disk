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


/* ---------- Helper functions ---------- */
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

int addFileToRootDir(char* inputFileName, char* inputFileptr, int inputFileSize, char* diskptr){
    return 0;
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
    int file = open(argv[1], O_RDONLY);
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
    char* diskptr = mmap(NULL, buffer.st_size, PROT_READ, MAP_SHARED, file, 0);
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

    int inputFile = open(filename, O_RDONLY);
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
        addFileToRootDir(inputFileName, inputFileptr, inputFileSize, diskptr);
    }

    // clean
    munmap(inputFileptr, inputFileSize);
    close(inputFile);
    munmap(diskptr, buffer.st_size);
    close(file);
 
    return 0;
}