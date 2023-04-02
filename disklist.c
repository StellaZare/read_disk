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
    char    extension[4];
    char    attr;
    int     creationMin;
    int     creationHour;
    int     creationDay;
    int     creationMonth;
    int     creationYear;
    int     logicalSector;
    uint32_t    size;
    
    
}__attribute__ ((packed)) dirEntry_t;

/* ---------- Helper functions ---------- */

void extractEntry(dirEntry_t* entryPtr, char* dirPtr){
    int b;
    for(b = 0; b < 8 && dirPtr[b] != 0x00; b++){
        entryPtr->filename[b] = (char)dirPtr[b];
    }
    entryPtr->filename[b] = '\0';
    for(b = 0; b < 3; b++){
        entryPtr->extension[b] = (char)dirPtr[8+b];
    }
    entryPtr->extension[b] = '\0';
    if (dirPtr[11] == 0x10){
        entryPtr->attr = 'D';
        entryPtr->logicalSector = dirPtr[26] + (dirPtr[27] << 8);
    }else{
        entryPtr->attr = 'F';

        entryPtr->creationYear = (((dirPtr[17] & 0xfe)) >> 1) + 1980;
		entryPtr->creationMonth = ((dirPtr[16] & 0xe0) >> 5) + (((dirPtr[17] & 0x01)) << 3);
		entryPtr->creationDay = (dirPtr[16] & 0x1f);
		entryPtr->creationHour = (dirPtr[15] & 0xf8) >> 3;
		entryPtr->creationMin = ((dirPtr[14] & 0xe0) >> 5) + ((dirPtr[15] & 0x7) << 3);
        
        entryPtr->size = (dirPtr[28] & 0xFF) + ((dirPtr[29] & 0xFF) << 8) + ((dirPtr[30] & 0xFF) << 16) + ((dirPtr[31] & 0xFF) << 24);
    }

}

void printDirectoryContents(char* parentName, dirEntry_t* entryPtr, int count){
    printf("%s\n", parentName);
    printf("====================\n");
    for(int i = 0; i < count; i++){
        if(entryPtr[i].attr == 'D'){
            printf("\tD            %s\n", entryPtr[i].filename);
        }
        else{
            printf("\tF %10d %s.%s\t%02d-%02d-%02d %02d:%02d\n", 
                entryPtr[i].size, entryPtr[i].filename, entryPtr[i].extension, 
                entryPtr[i].creationDay, entryPtr[i].creationMonth, entryPtr[i].creationYear,
                entryPtr[i].creationHour, entryPtr[i].creationMin ); 
        }
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
        dir+=32;
	}

    printDirectoryContents(parentName, dirContents, count);

    for(int i = 0; i < count; i++) {
        if(dirContents[i].attr == 'D'){
            traverseDirectory(dirContents[i].filename, fileptr, dirContents[i].logicalSector+31);
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

    char* rootDir = "Root Directory";
    // work with the disk
    traverseDirectory(rootDir, fileptr, 19);

    // clean
    munmap(fileptr, buffer.st_size);
    close(file);
 
    return 0;
}