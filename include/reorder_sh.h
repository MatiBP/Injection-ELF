#ifndef REORDER_SH_H
#define REORDER_SH_H

#include <elf.h>

typedef struct {
    int index_overwrite_sh;
    size_t size_section_headers;
    int direction;
} SortParameters;

//Réorganise les en-têtes de section ELF en fonction de l'index de section donné
int reorder_sh(Elf64_Ehdr* elf_headers, int index_overwrite_sh);

//Trie les en-têtes de section ELF en fonction de l'index de section donné et de la direction de tri
void sort_with_index(SortParameters sort_parameters, Elf64_Ehdr* elf_headers, Elf64_Shdr* section_headers);

#endif
