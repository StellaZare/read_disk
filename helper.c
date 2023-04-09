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
#define MAX_FAT_ENTRY 4093
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

int getDiskSize(char* diskptr){
    int numSectors = diskptr[19] + (diskptr[20] << 8);
    return (numSectors*SECTOR_SIZE);
}

int getFatEntry(int entry, char* diskptr){

    int b1;
    int b2;
    int result;

	if ((entry % 2) == 0) {
		b1 = diskptr[SECTOR_SIZE + ((3*entry) / 2) + 1] & 0x0F;
		b2 = diskptr[SECTOR_SIZE + ((3*entry) / 2)] & 0xFF;
		result = (b1 << 8) + b2;
	} else {
		b1 = diskptr[SECTOR_SIZE + (int)((3*entry) / 2)] & 0xF0;
		b2 = diskptr[SECTOR_SIZE + (int)((3*entry) / 2) + 1] & 0xFF;
        result = (b1 >> 4) + (b2 << 4);
	}
    return result;
}

void setFatEntry(int value, int entry, char* diskptr){
    char* Fat1 = diskptr + SECTOR_SIZE ;
    //char* Fat2 = diskptr + (SECTOR_SIZE * 10);
    if ((entry % 2) == 0) {
		Fat1[((3*entry) / 2) + 1] = (((value & 0xF00) >> 8) + (uint8_t)(Fat1[((3*entry) / 2) + 1] & 0xF0));
		Fat1[((3*entry) / 2)] = value & 0xFF;
        //printf("entry: %d b1: %02x b2: %02x\n", entry, (uint8_t)Fat1[((3*entry) / 2)], (uint8_t)Fat1[((3*entry) / 2) + 1]);
	} else {
		Fat1[(int)((3*entry) / 2)] = ((value & 0xF) << 4) + (uint8_t)(Fat1[(int)((3*entry) / 2)] & 0x0F);
		Fat1[(int)((3*entry) / 2) + 1] = (value & 0xFF0) >> 4;
        //printf("entry: %d b1: %02x b2: %02x\n", entry, (uint8_t)Fat1[(int)((3*entry) / 2)], (uint8_t)Fat1[(int)((3*entry) / 2) + 1]);
    }  
}

void extractDirectoryEntry(dirEntry_t* entryPtr, char* dirPtr){
    int b;
    for(b = 0; b < 8 && dirPtr[b] != 0x20; b++){
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
        entryPtr->logicalSector = dirPtr[26] + (dirPtr[27] << 8);
        entryPtr->size = (dirPtr[28] & 0xFF) + ((dirPtr[29] & 0xFF) << 8) + ((dirPtr[30] & 0xFF) << 16) + ((dirPtr[31] & 0xFF) << 24);
    }

}

int getFreeSize(int diskSize, char* diskptr){
    int numFreeSectors = 0;
    int numSectors = diskptr[19] + (diskptr[20] << 8);

    // traverse FAT table given one entry per cluster
    for(int entry = 2; entry < numSectors; entry++){
        int value = getFatEntry(entry, diskptr);
        if(value == 0x00){
            numFreeSectors++;
        }
    }
    return (numFreeSectors*SECTOR_SIZE);
}