#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <elf.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include "../include/overwrite_sh.h"

int overwrite_sh(uint64_t injected_code_offset, uint64_t file_size, Elf64_Ehdr* elf_headers, bfd_vma base_address) {

    int index_overwrite_sh = -1;

    // Obtenir l'index de la section header décrivant la section .shstrtab
    size_t shstrndx = elf_headers->e_shstrndx;

    // Obtenir les section headers
    Elf64_Shdr* section_headers = (Elf64_Shdr*)((uint8_t*)elf_headers + elf_headers->e_shoff);

    // Obtenir la table des chaînes
    char* shstrtab = (char*)((uint8_t*)elf_headers + section_headers[shstrndx].sh_offset);

    // Parcourir toutes les section headers
    for (int i = 0; i < elf_headers->e_shnum; ++i) {
        Elf64_Shdr* section_header = &section_headers[i];

        // Obtenir le nom de la section courante
        size_t name_index = section_header->sh_name;

        if (name_index < section_headers[shstrndx].sh_size) {
            char* name = shstrtab + name_index;
            if (strcmp(name, ".note.ABI-tag") == 0) {
                // Modifier les champs de la section header
                section_header->sh_type = SHT_PROGBITS;
                section_header->sh_addr = base_address;
                section_header->sh_offset = injected_code_offset;
                section_header->sh_size = file_size - injected_code_offset;
                section_header->sh_addralign = ALIGNMENT_SIZE;
                section_header->sh_flags |= SHF_EXECINSTR;
                index_overwrite_sh = i;
                break;
            }
        }
    }


    return index_overwrite_sh;
}
