#include <stdio.h>
#include <stdlib.h>
#include <argp.h>
#include <fcntl.h>
#include <unistd.h>
#include <elf.h>
#include <sys/mman.h>

#include "../include/isos_inject.h"
#include "../include/find_pt_note.h"
#include "../include/code_injection.h"
#include "../include/overwrite_sh.h"
#include "../include/reorder_sh.h"
#include "../include/set_name_sh.h"
#include "../include/overwrite_pt_note.h"
#include "../include/hijacking_GOT.h"


static const char args_doc[] = "-f ELF_FILE -b inject/INJECT_FILE -s SECTION_NAME -a BASE_ADDRESS -e MODIFY_ENTRY";

static const char doc[] = "We want to inject machine code into an ELF binary.";

static struct argp_option options[] = {
    {"help", 'h', 0, 0, "Show help.", 0},
    {"file", 'f', "FILE", 0, "ELF file that will be analyzed", 0},
    {"binary", 'b', "BINARY_FILE", 0, "Binary file that we will inject code into", 0},
    {"section", 's', "SECTION", 0, "Injected section name", 0},
    {"address", 'a', "ADDRESS", 0, "Base address of the injected code", 0},
    {"entry", 'e', "ENTRY", 0, "Whether the entry point should be changed", 0},
    {0}
};

