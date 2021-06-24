#define _XOPEN_SOURCE 500
#include "elf64.h"
#include "string.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

static const int BUFFSIZE = 1000;

void printBytes(int fd, int num_bytes, int offset){
    char buff[num_bytes];
    pread(fd, (void*)buff, num_bytes, offset);
    
    for(int i = num_bytes-1; i >= 0; --i)
        printf("%x: %x\n", i, *(buff+i));
    printf("\n");
}

int ropen(char* fname){
    int fd = open(fname, O_RDONLY);
    if(fd == -1)
        perror("opne");
    return fd;
}

void dumpSection(Elf64_Shdr sec_header){

    printf("section name: %x\n", sec_header.sh_name);
}

int main(int argc, char** argv){
    if(argc == 1){
        printf("no file specified!\n");
        return 1;
    }

    Elf64_Ehdr elf_header;
    int fd = ropen(argv[1]);
    pread(fd, &elf_header, sizeof(Elf64_Ehdr), 0);

    long int strtab_hdr_offset = elf_header.e_phoff + sizeof(Elf64_Shdr)*3;
    Elf64_Shdr strtab_header;
    pread(fd, &strtab_header, sizeof(Elf64_Shdr), strtab_hdr_offset);
    printf("offset of strtab header is: %lx\n", strtab_hdr_offset);
    printf("offset of strtab is: %lx\n", strtab_header.sh_addr);

    char strtab[BUFFSIZE];
    pread(fd, strtab, 56, strtab_header.sh_addr);
    //printf("in strtab: %x")
    
    //printf("section detail: %d\n", str_header);

    return 0;
}