#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include "../include/find_pt_note.h"

// On cherche l'index et l'offset d'un segment de type NOTE
uint64_t find_pt_note(Elf64_Ehdr* elf_headers, Elf64_Phdr** pt_note_hdr) {

    int pt_note_index = -1; // init -1 pour indiquer qu'on a rien trouvé

    // Si tout va bien jusqu'à présent, on recherche le segment PT_NOTE
    if (elf_headers != NULL && elf_headers->e_phnum > 0) {

        Elf64_Phdr* phdr = (Elf64_Phdr*)((uint8_t*)elf_headers + elf_headers->e_phoff); // crée un pointeur phdr vers le premier en-tête de programme
        for (int i = 0; i < elf_headers->e_phnum; i++, phdr++) {
            if (phdr->p_type == PT_NOTE) {
                *pt_note_hdr = phdr;
                pt_note_index = i;
                printf("PT_NOTE program header index %d and offset %ld\n", pt_note_index, phdr->p_offset);
                break;
            }
        }
    }

    return pt_note_index;
}