static error_t parse_opt(int key, char* arg, struct argp_state* state) {

    struct arguments* arguments = state->input;

    switch (key) {
    case 'h':
        argp_usage(state);
        exit(0);
    case 'f':
        if (arg != NULL) {
            arguments->elf_file = arg;
        }
        else {
            argp_error(state, "Option '--file' requires a non-empty argument.");
            return ARGP_ERR_UNKNOWN;
        }
        break;
    case 'b':
        if (arg != NULL) {
            arguments->inject_file = arg;
        }
        else {
            argp_error(state, "Option '--binary' requires a non-empty argument.");
            return ARGP_ERR_UNKNOWN;
        }
        break;
    case 's':
        if (arg != NULL) {
            arguments->section_name = arg;
        }
        else {
            argp_error(state, "Option '--section' requires a non-empty argument.");
            return ARGP_ERR_UNKNOWN;
        }
        break;
    case 'a':
        if (arg != NULL) {
            arguments->base_address = strtoul(arg, NULL, BASE_ADDRESS_HEX_DIGITS);
        }
        else {
            argp_error(state, "Option '--address' requires a non-empty argument.");
            return ARGP_ERR_UNKNOWN;
        }
        break;
    case 'e':
        if (arg != NULL && arg[0] != '\0') {
            arguments->modify_entry = (arg[0] == '1');
        }
        else {
            argp_error(state, "Option '--entry' requires a non-empty argument.");
            return ARGP_ERR_UNKNOWN;
        }
        break;
    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}


static struct argp argp = { options, parse_opt, args_doc, doc, NULL, NULL, NULL };

int main(int argc, char* argv[]) {

    struct arguments arguments = { 0 };


    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    if (argc < MIN_ARGUMENTS) {
        fprintf(stderr, "Error: Not enough arguments provided. Use -h for help.\n");
        return 1;
    }

    printf("-f for ELF file: %s\n", arguments.elf_file);
    printf("-b for Inject file: %s\n", arguments.inject_file);
    printf("-s for Section name: %s\n", arguments.section_name);
    printf("-a for Base address: 0x%lx\n", arguments.base_address);
    printf("-e for Modify entry: %d\n", arguments.modify_entry);


    bfd_init();

    // Open the ELF file using libbfd
    bfd* elf_bfd = bfd_openr(arguments.elf_file, NULL);
    if (!elf_bfd) {
        fprintf(stderr, "Error: Failed to open ELF file '%s'.\n", arguments.elf_file);
        return 1;
    }

    // Check if the file is a valid object file
    if (!bfd_check_format(elf_bfd, bfd_object)) {
        fprintf(stderr, "Error: '%s' is not an object file.\n", arguments.elf_file);
        bfd_close(elf_bfd);
        return 1;
    }

    // Check if the file is an ELF file
    if (bfd_get_flavour(elf_bfd) != bfd_target_elf_flavour) {
        fprintf(stderr, "Error: '%s' is not an ELF file.\n", arguments.elf_file);
        bfd_close(elf_bfd);
        return 1;
    }

    // Check if the file is a 64-bit file
    if (bfd_get_arch(elf_bfd) != bfd_mach_x86_64) {
        fprintf(stderr, "Error: '%s' is not a 64-bit file.\n", arguments.elf_file);
        bfd_close(elf_bfd);
        return 1;
    }

    // Check if the file is an executable file
    if (!(bfd_get_file_flags(elf_bfd) & EXEC_P)) {
        fprintf(stderr, "Error: '%s' is not an executable file.\n", arguments.elf_file);
        bfd_close(elf_bfd);
        return 1;
    }

    bfd_close(elf_bfd);


    // On ouver notre file ELF
    int elf_file_descriptor = open(arguments.elf_file, O_RDWR);
    if (elf_file_descriptor == -1) {
        perror("open");
        return -1;
    }

    // taille du fichier ELF
    uint64_t file_size = lseek(elf_file_descriptor, 0, SEEK_END);
    lseek(elf_file_descriptor, 0, SEEK_SET);

    // on ouvre le inject file
    int inject = open(arguments.inject_file, O_RDWR);
    if (!inject) {
        perror("open inject");
        close(elf_file_descriptor);
        return 0;
    }

    // taille du fichier inject
    uint64_t inject_size = lseek(inject, 0, SEEK_END);
    lseek(inject, 0, SEEK_SET);

    // On mappe tout le fichier en mémoire
    Elf64_Ehdr* elf_headers = mmap(NULL, (file_size + inject_size), PROT_READ | PROT_WRITE, MAP_SHARED, elf_file_descriptor, 0);
    if (elf_headers == MAP_FAILED) {
        perror("mmap ELF");
        close(inject);
        close(elf_file_descriptor);
        exit(1);
    }

    if (elf_headers->e_ident[EI_MAG0] != ELFMAG0 ||
        elf_headers->e_ident[EI_MAG1] != ELFMAG1 ||
        elf_headers->e_ident[EI_MAG2] != ELFMAG2 ||
        elf_headers->e_ident[EI_MAG3] != ELFMAG3) {
        fprintf(stderr, "%s is note an ELF file\n", arguments.elf_file);
        munmap(elf_headers, file_size);
        close(elf_file_descriptor);
        return -1;
    }

    // we Find the PT_NOTE segment in the ELF file
    Elf64_Phdr* pt_note_hdr = NULL;
    if (find_pt_note(elf_headers, &pt_note_hdr) == (uint64_t)-1) {
        fprintf(stderr, "Error: Could not find PT_NOTE segment in %s\n", arguments.elf_file);
        return -1;
    }

    codeInjectionParameters injection_parameters = {
        .elf_file_descriptor = elf_file_descriptor,
        .file_size = file_size,
        .inject_size = inject_size,
        .inject_file_descriptor = inject
    };
    // append the code we want to inject and return the offset of it
    uint64_t injected_code_offset = append_inject_code(injection_parameters);
    if (injected_code_offset == 0) {
        fprintf(stderr, "Error: Failed to inject code into the ELF file.\n");
        return -1;
    }
    printf("voici injected_code_offset 0x%lx\n", injected_code_offset);

    //On ajoute padding pour que l'address soit congrue avec l'offset
    while (((arguments.base_address - injected_code_offset) % PAGE_SIZE) != 0) {

        arguments.base_address += ((arguments.base_address - injected_code_offset) % PAGE_SIZE);
    }
    printf("voici new base address 0x%lx\n", arguments.base_address);

    // On recalcul la size du file en ajoutant notre inject_size
    file_size = file_size + inject_size;

    // on overwritte .note.ABI-tag pour qu'i reflete notre nouvelle section injecté
    int index_overwrite_sh = overwrite_sh(injected_code_offset, file_size, elf_headers, arguments.base_address);
    if (index_overwrite_sh < 0) {
        fprintf(stderr, "Error: Failed to modify section header in the ELF file.\n");
        return -1;
    }

    // On trie les sections headers selon leurs address
    int reorder = reorder_sh(elf_headers, index_overwrite_sh);
    if (reorder != 0) {
        fprintf(stderr, "Error: Failed to reorder sections headers in the ELF file.\n");
        munmap(elf_headers, file_size);
        close(elf_file_descriptor);
        close(inject);
        return -1;
    }

    // on rename notre section header modifié
    int set_name = set_name_sh(elf_headers, arguments.section_name);
    if (set_name != 0) {
        fprintf(stderr, "Error: Failed to rename section header in the ELF file.\n");
        munmap(elf_headers, file_size);
        close(elf_file_descriptor);
        close(inject);
        return -1;
    }

    overwritePtNoteParameters overwritePtNote_parameters = {
        .injected_code_offset = injected_code_offset,
        .injected_code_size = inject_size
    };
    // on overwrite notre program header pt_note
    int pt_note = overwrite_pt_note(overwritePtNote_parameters, pt_note_hdr, arguments.base_address);
    if (pt_note != 0) {
        fprintf(stderr, "Error: Failed to overwrite program header .\n");
        munmap(elf_headers, file_size);
        close(elf_file_descriptor);
        close(inject);
        return -1;
    }


    // modification entry point
    if (arguments.modify_entry) {
        int modified_GOT_entry = hijacking_GOT(elf_headers, arguments.base_address, "fputc");

        if (modified_GOT_entry != 0) {
            fprintf(stderr, "Error: Failed to modify GOT entry.\n");
            munmap(elf_headers, file_size);
            close(elf_file_descriptor);
            close(inject);
            return -1;
        }
    }


    // On munmap et close le file_descriptor
    if (munmap(elf_headers, file_size) == -1) {
        perror("munmap");
        close(elf_file_descriptor);
        close(inject);
        return -1;
    }

    close(elf_file_descriptor);
    close(inject);

    return 0;
}


