# Information

## Directory overview
This assignment uses 4 main source files:
* diskinfo.c
* disklist.c
* diskget.c
as well as 2 helper files
* helper.c
* helper.h
\
The disks folder has 2 example disk images
* disk.IMA (default)
* diskSubDir (an example with sub-directories)

## To compile
To compile all 4 parts simply run 'make'
```
make
```

## To run
For part 1
```
./diskinfo <diskname.IMA>
```
\
For part 2
```
./disklist <diskname.IMA>
```
\
For part 3
```
./diskget <diskname.IMA> <filename.ext>
```
Part 4
not get complete

# Part 1: diskinfo

## Explanation
The input disk image is processed using the __mmap__ function and stored as an array in virtual memory.
\
\
The __OS name__ can be found the boot sector (start byte = 3, length = 8)
\
\
The __volume label__ can be found in the boot sector (start byte = 43, length = 11)
Alternatively the label is retieved by traversing the root directory (sector 19) to find a 32 bit entry with the attribute field set to 0x08 (sub-directory) and retieving the name of this entry (start byte = 0, length = 8)
\
\
The __total disk size__ can be calculated my taking the number of sectors on the disk found in the boot sector (start byte = 19, length = 2) and the bytes per sector (given 512 bytes)
\
\
The __free disk size__ can be calculated by traversing a copy of the FAT tables (sector 1) starting at entry 2 to count empty entries (entry = 0x00). Free disk space in bytes can then be calculated using the expression: number of 0x00 entries * bytes per sector (given 512 bytes)
\
\
To count the __number of files__ on the disk a recursive approach is ideal. The function getNumberofFiles() takes as input a pointer to the first byte of the disk and a physical sector number. Sets char* dir to point to the first byte of the specified physical sector. Then traverses the 32 bit entries to count the number of files. If a directory entry has attribute byte set to 0x08 (sub-directory), the function with recurse passing in the file pointer and logical cluster number of the entry+31.
\
\
The number of __FAT table copies__ on the disk can be found in the boot sector (byte 16)
\
\
The number of __sector per FAT table__ can be found in the boot sector (start byte = 22, length = 2)

## Error handling
* Error: please include a disk image 
* Error: unable to open disk image
* Error: fstat() call failed
* Error: mmap() call failed

# Part 2: disklist

## Explanation

## Error handling

# Part 3: diskget

## Explanation

## Error handling


# Part 4: diskput

not yet complete

