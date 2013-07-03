#pragma once

// #define ASM_REGISTER(decl, reg, args...) decl args
#define ASM_REGISTER(decl, reg, args...) register decl asm(#reg) args
#define ASM_LINE(command) "\t" command "\n"
#define ASM_BLOCK(args...) asm VOLATILE(args)

#define ASM_PTR_ADD(ptr, offset)                                 \
    ASM_BLOCK(ASM_LINE("lea (%0, %1, %c2), %0") :                \
              /* writes %0 */ "+r" (ptr) :                       \
              /* reads %1 */ "r" (offset),                       \
              /* reads %2 */ "i" (sizeof(*ptr)));
