#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ptrace.h>

struct user_regs_struct {
    unsigned long long int r15;
    unsigned long long int r14;
    unsigned long long int r13;
    unsigned long long int r12;
    unsigned long long int rbp;
    unsigned long long int rbx;
    unsigned long long int r11;
    unsigned long long int r10;
    unsigned long long int r9;
    unsigned long long int r8;
    unsigned long long int rax;
    unsigned long long int rcx;
    unsigned long long int rdx;
    unsigned long long int rsi;
    unsigned long long int rdi;
    unsigned long long int orig_rax;
    unsigned long long int rip;
    unsigned long long int cs;
    unsigned long long int eflags;
    unsigned long long int rsp;
    unsigned long long int ss;
    unsigned long long int fs_base;
    unsigned long long int gs_base;
    unsigned long long int ds;
    unsigned long long int es;
    unsigned long long int fs;
    unsigned long long int gs;
};

int spid;

void* addrOfFunctionNamed(char* func_name, char* elf_name){
    
}

int isInstrSyscall(void** instr_addr){
    struct user_regs_struct regs;
    ptrace(PTRACE_GETREGS, spid, NULL ,&regs);
    long instr = ptrace(PTRACE_PEEKTEXT, spid, regs.rip, NULL);
    *instr_addr = (void*)regs.rip;
    return (instr<<48>>48) == 0x050f;
    //printf("0x%lx\n", instr<<48>>48);
}

int isErrorRetvalue(int* ret_val){
    struct user_regs_struct regs;
    ptrace(PTRACE_GETREGS, spid, NULL ,&regs);
    *ret_val = regs.rax;
    return *ret_val < 0;
}

void runDebugger(){
    int wait_status;
    int is_instr_syscall = 0;
    void* instr_addr = NULL;

    wait(&wait_status);
    while(WIFSTOPPED(wait_status)){
        if(is_instr_syscall){
            int sys_ret_val;
            if(isErrorRetvalue(&sys_ret_val))
                printf("PRF:: syscall in %llx returned with %d\n",
                    instr_addr, sys_ret_val);
        }
        
        is_instr_syscall = isInstrSyscall(&instr_addr);
        
        if(ptrace(PTRACE_SINGLESTEP, spid, NULL, NULL) < 0){
            perror("ptrace");
            exit(1);
        }

        wait(&wait_status);
    }
}

int main(int argc, char** argv){
    if(argc < 2){
        printf("expecting a parameter to run.\n");
        exit(1);
    }

    spid = fork();
    if(spid == 0){
        printf("son with pid: %d, about to run program: %s.\n", getpid(), argv[1]);
        if(ptrace(PTRACE_TRACEME, 0, NULL, NULL) < 0){
            perror("ptrace");
            exit(1);
        } else {
            execv(argv[1], argv+1);
        }
    }

    runDebugger(spid);

    return 0;
}