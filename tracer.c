#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <elf64.h>
#include <fcntl.h>
#include <string.h>

#define true    1
#define false   0
#define LOCAL   0
#define GLOBAL  1
#define NOTYPE  0
#define FUNC    2

typedef unsigned char bool;

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

typedef enum {FOUND_GLOBAL, FOUND_LOCAL, NOT_FOUND}  funcBindResult;

funcBindResult addrOfFunctionNamed(char* func_name, char* elf_name, Elf64_Addr* addr){
    Elf64_Ehdr elf_header;
    int fd = ropen(elf_name);                       //add do_sys
    pread(fd, &elf_header, sizeof(Elf64_Ehdr), 0);  //add do_sys

    Elf64_Xword sym_size;
    Elf64_Xword sym_entsize;
    uint64_t sym_num;
    Elf64_Off sym_offset;
    Elf64_Off elf_shoff = elf_header.e_shoff;
    bool foundSym = false;
    for(uint64_t i = 0; i < elf_header.e_shnum; ++i) {
        Elf64_Word sh_name = elf_shoff; // unsigened 64 bit truncated to 32 is well defined
        if (strcmp((char*) sh_name, ".symtab") == 0) { 
                Elf64_Shdr shdr;
                pread(fd, &shdr, sizeof(Elf64_Shdr), elf_shoff); //add do_sys
                sym_size = shdr.sh_size;
                sym_entsize = shdr.sh_entsize;
                sym_offset = shdr.sh_offset;
                sym_num = sym_size/sym_entsize; //assert(sym_size == sym_entsize*sym_num)
                foundSym = true;
        }
        elf_shoff += elf_header.e_shentsize;
    }

    if (!foundSym) {
        return;
    }

    for(uint64_t i = 0; i < sym_num; ++i) {
        Elf64_Word sym_name = sym_offset;
        if (strcmp((char*)sym_name, func_name) == 0) {
            Elf64_Sym sym;
            pread(fd, &sym, sizeof(Elf64_Sym), sym_offset); //add do_sys
            unsigned char info_copy1 = sym.st_info;
            unsigned char bind = ELF64_ST_BIND(info_copy1);
            unsigned char info_copy2 = sym.st_info;
            unsigned char type = ELF64_ST_TYPE(info_copy2);
            if ((bind == GLOBAL) && ((type == NOTYPE) || (type == FUNC))) {//notype needed?
                *addr = sym.st_value;
                return FOUND_GLOBAL;
            }
            else if ((bind == LOCAL) && ((type == NOTYPE) || (type == FUNC))) {
                *addr = NULL;
                return FOUND_LOCAL;
            }
        }
        sym_offset += sym_entsize;
    }

    *addr = NULL;
    return NOT_FOUND;
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