#ifndef CODE_INJECTION_H
#define CODE_INJECTION_H

#include <stdint.h>

#define PAGE_SIZE 4096

typedef struct {
    int elf_file_descriptor;
    uint64_t file_size;
    uint64_t inject_size;
    int inject_file_descriptor;
} codeInjectionParameters;


uint64_t append_inject_code(codeInjectionParameters injection_parameters);

#endif
