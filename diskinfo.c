#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

/* ---------- Constants and global variables ---------- */

#define FAILED_EXIT -1
#define TRUE 1
#define FALSE 0
#define SECTOR_SIZE 512
#define STR_BUFFER_SIZE 100

/* ---------- Helper functions ---------- */

void getOSName(char output[STR_BUFFER_SIZE], char* fileptr){
    // file pointer starting byte 3 length 8
    int idx;
    for(idx = 0; idx < 8; idx++){
        output[idx] = fileptr[idx+3];
    }
    output[idx] = '\0';
}

void getLabel(char output[STR_BUFFER_SIZE], char* fileptr){
    // file pointer bytes at index 43 length 11
    int idx;
    for(idx = 0; idx < 11; idx++){
        output[idx] = fileptr[idx+43];
    }

    output[idx] = '\0';
}


/* ---------- Main function ---------- */

int main(int argc, char* argv[]){
    // check input
    if(argc < 2){
        printf("Error: please include a disk image\n");
        return FAILED_EXIT;
    }
    
    // open file
    int file = open(argv[1], O_RDONLY);
    if(file == -1){
        printf("Error: unable to open disk image\n");
        return FAILED_EXIT;
    }

    // create buffer and get size
    struct stat buffer;
    if(fstat(file, &buffer) == -1){
        printf("Error: fstat() call failed\n");
        return FAILED_EXIT;
    }

    // load disk image into buffer
    char* fileptr = mmap(NULL, buffer.st_size, PROT_READ, MAP_SHARED, file, 0);
    if(fileptr == MAP_FAILED){
        printf("Error: mmap() call failed\n");
        return FAILED_EXIT;
    }

    // retrieve info
    char OSName[STR_BUFFER_SIZE];
    getOSName(OSName, fileptr);
    char label[STR_BUFFER_SIZE];
    getLabel(label, fileptr);

    // print info
    printf("OS Name: \t%s\n", OSName);
    printf("Label: \t%s\n", label);

    // clean
    munmap(fileptr, buffer.st_size);
    close(file);
 
    return 0;
}