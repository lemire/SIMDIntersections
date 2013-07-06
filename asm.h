#pragma once

// #define ASM_REGISTER(decl, reg, args...) decl args

#define ASM_REGISTER(decl, reg, args...) register decl __asm(#reg) args
#define ASM_LINE(command) "\t" command "\n"
#define ASM_BLOCK(args...) __asm VOLATILE(args)

#define ASM_PTR_ADD(ptr, offset)                                 \
    ASM_BLOCK(ASM_LINE("lea (%0, %1, %c2), %0") :                \
              /* writes %0 */ "+r" (ptr) :                       \
              /* reads %1 */ "r" (offset),                       \
              /* reads %2 */ "i" (sizeof(*ptr)));

#define ASM_PUSH_REGISTERS()                    \
    ASM_BLOCK(                                  \
              ASM_LINE("push %rax")            \
              ASM_LINE("push %rcx")            \
              ASM_LINE("push %rdx")            \
              ASM_LINE("push %rsi")            \
              ASM_LINE("push %rdi")            \
              ASM_LINE("push %r8")             \
              ASM_LINE("push %r9")             \
              ASM_LINE("push %r10")            \
              ASM_LINE("push %r11")             \
              ASM_LINE("sub $16, %rsp; movdqa %xmm0, (%rsp)")   \
              ASM_LINE("sub $16, %rsp; movdqa %xmm1, (%rsp)")   \
              ASM_LINE("sub $16, %rsp; movdqa %xmm2, (%rsp)")   \
              ASM_LINE("sub $16, %rsp; movdqa %xmm3, (%rsp)")   \
              ASM_LINE("sub $16, %rsp; movdqa %xmm4, (%rsp)")   \
              ASM_LINE("sub $16, %rsp; movdqa %xmm5, (%rsp)")   \
              ASM_LINE("sub $16, %rsp; movdqa %xmm6, (%rsp)")   \
              ASM_LINE("sub $16, %rsp; movdqa %xmm7, (%rsp)")   \
              ASM_LINE("sub $16, %rsp; movdqa %xmm8, (%rsp)")   \
              ASM_LINE("sub $16, %rsp; movdqa %xmm9, (%rsp)")   \
              ASM_LINE("sub $16, %rsp; movdqa %xmm10, (%rsp)")   \
              ASM_LINE("sub $16, %rsp; movdqa %xmm11, (%rsp)")   \
              ASM_LINE("sub $16, %rsp; movdqa %xmm12, (%rsp)")   \
              ASM_LINE("sub $16, %rsp; movdqa %xmm13, (%rsp)")   \
              ASM_LINE("sub $16, %rsp; movdqa %xmm14, (%rsp)")   \
              ASM_LINE("sub $16, %rsp; movdqa %xmm15, (%rsp)")   \
                                                )
#define ASM_POP_REGISTERS()                    \
    ASM_BLOCK(                                  \
              ASM_LINE("movdqa (%rsp), %xmm15; add $16, %rsp")  \
              ASM_LINE("movdqa (%rsp), %xmm14; add $16, %rsp")  \
              ASM_LINE("movdqa (%rsp), %xmm13; add $16, %rsp")  \
              ASM_LINE("movdqa (%rsp), %xmm12; add $16, %rsp")  \
              ASM_LINE("movdqa (%rsp), %xmm11; add $16, %rsp")  \
              ASM_LINE("movdqa (%rsp), %xmm10; add $16, %rsp")  \
              ASM_LINE("movdqa (%rsp), %xmm9; add $16, %rsp")  \
              ASM_LINE("movdqa (%rsp), %xmm8; add $16, %rsp")  \
              ASM_LINE("movdqa (%rsp), %xmm7; add $16, %rsp")  \
              ASM_LINE("movdqa (%rsp), %xmm6; add $16, %rsp")  \
              ASM_LINE("movdqa (%rsp), %xmm5; add $16, %rsp")  \
              ASM_LINE("movdqa (%rsp), %xmm4; add $16, %rsp")  \
              ASM_LINE("movdqa (%rsp), %xmm3; add $16, %rsp")  \
              ASM_LINE("movdqa (%rsp), %xmm2; add $16, %rsp")  \
              ASM_LINE("movdqa (%rsp), %xmm1; add $16, %rsp")  \
              ASM_LINE("movdqa (%rsp), %xmm0; add $16, %rsp")  \
              ASM_LINE("pop %r11")             \
              ASM_LINE("pop %r10")            \
              ASM_LINE("pop %r9")             \
              ASM_LINE("pop %r8")             \
              ASM_LINE("pop %rdi")            \
              ASM_LINE("pop %rsi")            \
              ASM_LINE("pop %rdx")            \
              ASM_LINE("pop %rcx")            \
              ASM_LINE("pop %rax")            \
                                                )
