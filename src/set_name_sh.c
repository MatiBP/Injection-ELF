#include <stdio.h>
#include <string.h>

#include "../include/set_name_sh.h"


//Function pour mettre notre nouveau nom à la section
int set_name_sh(Elf64_Ehdr* elf_headers, char* section_name) {
    // on verifie que le nouveau nom n'est pas plus long que l'ancien pour pas overwritte le prochain section name
    if (strlen(section_name) > size_noteABItag) {
        fprintf(stderr, "error: section name too long\n");
        return -1;
    }

    size_t shstrndx = elf_headers->e_shstrndx;

    Elf64_Shdr* section_headers = (Elf64_Shdr*)((uint8_t*)elf_headers + elf_headers->e_shoff);

    char* shstrtab = (char*)((uint8_t*)elf_headers + section_headers[shstrndx].sh_offset);

    // Parcourir toutes les section headers
    for (size_t i = 0; i < elf_headers->e_shnum; ++i) {
        Elf64_Shdr* section_header = &section_headers[i];

        // Obtenir le nom de la section courante
        size_t name_index = section_header->sh_name;
        if (name_index < section_headers[shstrndx].sh_size) {
            char* name = shstrtab + name_index;
            if (strcmp(name, ".note.ABI-tag") == 0) {
                // Modifier les champs de la section header avec notre propre strcpy car celui libc donne warning tidy
                my_strcpy(section_name, name, size_noteABItag);
                break;
            }
        }
    }

    return 0;
}


//Notre propre strcpy
void my_strcpy(const char* src, char* dest, size_t dest_size) {
    size_t index = 0;
    while (src[index] != '\0' && index < dest_size) {
        dest[index] = src[index];
        index++;
    }
    dest[index] = '\0';
}


