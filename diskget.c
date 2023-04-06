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

int getFileEntry(char* filename, char* diskptr, int physicalSector, dirEntry_t* fileEntry){
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
                *fileEntry = currentDir;
                return 1;
            }
            memset(buffer, 0, STR_BUFFER_SIZE);
		}
        dir+=32;
	}
    // if file was not found in root directory 
    return FAILED_EXIT;
}

void copyFileEntry(char* diskptr, char* newFileptr, dirEntry_t* fileEntry){
    int logicalSector = (uint8_t)fileEntry->logicalSector;
    int fileSize = fileEntry->size;
    int bytesRemaining = fileSize;

    while(bytesRemaining > 0){
        int physicalSector = SECTOR_SIZE * (logicalSector + 31);
        printf("Bytes remaining: %d  FatEntry: %d  physSector: %d\n", bytesRemaining, logicalSector, physicalSector);

        int byte;
        for(byte = 0; byte < SECTOR_SIZE; byte++){
            if(bytesRemaining == 0){
                printf("\tDone copying file!\n");
                return;
            }
            newFileptr[fileSize-bytesRemaining] = (uint8_t)diskptr[physicalSector+byte];
            bytesRemaining--;
        }
        logicalSector = getFatEntry(logicalSector, diskptr);
        printf("NextFat: %d\n", logicalSector);
    }
    return; 
}


/* ---------- Main function ---------- */

int main(int argc, char* argv[]){
    // check input
    if(argc < 3){
        printf("Error: program should be invoked as follows:\n");
        printf("\t./diskget <diskfile> <filename>\n");
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

    // extract input filename
    char filename[STR_BUFFER_SIZE];
    int i;
    for(i = 0; argv[2][i] != '\0'; i++){
        filename[i] = toupper(argv[2][i]);
    }
    filename[i] = '\0';

    // struct to hold file entry details
    dirEntry_t fileEntry;
    int result = getFileEntry(filename, diskptr, 19, &fileEntry);
    if(result == FAILED_EXIT){
        printf("Error: file \"%s\" not found in root directory\n", filename);
        munmap(diskptr, buffer.st_size);
        close(file);
        return FAILED_EXIT;
    }

    //open a new file
    int newFile = open(filename, O_RDWR | O_CREAT, 0666);
    if (newFile < 0){
        printf("Error: could not open new file\n");
        close(file);
        munmap(diskptr, buffer.st_size);
        return FAILED_EXIT;
    }

    // set the size of the new file
    if (ftruncate(newFile, fileEntry.size) < 0) {
        printf("Error: ftruncate() call failed\n");
        close(file);
        munmap(diskptr, buffer.st_size);
        return FAILED_EXIT;
    }

    // check write access
    result = write(newFile, "", 1);
    if (result != 1) {
        close(file);
        close(newFile);
        munmap(diskptr, buffer.st_size);
        printf("Error: unable to write to new file\n");
        return FAILED_EXIT;
    }

    // load new memory into buffer
    char* newFileptr = mmap(NULL, fileEntry.size, PROT_READ|PROT_WRITE, MAP_SHARED, newFile, 0);
    if(newFileptr == MAP_FAILED){
        printf("Error: mmap() call failed\n");
        close(file);
        return FAILED_EXIT;
    }

    // copy file contents
    copyFileEntry(diskptr, newFileptr, &fileEntry);


    // clean
    munmap(newFileptr, fileEntry.size);
    close(newFile);
    munmap(diskptr, buffer.st_size);
    close(file);
 
    return 0;
}