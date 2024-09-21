#ifndef HIJACKING_GOT_H
#define HIJACKING_GOT_H

#include <bfd.h>
#include <elf.h>

int hijacking_GOT(Elf64_Ehdr* elf_headers, bfd_vma base_address, const char* symbol_name);
int find_got_index(uint64_t num_relocations, Elf64_Rela* relocations, char* dynstrtab, Elf64_Sym* symbols, const char* symbol_name);

#endif
