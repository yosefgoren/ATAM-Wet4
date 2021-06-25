#include <stdio.h>
#include <unistd.h>

void loopGetAscii(){
    char c[100];
    while(1){
        scanf("%s", c);
        char* tmp = c;
        printf("%s:\n", c);
        while(*tmp != '\0'){
            printf("    char: %c, dec: %d, hex: %x\n", *tmp, *tmp, *tmp);
            tmp++;
        }
    }
}

int main(int argc, char** argv){
    loopGetAscii();
    return 0;
}
