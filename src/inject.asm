BITS 64

SECTION .text
    align 4096  ; Alignement sur une page (4096 octets)
global injected_code

injected_code:
    ; save context
    push rax
    push rcx
    push rdx
    push rsi
    push rdi
    push r11

    lea rsi, [rel message] ; on charge l'adresse relative de la chaîne de caractères (pas utiliser adresse absolue avec mov)
    mov rdx, len
    mov rdi, 1
    mov rax, 1
    syscall

    pop r11
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rax

    ; call original entry point
    ;mov rax, 0x4022e0
    ;jmp rax
    ret
  

SECTION .data
    message db "Je suis trop un hacker", 0xa
    len equ $ - message
