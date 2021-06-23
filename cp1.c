#include <stdio.h>

#define BUFF 100

void andBenEl(){
    printf("im also a part of the band!\n");
}

int main(int argc, char** argv){
    char c[BUFF];
    while(1){
        scanf("%s", c);
        char* tmp = c;
        printf("%s:\n", c);
        while(*tmp != '\0'){
            printf("    char: %c, dec: %d, hex: %x\n", *tmp, *tmp, *tmp);
            tmp++;
        }
    }
    return 0;
}
