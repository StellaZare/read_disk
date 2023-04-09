#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
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

void printDirectoryContents(char* parentName, dirEntry_t* entryPtr, int count){
    printf("%s\n", parentName);
    printf("====================\n");
    for(int i = 0; i < count; i++){
        if(entryPtr[i].attr == 'D'){
            printf("\tD            %s\n", entryPtr[i].filename);
        }
        else{
            printf("\tF %10d %10.10s.%s\t%02d-%02d-%02d %02d:%02d\n", 
                entryPtr[i].size, entryPtr[i].filename, entryPtr[i].extension, 
                entryPtr[i].creationDay, entryPtr[i].creationMonth, entryPtr[i].creationYear,
                entryPtr[i].creationHour, entryPtr[i].creationMin ); 
        }
    }
}

void traverseDirectory(char* parentName, char* diskptr, int physicalSector) {
	char* dir = &diskptr[SECTOR_SIZE * physicalSector];
    int logicalSector = physicalSector - 31;
    int count = 0;
    int bytes = 0;
    dirEntry_t dirContents[100];

	while(dir[0] != 0x00) {	

        if (!(dir[11] & 0x08) && dir[0] != 0x2e) {
			extractDirectoryEntry(&dirContents[count], dir);
            count++;
		}
        dir+=32;
        bytes+=32;
        if(bytes >= 512 && logicalSector >= 2){
            logicalSector = getFatEntry(logicalSector, diskptr);
            bytes = 0;
            dir = &diskptr[SECTOR_SIZE * (logicalSector+31)];
        }
	}

    printDirectoryContents(parentName, dirContents, count);

    for(int i = 0; i < count; i++) {
        if(dirContents[i].attr == 'D'){
            printf("%s going to logical sector %d\n",dirContents[i].filename, dirContents[i].logicalSector);
            traverseDirectory(dirContents[i].filename, diskptr, (dirContents[i].logicalSector)+31);
        }
    }
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
        close(file);
        return FAILED_EXIT;
    }

    // create buffer and get size
    struct stat buffer;
    if(fstat(file, &buffer) == -1){
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

    // work with the disk
    char* rootDir = "Root Directory";
    traverseDirectory(rootDir, diskptr, 19);

    // clean
    munmap(diskptr, buffer.st_size);
    close(file);
 
    return 0;
}