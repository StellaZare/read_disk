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

void extractDirectoryEntry(dirEntry_t* entryPtr, char* dirPtr){
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