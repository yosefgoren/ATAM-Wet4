#define _XOPEN_SOURCE 500
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

void causeError(){
    printf("causeError:\n");
    
    __asm__( 
          "movq $1, %%rax;"
          "movq $1, %%rdi;"
          "movq $0, %%rsi;"
          "movq $1, %%rdx;"
          "syscall"
        :
        :
        :
    );
}

void bar(int x);

void foo(int x){
    causeError();
    if(x == 0)
        return;
    bar(x-1);
}

void bar(int x){
    causeError();
    foo(x);
}

int main(){
    bar(2);
    foo(0);
    bar(1);

    return 0;
}