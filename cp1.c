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
    // int spid = fork();
    // if(spid == 0){
    //     //DB(printf("son with pid: %d, about to run program: %s.\n", getpid(), exefile_name));
    //     if(ptrace(PTRACE_TRACEME, 0, NULL, NULL) < 0){
    //         perror("ptrace");
    //         exit(1);
    //     } else {
    //         execv(exefile_name, argv+2);
    //     }
    // }

    // runDebugger(spid, func_addr);

    return 0;
}
