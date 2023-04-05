#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

int main(){
    int fls = 360;
    int b26 = 0;
    int b27 = 0;

    printf("fls: %08x b26: %08x b27: %08x\n", fls, b26, b27);
    b26 = (fls - (0 << 8)) & 0xff;
    printf("fls: %08x b26: %08x b27: %08x\n", fls, b26, b27);
    b27 = ((fls) - b26) >> 8;
    printf("fls: %08x b26: %08x b27: %08x\n", fls, b26, b27);

    printf("wtf %08x and wtf 2 %08x \n", fls << 8, fls - (0<<8));


    return 0;
}