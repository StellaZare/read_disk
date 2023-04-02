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

void getOSName(char output[STR_BUFFER_SIZE], char* fileptr){
    // file pointer starting byte 3 length 8
    int idx;
    for(idx = 0; idx < 8; idx++){
        output[idx] = fileptr[idx+3];
    }
    output[idx] = '\0';
}

void getLabel(char output[STR_BUFFER_SIZE], char* fileptr){
    int idx;
    // if label in boot sector: start byte 43 length 11 bytes
    for(idx = 0; idx < 11; idx++){
        output[idx] = fileptr[idx+43];
    }

    // if label in root directory: traverse root directory 
    if(output[0] == ' '){
        fileptr = &fileptr[SECTOR_SIZE*19];
        while(fileptr[0] != 0x00){
            if(fileptr[11] == 0x08){
                for(idx = 0; idx < 11; idx++){
                    output[idx] = fileptr[idx];
                }   
            }
            // incerement to the next sector
            fileptr = &fileptr[32];
        }
    }
    output[idx] = '\0';
}

int getDiskSize(char* fileptr){
    int numSectors = fileptr[19] + (fileptr[20] << 8);
    return (numSectors*SECTOR_SIZE);
}

int getFreeSize(int diskSize, char* fileptr){
    int numFreeSectors = 0;
    int numSectors = fileptr[19] + (fileptr[20] << 8);

    // traverse FAT table given one entry per cluster
    for(int entry = 2; entry < numSectors; entry++){
        int value = getFatEntry(entry, fileptr);
        if(value == 0x00){
            numFreeSectors++;
        }
    }
    return (numFreeSectors*SECTOR_SIZE);
}

int getNumberOfFiles(char* fileptr, int physicalSector) {
	char* dir = &fileptr[SECTOR_SIZE * physicalSector];
	int count = 0;

	while(dir[0] != 0x00) {	
        if((dir[11] & 0x10) && dir[0] != 0x2e){
            int logicalCluster = dir[26] + (dir[27] << 8);
            int inSub = getNumberOfFiles(fileptr, logicalCluster+31);
            count += inSub;
		}
        else if (!(dir[11] & 0x08) && !(dir[11] & 0x10)) {
			count++;
		}
        dir+=32;
	}
	return count;
}


int getNumberOfCopies(char* fileptr){
    return fileptr[16];
}

int getNumberOfSectorsPerFAT(char* fileptr){
    return fileptr[22] + (fileptr[23] << 8);
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
    char* fileptr = mmap(NULL, buffer.st_size, PROT_READ, MAP_SHARED, file, 0);
    if(fileptr == MAP_FAILED){
        printf("Error: mmap() call failed\n");
        close(file);
        return FAILED_EXIT;
    }

    // retrieve info
    char OSName[STR_BUFFER_SIZE];
    getOSName(OSName, fileptr);
    char label[STR_BUFFER_SIZE];
    getLabel(label, fileptr);
    int diskSize = getDiskSize(fileptr);
    int freeSize = getFreeSize(diskSize, fileptr);
    int numFiles = getNumberOfFiles(fileptr, 19);
    int numCopies = getNumberOfCopies(fileptr);
    int numSectorsPerFAT = getNumberOfSectorsPerFAT(fileptr);

    // print info
    printf("OS Name: %s\n", OSName);
    printf("Label: %s\n", label);
    printf("Disk size: %d [bytes]\n", diskSize);
    printf("Free disk size: %d [bytes]\n", freeSize);
    printf("Number of Files: %d \n", numFiles);
    printf("Number of FAT copies: %d\n", numCopies);
    printf("Sectors per FAT: %d\n", numSectorsPerFAT);

    // clean
    munmap(fileptr, buffer.st_size);
    close(file);
 
    return 0;
}