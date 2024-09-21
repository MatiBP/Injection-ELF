#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <dlfcn.h>
#include <string.h>
#include <sys/mman.h>
#include "../include/hijacking_GOT.h"


// Trouver l'index d'une relocalisation dans la table de relocalisation en fonction de l'index du symbole dans la table des symboles
int find_got_index(uint64_t num_relocations, Elf64_Rela* relocations, char* dynstrtab, Elf64_Sym* symbols, const char* symbol_name) {
    for (int i = 0; (uint64_t)i < num_relocations; ++i) {
        Elf64_Sym* symbol = &symbols[ELF64_R_SYM(relocations[i].r_info)];
        char* name_dynstrtab = dynstrtab + symbol->st_name;

        if (name_dynstrtab != NULL && strcmp(name_dynstrtab, symbol_name) == 0) {
            printf("We find : %s\n", name_dynstrtab);
            return i;
        }
    }
    return -1;
}


// Hijack La GOT
int hijacking_GOT(Elf64_Ehdr* elf_headers, bfd_vma base_address, const char* symbol_name) {
    //secion .dynsym
    Elf64_Sym* symbols = NULL;
    uint64_t num_symbols = 0;
    // section .dynstr
    char* dynstrtab = NULL;
    // section .rela.plt 
    Elf64_Rela* relocations = NULL;
    // section .got.plt
    Elf64_Addr* got_section = NULL;

    //Nous allons comme d'habitude chercher les sections grace à leur name dans .shstrtab
    size_t shstrndx = elf_headers->e_shstrndx;
    Elf64_Shdr* section_headers = (Elf64_Shdr*)((uint8_t*)elf_headers + elf_headers->e_shoff);
    char* shstrtab = (char*)((uint8_t*)elf_headers + section_headers[shstrndx].sh_offset);

    for (size_t i = 0; i < elf_headers->e_shnum; ++i) {
        Elf64_Shdr* section_header = &section_headers[i];
        size_t name_index = section_header->sh_name;
        if (name_index < section_headers[shstrndx].sh_size) {
            char* name = shstrtab + name_index;

            if (strcmp(name, ".dynsym") == 0) {
                symbols = (Elf64_Sym*)((uint8_t*)elf_headers + section_header->sh_offset);
                num_symbols = section_header->sh_size / sizeof(Elf64_Sym);

            }
            else if (strcmp(name, ".dynstr") == 0) {
                dynstrtab = (char*)((uint8_t*)elf_headers + section_header->sh_offset);

            }
            else if (strcmp(name, ".rela.plt") == 0) {
                relocations = (Elf64_Rela*)((uint8_t*)elf_headers + section_header->sh_offset);
            }
            else if (strcmp(name, ".got.plt") == 0) {
                got_section = (Elf64_Addr*)((uint8_t*)elf_headers + section_header->sh_offset);
            }
        }
    }

    if (symbols == NULL || dynstrtab == NULL || relocations == NULL || got_section == NULL) {
        fprintf(stderr, "Error: Failed to find necessary sections\n");
        return -1;
    }


    // Nous recuperons l'index de relocation de la fonction à hijack en fonction de son index dans .dynsym
    int got_index = find_got_index(num_symbols, relocations, dynstrtab, symbols, symbol_name);
    if (got_index == -1) {

        fprintf(stderr, "Error: Failed to find got_index\n");
        return -1;
    }


    // Dans .got.plt nous modifions l'address d'appel de la fonction à hijack par notre base_address
    Elf64_Addr* got_entry_modified = &got_section[got_index + 3]; // +3 car il y a un header dans .got.plt, cela ma malheureusement pris beaucoup de temps à trouver
    *got_entry_modified = (Elf64_Addr)base_address;


    printf("Successfully hijacked GOT entry for symbol %s\n", symbol_name);

    return 0;
}
