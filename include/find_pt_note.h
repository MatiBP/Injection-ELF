#ifndef FIND_PT_NOTE_H
#define FIND_PT_NOTE_H

#include <elf.h>

uint64_t find_pt_note(Elf64_Ehdr* elf_headers, Elf64_Phdr** pt_note_hdr);

#endif
