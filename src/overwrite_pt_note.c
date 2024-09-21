#include <stdio.h>
#include <elf.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include "../include/overwrite_pt_note.h"

int overwrite_pt_note(overwritePtNoteParameters overwritePtNote_parameters, Elf64_Phdr* pt_note_hdr, bfd_vma base_address) {

    // Vérifier que l'en-tête de programme est valide
    if (pt_note_hdr->p_type != PT_NOTE) {
        fprintf(stderr, "Error: The program header is not a PT_NOTE header.\n");
        return -1;
    }

    // Mettre à jour les champs de l'en-tête de programme
    pt_note_hdr->p_type = PT_LOAD;
    pt_note_hdr->p_offset = overwritePtNote_parameters.injected_code_offset;

    pt_note_hdr->p_vaddr = base_address;
    pt_note_hdr->p_paddr = base_address;

    pt_note_hdr->p_filesz = overwritePtNote_parameters.injected_code_size;
    pt_note_hdr->p_memsz = overwritePtNote_parameters.injected_code_size;

    pt_note_hdr->p_flags = PF_R | PF_X;
    pt_note_hdr->p_align = PT_NOTE_ALIGN;

    return 0;
}