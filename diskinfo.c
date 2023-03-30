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
#define ENTRY_SIZE 32
#define SECTOR_SIZE 512
#define STR_BUFFER_SIZE 100

/* ---------- Helper functions ---------- */

int getHex(int entry, char* fileptr){
    int b1 = fileptr[SECTOR_SIZE + (int)(3*entry /2)];
    int b2 = fileptr[SECTOR_SIZE + (int)((3*entry /2) +1)];

    if(entry%2 == 0){
        return (b1 << 8) + (b2 & 0X0F);
    }
    else{
        return ((b1 & 0xF0) >> 4) + (b2 << 4);
    }
}

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
        int value = getHex(entry, fileptr);
        if(value == 0x00){
            numFreeSectors++;
        }
    }
    return (numFreeSectors*SECTOR_SIZE);
}

int getNumberOfFiles(char* p) {
	p = &p[SECTOR_SIZE * 19];
	int count = 0;

    int byte = 0;
	while (p[byte] != 0x00) {
		if (!(p[11] & 0x08) && !(p[11] & 0x10)) {
			count++;
		}
        else if(p[11] & 0x10){
			printf("Found a subdirectory\n");
		}
		p = &p[byte + 32];
	}

	return count;
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
    int diskSize = getDiskSize(fileptr);
    int freeSize = getFreeSize(diskSize, fileptr);
    int numFiles = getNumberOfFiles(fileptr);

    // print info
    printf("OS Name: \t%s\n", OSName);
    printf("Label: \t\t%s\n", label);
    printf("Disk size: \t%d [bytes]\n", diskSize);
    printf("Free size: \t%d [bytes]\n", freeSize);
    printf("Num Files: \t%d \n", numFiles);


    // clean
    munmap(fileptr, buffer.st_size);
    close(file);
 
    return 0;
}