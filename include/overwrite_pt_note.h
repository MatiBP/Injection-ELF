#ifndef OVERWRITE_PT_NOTE_H
#define OVERWRITE_PT_NOTE_H
#include <elf.h>
#include <bfd.h>

#define PT_NOTE_ALIGN 0x1000


typedef struct {
    uint64_t injected_code_offset;
    uint64_t injected_code_size;
} overwritePtNoteParameters;


int overwrite_pt_note(overwritePtNoteParameters overwritePtNote_parameters, Elf64_Phdr* pt_note_hdr, bfd_vma base_address);

#endif
