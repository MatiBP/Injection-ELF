#include <stdio.h>
#include <elf.h>
#include <sys/mman.h>
#include <string.h>
#include "../include/reorder_sh.h"

// On reorder les sections headers en fonction de leur address
int reorder_sh(Elf64_Ehdr* elf_headers, int index_overwrite_sh) {

    if (!elf_headers || index_overwrite_sh < 0) {
        fprintf(stderr, "Invalid parameters\n");
        return -1;
    }
    Elf64_Shdr* section_headers = (Elf64_Shdr*)((uint8_t*)elf_headers + elf_headers->e_shoff);

    // Vérifier que l'index de la section à réorganiser est valide
    if (index_overwrite_sh < 0 || index_overwrite_sh >= elf_headers->e_shnum) {
        fprintf(stderr, "Invalid section header index\n");
        return -1;
    }
    // Trouver la section à réorganiser
    Elf64_Shdr* overwrite_section = &section_headers[index_overwrite_sh];

    // Initialiser la direction de déplacement
    int direction = 0;

    // Comparer avec la section précédente
    if (index_overwrite_sh > 0) {
        Elf64_Shdr* prev_section = &section_headers[index_overwrite_sh - 1];
        if (overwrite_section->sh_addr < prev_section->sh_addr) {
            direction = -1;
        }
    }

    // Comparer avec la section suivante
    if (index_overwrite_sh < elf_headers->e_shnum - 1) {
        Elf64_Shdr* next_section = &section_headers[index_overwrite_sh + 1];
        if (overwrite_section->sh_addr > next_section->sh_addr) {
            direction = 1;
        }
    }

    SortParameters sort_parameters = {
        .index_overwrite_sh = index_overwrite_sh,
        .size_section_headers = elf_headers->e_shnum,
        .direction = direction
    };
    sort_with_index(sort_parameters, elf_headers, section_headers);

    return 0;
}

// Fonction de trie
void sort_with_index(SortParameters Sort_parameters, Elf64_Ehdr* elf_headers, Elf64_Shdr* section_headers) {

    Elf64_Shdr key = section_headers[Sort_parameters.index_overwrite_sh];
    int distance = 0;

    size_t shstrndx = elf_headers->e_shstrndx;
    char* shstrtab = (char*)((uint8_t*)elf_headers + section_headers[shstrndx].sh_offset);

    // Déplacez l'élément vers la droite ou vers la gauche jusqu'à ce qu'il soit à sa position correcte
    for (size_t i = Sort_parameters.index_overwrite_sh + Sort_parameters.direction; i < Sort_parameters.size_section_headers; i += Sort_parameters.direction) {
        Elf64_Shdr* section_header = &section_headers[i];

        // compare les address en fonction de la direction de trie
        if ((Sort_parameters.direction == 1 && section_header->sh_addr < key.sh_addr) ||
            (Sort_parameters.direction == -1 && section_header->sh_addr > key.sh_addr)) {


            Elf64_Shdr temp = section_headers[i];
            section_headers[i] = section_headers[i - Sort_parameters.direction];
            section_headers[i - Sort_parameters.direction] = temp;
            // met à jour e_shstrndx si la section a bougé
            if (strcmp((shstrtab + temp.sh_name), ".shstrtab") == 0) {
                elf_headers->e_shstrndx = elf_headers->e_shstrndx - Sort_parameters.direction;
            }

            distance++;
        }
        else {
            break;
        }
        key = section_headers[i];
    }
    // met à jour les link
    for (int i = 0; i < (int)Sort_parameters.size_section_headers; i++) {
        Elf64_Shdr* section_header = &section_headers[i];

        int link = (int)section_header->sh_link;

        if (Sort_parameters.direction == -1 && link > 0 && link < (Sort_parameters.index_overwrite_sh - distance)) {
            section_header->sh_link++;

        }

        if (Sort_parameters.direction == 1 && link > 0 && link < (Sort_parameters.index_overwrite_sh + distance)) {
            section_header->sh_link--;

        }
    }
}




