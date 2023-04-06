#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

void setFatEntry(){
    int value[] = {0x097, 0x098, 0x099, 0x09a, 0x09b, 0xfff};
    int entry;

    int b1 = 0;
    int b2 = 0;
    int b3 = 0;

    printf("b1  b2  b3\n");
    int idx = 0;
    for(entry = 150; entry <= 155; entry++){
        if ((entry % 2) == 0) {
            b1 = value[idx] & 0xFF;
            b2 = (((value[idx] & 0xF00) >> 8) + (uint8_t)(b2 & 0xF0));
        } else {
            b2 = ((value[idx] & 0xF00) >> 4) + (uint8_t)(b2 & 0x0F);
            b3 = (value[idx] & 0xFF) & 0xFF;
        }  
        idx++;
        printf("%02x  %02x  %03x\n", b1, b2, b3);
    }
}


int main(){
    /*
    uint16_t fls = 0xF38F;
    int b26 = 0;
    int b27 = 0;

    printf("fls: %08x b26: %08x b27: %08x\n", fls, b26, b27);
    b26 = (uint32_t)(fls & 0x000000FF);
    printf("fls: %08x b26: %08x b27: %08x\n", fls, b26, b27);
    b27 = (fls & 0x0000FF00) >> 8;
    printf("fls: %08x b26: %08x b27: %08x\n", fls, b26, b27);

    printf("wtf %08x and wtf 2 %08x \n", fls << 8, fls - (0<<8));
    */

   setFatEntry();

    return 0;
}