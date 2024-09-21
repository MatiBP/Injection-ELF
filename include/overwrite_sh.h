#ifndef OVERWRITE_SH_H
#define OVERWRITE_SH_H

#include <bfd.h>
#include <elf.h>

#define ALIGNMENT_SIZE 16

int overwrite_sh(uint64_t injected_code_offset, uint64_t file_size, Elf64_Ehdr* elf_headers, bfd_vma base_address);

#endif 
