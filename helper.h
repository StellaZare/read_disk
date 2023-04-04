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

int getFreeSize(int diskSize, char* fileptr);

int getDiskSize(char* fileptr);

int getFatEntry(int entry, char* fileptr);

void extractDirectoryEntry(dirEntry_t* entryPtr, char* dirPtr);