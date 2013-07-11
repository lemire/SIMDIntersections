#pragma once


#define ASM_VOLATILE volatile

// #define ASM_REGISTER(decl, reg, args...) decl args
#define ASM_REGISTER(decl, reg, args...) register decl __asm(#reg) args
#define ASM_LINE(command) command "\t\n"
#define ASM_BLOCK(args...) __asm ASM_VOLATILE(args)

#define ASM_LEA_ADD_BASE(ptr, base)                              \
    ASM_BLOCK(ASM_LINE("lea (%0, %1, %c2), %0") :                \
              /* writes %0 */ "+r" (ptr) :                       \
              /* reads %1 */ "r" (base),                         \
              /* reads %2 */ "i" (sizeof(*ptr)));

#define ASM_LEA_ADD_BASE_MUL(ptr, base, mul)                     \
    ASM_BLOCK(ASM_LINE("lea (%0, %1, %c2), %0") \
              : /* writes */                                     \
              "+r" (ptr)                                   \
              : /* reads */                                      \
              "r" (base),                                 \
              "i" (mul));

#define ASM_LEA_ADD_BYTES(ptr, bytes)                            \
    ASM_BLOCK(ASM_LINE("lea %c1(%0), %0")                        \
              : /* writes */                                     \
              "+r" (ptr)                                         \
              : /* reads */                                      \
              "i" (bytes));


// NOTE: cmova/cmovb are one cycle slower than cmovg/cmovl
#define ASM_ADD_CMOV(var, add, cmp1, cmp2, cmov)   {                    \
    typeof(var) _new = var + add;                                       \
    ASM_BLOCK(ASM_LINE("cmp %2, %1")                                    \
              ASM_LINE(#cmov " %3, %0")                                 \
              : /* writes */                                            \
              /* %0 */ "+r" (var)                                       \
              : /* reads */                                             \
              /* %1 */ "ri" (cmp1),                                     \
              /* %2 */ "rm" (cmp2),                                     \
              /* %3 */ "r" (_new)                                       \
              : /* clobbers */   "cc");                                 \
    }

    
