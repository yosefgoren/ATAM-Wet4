#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include "elf64.h"
#include <fcntl.h>
#include <string.h>

//#define MY_DEBUG
#ifdef MY_DEBUG
#define DB(s) s
#else
#define DB(s)
#endif

#define true    1
#define false   0
#define LOCAL   0
#define GLOBAL  1
#define NOTYPE  0
#define FUNC    2

typedef unsigned char bool;
typedef unsigned long long MemAdrr;

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

int ropen(char* fname){
    int fd = open(fname, O_RDONLY);
    if(fd == -1)
        perror("opne");
    return fd;
}

typedef enum {FOUND_GLOBAL, FOUND_LOCAL, NOT_FOUND}  funcBindResult;

char *mallocStringFromTable(int elf_fd, Elf64_Off table_off, Elf64_Word name_indx) {
    int size = 0;
    char curr;
    pread(elf_fd, &curr, sizeof(char), name_indx+table_off+size);
    while (curr != 0) {
        ++size;
        pread(elf_fd, &curr, sizeof(char), name_indx+table_off+size);
    }
    ++size;

    char *res = malloc(size);
    pread(elf_fd, res, size, table_off+name_indx);

    return res;
}

funcBindResult addrOfFunctionNamed(char* func_name, char* elf_name, Elf64_Addr* addr){
    Elf64_Ehdr elf_header;
    int fd = ropen(elf_name);                       //add do_sys
    pread(fd, &elf_header, sizeof(Elf64_Ehdr), 0);  //add do_sys

    Elf64_Shdr shdr_shstrtab;
    pread(fd, &shdr_shstrtab, sizeof(Elf64_Shdr), elf_header.e_shoff + (elf_header.e_shentsize * elf_header.e_shstrndx));

    Elf64_Shdr shdr_symtab;
    Elf64_Shdr shdr_strtab;
    Elf64_Off elf_shoff = elf_header.e_shoff;
    bool foundSym = false;
    for(uint64_t i = 0; i < elf_header.e_shnum; ++i) {
        Elf64_Word sh_name_indx;        
        pread(fd, &sh_name_indx, sizeof(Elf64_Word), elf_shoff);
        char *sh_name = mallocStringFromTable(fd, shdr_shstrtab.sh_offset, sh_name_indx);
        if (strcmp(sh_name, ".symtab") == 0) { 
            pread(fd, &shdr_symtab, sizeof(Elf64_Shdr), elf_shoff); //add do_sys
            foundSym = true;
        }
        else if (strcmp(sh_name, ".strtab") == 0) {
            pread(fd, &shdr_strtab, sizeof(Elf64_Shdr), elf_shoff); //add do_sys
        }
        elf_shoff += elf_header.e_shentsize;
        free(sh_name);
    }

    if (!foundSym) {
        return NOT_FOUND;
    }

    Elf64_Off symtab_offset = shdr_symtab.sh_offset;
    uint64_t sym_num = shdr_symtab.sh_size/shdr_symtab.sh_entsize; //assert(sym_size == sym_entsize*sym_num)
    for(uint64_t i = 0; i < sym_num; ++i) {
        Elf64_Word sym_name_indx;
        pread(fd, &sym_name_indx, sizeof(Elf64_Word), symtab_offset);
        char *sym_name = mallocStringFromTable(fd, shdr_strtab.sh_offset, sym_name_indx); //idk about this seems costly
        if (strcmp(sym_name, func_name) == 0) {
            Elf64_Sym sym;
            pread(fd, &sym, sizeof(Elf64_Sym), symtab_offset); //add do_sys
            unsigned char info_copy1 = sym.st_info;
            unsigned char bind = ELF64_ST_BIND(info_copy1);
            unsigned char info_copy2 = sym.st_info;
            unsigned char type = ELF64_ST_TYPE(info_copy2);
            if ((bind == GLOBAL) && ((type == NOTYPE) || (type == FUNC))) {//notype needed?
                *addr = sym.st_value;
                return FOUND_GLOBAL;
            }
            else if ((bind == LOCAL) && ((type == NOTYPE) || (type == FUNC))) {
                //OLD: *addr = `NULL;
                return FOUND_LOCAL;
            }
        }
        symtab_offset += shdr_symtab.sh_entsize;
        free(sym_name);
    }

    //*addr = NULL;
    return NOT_FOUND;
}

int isNextInstrSyscall(int spid){
    struct user_regs_struct regs;
    ptrace(PTRACE_GETREGS, spid, NULL ,&regs);
    long instr = ptrace(PTRACE_PEEKTEXT, spid, regs.rip, NULL);
    return (instr<<48>>48) == 0x050f;
}

