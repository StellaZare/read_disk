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
    char* fileptr = mmap(NULL, buffer.st_size, PROT_READ, MAP_SHARED, file, 0);
    if(fileptr == MAP_FAILED){
        printf("Error: mmap() call failed\n");
        close(file);
        return FAILED_EXIT;
    }

    // do the same for the file given
    char* inputFileName = argv[argc-1];
    int inputFile = open(inputFileName, O_RDONLY);
    if(inputFile == FAILED_EXIT){
        printf("Error: input file not found\n");
        return FAILED_EXIT;
    }

    struct stat inputFileBuffer;
    if(fstat(inputFile, &inputFileBuffer) == FAILED_EXIT){
        printf("Error: fstat() call failed\n");
        close(file);
        return FAILED_EXIT;
    }

    int inputFileSize = inputFileBuffer.st_size;
    char* intputFileptr = mmap(NULL, inputFileSize, PROT_READ, MAP_SHARED, file, 0);
    if(intputFileptr == MAP_FAILED){
        printf("Error: mmap() call failed\n");
        close(file);
        return FAILED_EXIT;
    }

    int diskSize = getDiskSize(fileptr);
    int freeSize = getFreeSize(diskSize, fileptr);

    // clean
    munmap(intputFileptr, inputFileSize);
    close(inputFile);
    munmap(fileptr, buffer.st_size);
    close(file);
 
    return 0;
}