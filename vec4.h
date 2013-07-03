#pragma once

#include <immintrin.h>
#include <smmintrin.h>
#include <nmmintrin.h>
 

#define VEC_T __m128i
#define VEC_FLOAT_T __m128

// NOTE: Using "asm volatile" required to keep desired ordering
//       Remove to allow compiler to reschedule operations
// #define VOLATILE
#define VOLATILE volatile

#define VEC_OR(dest, other)                                     \
    asm VOLATILE("por %1, %0" : "+x" (dest) : "x" (other) )

#define VEC_LOAD_OFFSET(reg, ptr, offset)                      \
    asm VOLATILE("movdqu %c2(%1), %0" : "=x" (reg) :           \
                 "g" (ptr), "i" ((offset) * sizeof(VEC_T)) )

#define VEC_LOAD(reg, ptr)                                          \
    asm VOLATILE("movdqu (%1), %0" : "=x" (reg) : "g" (ptr) )

#define VEC_STORE(ptr, src)                                     \
    asm VOLATILE("movdqu %1, %0" : "=m" (*ptr) : "x" (src))

#define VEC_SET_ALL_TO_INT(reg, int32)                                 \
    asm VOLATILE("movd %1, %0; pshufd $0, %0, %0" : "=x" (reg) : "g" (int32) ) 

#define VEC_MATCH(dest, other)                                  \
    asm VOLATILE("pcmpeqd %1, %0" : "+x" (dest) : "x" (other))

#define VEC_SET_ALL_FROM_INDEX(dest, src, index)                        \
    asm VOLATILE("pshufd %2, %1, %0" : "=x" (dest) : "x" (src),       \
                 "i" (index << 6 | index << 4 | index << 2 | index))

#define VEC_SHUFFLE_BYTES(dest, shuf)                                   \
    asm VOLATILE("pshufb %1, %0" : "+x" (dest) : "m" (shuf))       

#define VEC_ZERO(reg)                                                   \
    asm VOLATILE("pxor %0, %0" : "=x" (reg))                              

#define VEC_READ_MASK(mask, xmm)                                \
    asm VOLATILE("movmskps %1, %0" : "=r" (mask) : "x" (xmm))

#define VEC_CMP_GREATER(dest, other) \
    asm VOLATILE("pcmpgtd %1, %0" : "+x" (dest) : "x" (other))

#define VEC_IS_ZERO(reg) _mm_testz_si128(reg, reg)

#define VEC_POPCNT(count, var)                   \
    asm VOLATILE("popcnt %1, %0" : "=r" (count) : "g" (var))       

#define VEC_SET_PTEST(var, val, xmm) \
    asm VOLATILE("ptest %2, %2; cmovnz %1, %0" :          \
                 /* writes */ "+r" (var) :                \
                 /* reads */ "r" (val), "x" (xmm) :       \
                 /* clobbers */ "cc")
// NOTE: "+r" (var) since the initial value is needed too


#define VEC_COPY(xmm_dest, xmm_src)             \
    asm VOLATILE("movdqa %1, %0" : "=x" (xmm_dest) : "x" (xmm_src))       