int isSyscallErrorRetvalue(int spid, unsigned long long* ret_val){
    struct user_regs_struct regs;
    ptrace(PTRACE_GETREGS, spid, NULL ,&regs);
    if(ret_val != NULL)
        *ret_val = regs.rax;
    return regs.rax < 0;
}

void printTraceeRegs(int spid){
    DB(printf("printRegs:\n"));
    struct user_regs_struct regs;
    ptrace(PTRACE_GETREGS, spid, 0, &regs);
    DB(printf("     rip = %llx\n", regs.rip));
    DB(printf("     rax = %llx\n", regs.rax));
}

long getTraceeStackTop(int spid){
    struct user_regs_struct regs;
    ptrace(PTRACE_GETREGS, spid, 0, &regs);
    return ptrace(PTRACE_PEEKDATA, spid, regs.rsp, NULL);
}

void printStackTop(int spid){
    printf("printStackTop:\n");
    long stack_top = getTraceeStackTop(spid);
    printf("    value in top of stack is: %lx\n", stack_top);
}

long addBreakpoint(int spid, Elf64_Addr break_addr){
    DB(printf("adding breakpoint at: %lx.\n", (long)break_addr));
    long original_text = ptrace(PTRACE_PEEKTEXT, spid, (void*)break_addr, NULL);
    unsigned long trapped_text = (0xffffffffffffff00 & original_text) | 0xcc;
    DB(printf("     original text: %lx, new text: %lx.\n", original_text, trapped_text));
    ptrace(PTRACE_POKETEXT, spid, (void*)break_addr, (void*)trapped_text);
    return original_text;
}

void removeBreakpointAndFixRip(int spid, Elf64_Addr break_addr, long original_text){
    struct user_regs_struct regs;
    ptrace(PTRACE_GETREGS, spid, 0, &regs);
    --regs.rip;
    ptrace(PTRACE_SETREGS, spid, 0, &regs);
    
    DB(printf("removing breakpoint at: %lx, original text: %lx.\n", (long int)break_addr, original_text));
    ptrace(PTRACE_POKETEXT, spid, (void*)break_addr, (void*)original_text);

}

/**
 * returns 0 if the tracee stopped and returns 1 if it finished.
 */
int advanceBreakpoint(int spid, Elf64_Addr break_addr, long original_text){
    ptrace(PTRACE_CONT, spid, 0, 0);
    
    int wait_status;
    if(wait(&wait_status) == -1 || !WIFSTOPPED(wait_status)){
        return 1;
    }

    removeBreakpointAndFixRip(spid, break_addr, original_text);
    return 0;
}

void runFirstArrival(int spid, Elf64_Addr break_addr, void (*to_do)(int)){
    int wait_status;
    wait(&wait_status);
    if(!WIFSTOPPED(wait_status)){
        DB(printf("runFirstArrival: the son program did not stop at start.\n"));
        return;
    }

    long original_text = addBreakpoint(spid, break_addr);
    if(advanceBreakpoint(spid, break_addr, original_text) == 1){
        DB(printf("runFirstArrival: program did not get to specified address.\n"));
        return;
    }
    to_do(spid);

    ptrace(PTRACE_CONT, spid, NULL, NULL);    
    wait(&wait_status);
    
    DB(printf("runFirstArrival: finished.\n"));
}

void runEachArrival(int spid, Elf64_Addr break_addr, void (*to_do)(int)){
    int wait_status;
    
    wait(&wait_status);
    if(!WIFSTOPPED(wait_status))
        return;
    while(1){
        DB(printf("\nrunEachArrival: stating main loop.\n"));
        long initial_text_at_func = addBreakpoint(spid, break_addr);
        int ret_val = advanceBreakpoint(spid, break_addr, initial_text_at_func);
        //DB(printf("ret_val = %d.\n", ret_val));
        if(ret_val == 1){
            DB(printf("runEachArrival: finished reaching all arrivals to specified addr.\n"));
            return;
        }
        to_do(spid);
        ptrace(PTRACE_SINGLESTEP, spid, NULL, NULL);
        wait(&wait_status);
    }
}

enum WhatToDo{
    CHECK_SYSCALL_RESULT,
    IGNORE_NEXT_BREAKPOINT,
    FINISH_HANDLING_FUNCTION
};
#define WAIT_AND_CHECK_RES(caller_name)\
    if(wait(NULL) == -1){ \
        DB(printf(caller_name));\
        DB(printf(": tracee eneded unexpectedly.\n"));\
        return;\
    }

enum WhatToDo howToHandleTraceeStopped(int spid, MemAdrr orig_rsp){
    struct user_regs_struct regs;
    ptrace(PTRACE_GETREGS, spid, NULL, &regs);
    MemAdrr curr_rsp = regs.rsp;
    if(curr_rsp > orig_rsp)
        return FINISH_HANDLING_FUNCTION;
    
