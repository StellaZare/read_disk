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
./diskput <diskname.IMA> <filename.ext>

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
The disk image is processed the same way as in part 1
\
\
The function getNumberOfFiles() from part 1 of the assignment is used again and modified to now print the directory contents as it traverses the directory hierarchy.
The new function is called traverseDirecotry() and takes as input a pointer to the name of the directory it is currently processes, a pointer to the first byte of the disk image and a physical sector number.
The function first traverses the current directory, extracts entries into struct dirEntry_t and stores the information in dirContents array. Then prints the contents of the current directory.
Then traverses on the sub-directories it found previously.
\
\
The function relies on two helper functions: 
* extractDirectoryEntry() (helper.c)
* printDirectoryContents()


## Error handling
* Error: please include a disk image 
* Error: unable to open disk image
* Error: fstat() call failed
* Error: mmap() call failed

# Part 3: diskget

## Explanation
The disk image is processed the same way as in part 1 however additional error handling is required
\
\
The file name given to the program is converted to upper case and stored in a char array. Then the fucntion getFileEntry() is called with input filename, pointer to the fist byte of the disk image and a pointer to dirEntry_t struct. The function is a modified version of the function traverseDirecotry() from part 2 which does not recurse, but instead extracts each 32 bit entry and check the name and extension against the input file name. If the input file exists in the root direcotry, the function returns 1 and updates the dirEntry_t struct in main. Otherwise the function returns -1 and the program ends with the message "Error: file <filename> not found in root directory".
\
\
The function copyFileEntry() is called to tranfer bytes from the original file to the new file. The function takes as input a pointer to the first byte of the original file, a pointer to the first memory location of the new file and a pointer to the dirEntry_t object in main. 
The function copies over the first 512 bytes of the file using the first logical sector field of the struct. Then for each subsequent sector the function queries the FAT tables using the helper function getFatEntry() by providing the current logical secotr number and a pointer to the first byet of the original file. The return value will be the next logical sector to check. This loop terminates once the call to getFatEntry() returns 0xFFF signaling the end of the file. 

## Error handling
* Error: program should be invoked as follows: ./diskget <diskfile> <filename>
* Error: unable to open disk image
* Error: fstat() call failed
* Error: mmap() call failed (twice)
* Error: file <filename> not found in root directory
* Error: could not open new file
* Error: ftruncate() call failed
* Error: unable to write to new file

# Part 4: diskput

## Explanation
This implementation is not complete and only accepts cases where the file is being added to the root directory of the disk image. (Going for part marks!)
\
\
The disk image and the new file are processes in a similar way to part 3 with mmap and the necessary permissions. First the input file size is compared with the available bytes on the disk using the stat struct and the function getFreeSize() from part 1.
\
\
If there is sufficient space on the disk the function copyFileToRootDir() is called which handles the bulk of the operations.
\
\
The function first get the entry # for the next free entry in the FAT table. This corresponds to the first logical cluster we can use to store the contents of the new file. The function then adds an entry in the root directory by calling the function addRootEntry() to set the file name, attribute flag, first logical cluster and the file size. The date created/modified did not seem like vital fields so they were left with default values.
\
\
Then the function copyFileToRootDir() begins to copy the contents of the file to the root directory byte-by-byte. Once a sector is full, the function find the next empty FAT entry; stores the value in the previously empty FAT entry; and continues to copy into the new logical sector. The function terminates when the file has been successfully copied over, and the last FAT entry is set with the value 0xFFF.

## Error handling 
* Error: program should be invoked as follows
* Error: unable to open disk image
* Error: fstat() call failed
* Error: mmap() call failed
* Error: not enough space on the disk
* Error: file already exists in root directory

# Useful commands
```
xxd -g <num> -s <num> -l <num> <disk image>
```
# Useful info
| Name     | Sector  | 1st Byte |
|----------|---------|----------|
| boot     | 0       | 0        |
| FAT1     | 1       | 512      |
| FAT1     | 10      | 5,120    |
| root     | 19      | 9,728    |
| data     | 33      | 16,896   |

# Creating a disk image
mkdir test
dd if=/dev/zero of=mydisk.IMA count=2880 bs=512
