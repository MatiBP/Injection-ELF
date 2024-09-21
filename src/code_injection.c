#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <elf.h>
#include <sys/mman.h>
#include <unistd.h> 
#include <fcntl.h>
#include "../include/code_injection.h"

// On inject notre code à la fin du ELF
uint64_t append_inject_code(codeInjectionParameters injection_parameters) {

    if (injection_parameters.inject_size == 0) {
        fprintf(stderr, "Error: inject_size is zero, you inject nothing\n");
        return 0;
    }

    uint64_t injected_code_offset = injection_parameters.file_size;

    uint8_t* inject_data = malloc(injection_parameters.inject_size);
    if (!inject_data) {
        perror("Error malloc");
        return 0;
    }

    ssize_t bytes_read = read(injection_parameters.inject_file_descriptor, inject_data, injection_parameters.inject_size);
    if (bytes_read != (ssize_t)injection_parameters.inject_size) {
        perror("Error read");
        free(inject_data);
        return 0;
    }

    // Seek to the offset where to append the injected code
    if (lseek(injection_parameters.elf_file_descriptor, (off_t)injected_code_offset, SEEK_SET) == -1) {
        perror("lseek");
        free(inject_data);
        return 0;
    }

    // Write the injected code to the ELF file
    ssize_t bytes_written = write(injection_parameters.elf_file_descriptor, inject_data, injection_parameters.inject_size);
    if (bytes_written != (ssize_t)injection_parameters.inject_size) {
        perror("write");
        free(inject_data);
        return 0;
    }

    printf("Injected code appended successfully.\n");

    free(inject_data);

    return injected_code_offset;
}