    long last_text = ptrace(PTRACE_PEEKTEXT, regs.rip-1, NULL);
    if((last_text & 0x00000000000000ff) == 0xcc)
        return IGNORE_NEXT_BREAKPOINT;
    return CHECK_SYSCALL_RESULT;
}

void checkSyscallResult(int spid){
    struct user_regs_struct regs;
    ptrace(PTRACE_GETREGS, spid, NULL, &regs);
    MemAdrr addr_of_syscall = regs.rip;

    ptrace(PTRACE_SYSCALL, spid, NULL, NULL);
    WAIT_AND_CHECK_RES("checkSyscallResult");

    unsigned long long retval;
    if(isSyscallErrorRetvalue(spid, &retval)){
        printf("PRF:: syscall in %llx returned with %lld\n", addr_of_syscall, retval);
    }
}

void ignoreNextBreakpoint(int spid, MemAdrr breakpoint_addr, long orig_breakpoint_text){
    removeBreakpointAndFixRip(spid, breakpoint_addr, orig_breakpoint_text);
    if(isNextInstrSyscall(spid))
        checkSyscallResult(spid);
    else {
        ptrace(PTRACE_SINGLESTEP, spid, NULL);
        WAIT_AND_CHECK_RES("ignoreNextBreakpoint");
    }
    addBreakpoint(spid, breakpoint_addr);
}

void handleSyscallFailsInSubtree(int spid){
    struct user_regs_struct regs;
    ptrace(PTRACE_GETREGS, spid, 0, &regs);
    MemAdrr orig_rsp = regs.rsp;
    MemAdrr ret_addr = ptrace(PTRACE_PEEKDATA, spid, orig_rsp, NULL);

    long orig_breakpoint_text = addBreakpoint(spid, ret_addr);
    
    ptrace(PTRACE_SYSCALL, spid, NULL, NULL);
    WAIT_AND_CHECK_RES("handleSyscallFailsInSubtree");
    
    while(1){
        enum WhatToDo to_do = howToHandleTraceeStopped(spid, orig_rsp);
        switch (to_do){
        case FINISH_HANDLING_FUNCTION:
            removeBreakpointAndFixRip(spid, ret_addr, orig_breakpoint_text);
            return;
            break;
        case CHECK_SYSCALL_RESULT:
            checkSyscallResult(spid);
            break;
        case IGNORE_NEXT_BREAKPOINT:
            ignoreNextBreakpoint(spid, ret_addr, orig_breakpoint_text);
            break;
        }
        ptrace(PTRACE_SYSCALL, spid, NULL, NULL);
        WAIT_AND_CHECK_RES("handleSyscallFailsInSubtree");    
    }

    // enum WhatToDo to_do = howToHandleTraceeStopped(spid, orig_rsp);
    // while(to_do != FINISH_HANDLING_FUNCTION){
    //     if(to_do == CHECK_SYSCALL_RESULT){
    //         checkSyscallResult(spid);
    //     } else {
    //         ignoreNextBreakpoint(spid, ret_addr, orig_breakpoint_text);
    //     }
    //     ptrace(PTRACE_SYSCALL, spid, NULL, NULL);
    //     if(wait(&wait_status) == -1){
    //         DB(printf("handleSyscallFailsInSubtree: tracee eneded unexpectedly.\n"));
    //         return;
    //     }
    //     to_do = howToHandleTraceeStopped(spid, orig_rsp);
    // }
}
    
void runDebugger(int spid, Elf64_Addr fun_addr){
    //runEachArrival(spid, fun_addr, handleSyscallFailsInSubtree);
    runEachArrival(spid, fun_addr, handleSyscallFailsInSubtree);
}

int main(int argc, char** argv){
    if(argc < 3){
        printf("expecting two parameters!\n");
        exit(1);
    }

    char* func_name = argv[1];
    char* exefile_name = argv[2];

    Elf64_Addr func_addr;
    funcBindResult res = addrOfFunctionNamed(func_name, exefile_name, &func_addr);
    switch (res)
    {
    case FOUND_GLOBAL:
        //DB(printf("%s func_addr: %lx\n",func_name ,func_addr));
        break;
    case FOUND_LOCAL:
        printf("PRF:: local found!\n");
        return 0;
    case NOT_FOUND:
        printf("PRF:: not found!\n");
        return 0;
    }

    int spid = fork();
    if(spid == 0){
        //DB(printf("son with pid: %d, about to run program: %s.\n", getpid(), exefile_name));
        if(ptrace(PTRACE_TRACEME, 0, NULL, NULL) < 0){
            perror("ptrace");
            exit(1);
        } else {
            execv(exefile_name, argv+2);
        }
    }

    runDebugger(spid, func_addr);

    return 0;
}