#ifndef ISOS_PROJECT_H
#define ISOS_PROJECT_H

#include <bfd.h>

#define BASE_ADDRESS_HEX_DIGITS 16
#define MIN_ARGUMENTS 10
#define ELF_ARCH_BITS 64

struct arguments {
    char* elf_file;
    char* inject_file;
    char* section_name;
    bfd_vma base_address; //or uintptr_t not sure, bfd_vma this is from the libbfd and this type represent virtual memorie address
    int modify_entry;
};


#endif 
