#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

/* ---------- Constants and global variables ---------- */

#define FAILED_EXIT -1
#define TRUE 1
#define FALSE 0
#define ENTRY_SIZE 32
#define SECTOR_SIZE 512
#define STR_BUFFER_SIZE 100

/* ---------- Structures ---------- */

typedef struct {
    char    filename[9];
    char    extension[3];
    char    attr;
    uint32_t    size;
    uint8_t     createTimeMicroSec;
    uint16_t    createTime;
    
}__attribute__ ((packed)) dirEntry_t;

/* ---------- Helper functions ---------- */

void extractEntry(dirEntry_t* entryPtr, char* dirPtr){
    int b;
    for(b = 0; b < 8 && dirPtr[b] != 0x00; b++){
        entryPtr->filename[b] = (char)dirPtr[b];
    }
    entryPtr->filename[b] = '\0';
}

void printDirectoryContents(char* parentName, dirEntry_t* entryPtr, int count){
    printf("Directory: %s\n", parentName);
    printf("=========================\n");
    for(int i = 0; i < count; i++){
        printf("\t%s\n", (entryPtr[i]).filename);
    }
}

void traverseDirectory(char* parentName, char* fileptr, int physicalSector) {
	char* dir = &fileptr[SECTOR_SIZE * physicalSector];
	int count = 0;
    dirEntry_t dirContents[16];

	while(dir[0] != 0x00) {	
        if (!(dir[11] & 0x08) && dir[0] != 0x2e) {
			extractEntry(&dirContents[count], dir);
            count++;
		}
        if((dir[11] & 0x10) && dir[0] != 0x2e){
            int logicalCluster = dir[26] + (dir[27] << 8);
            traverseDirectory(&dirContents[count-1].filename, fileptr, logicalCluster+31);   
		}
        
        dir+=32;
	}

    printDirectoryContents(parentName, dirContents, count);
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

    char rootDir[] = "Root Directory";
    // work with the disk
    traverseDirectory(rootDir, fileptr, 19);

    // clean
    munmap(fileptr, buffer.st_size);
    close(file);
 
    return 0;
}