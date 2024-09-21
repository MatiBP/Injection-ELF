#ifndef SET_NAME_SH_H
#define SET_NAME_SH_H

#include <elf.h>

#define size_noteABItag 13

int set_name_sh(Elf64_Ehdr* elf_headers, char* section_name);
void my_strcpy(const char* src, char* dest, size_t dest_size);

#endif