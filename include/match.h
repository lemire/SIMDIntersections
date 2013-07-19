#pragma once

#include <stdint.h>
#include <stddef.h>
#include <immintrin.h>

#define VEC_T __m128i

#define VEC_COPY_LOW(reg_dest, xmm_src)                                 \
    __asm volatile("movd %1, %0" : "=r" (reg_dest) : "x" (xmm_src))

#define VEC_OR(dest, other)                                             \
    __asm volatile("por %1, %0" : "+x" (dest) : "x" (other) )

#define VEC_ADD_PTEST(var, add, xmm)      {                             \
        typeof(var) _new = var + add;                                   \
        __asm volatile("ptest %2, %2\n\t"                           \
                           "cmovnz %1, %0\n\t"                          \
                           : /* writes */ "+r" (var)                    \
                           : /* reads */  "r" (_new), "x" (xmm)         \
                           : /* clobbers */ "cc");                      \
    }

#define VEC_CMP_GREATER(dest, other)                                    \
    __asm volatile("pcmpgtd %1, %0" : "+x" (dest) : "x" (other))

#define VEC_CMP_EQUAL(dest, other)                                      \
    __asm volatile("pcmpeqd %1, %0" : "+x" (dest) : "x" (other))

#define VEC_SET_ALL_TO_INT(reg, int32)                                  \
    __asm volatile("movd %1, %0; pshufd $0, %0, %0"                 \
                       : "=x" (reg) : "g" (int32) )

#define VEC_LOAD_OFFSET(xmm, ptr, bytes)                    \
    __asm volatile("movdqu %c2(%1), %0" : "=x" (xmm) :  \
                   "r" (ptr), "i" (bytes))

#define COMPILER_LIKELY(x)     __builtin_expect((x),1)
#define COMPILER_RARELY(x)     __builtin_expect((x),0)

#define ASM_LEA_ADD_BYTES(ptr, bytes)                            \
    __asm volatile("lea %c1(%0), %0\n\t" :                       \
                   /* reads/writes %0 */  "+r" (ptr) :           \
                   /* reads */ "i" (bytes));


#ifdef __cplusplus

#define typeof(arg) decltype(arg)

extern "C" {
#endif

size_t match_scalar(const uint32_t *A, const size_t lenA,
                    const uint32_t *B, const size_t lenB,
                    uint32_t *out);


size_t match_v4_f2_p0
(const uint32_t *rare, size_t lenRare,
 const uint32_t *freq, size_t lenFreq,
 uint32_t *matchOut);


size_t match_v4_f4_p0
(const uint32_t *rare, size_t lenRare,
 const uint32_t *freq, size_t lenFreq,
 uint32_t *matchOut);


size_t match_v4_f8_p0
(const uint32_t *rare, size_t lenRare,
 const uint32_t *freq, size_t lenFreq,
 uint32_t *matchOut);


size_t match_v4_f2_p1
(const uint32_t *rare, size_t lenRare,
 const uint32_t *freq, size_t lenFreq,
 uint32_t *matchOut);

size_t match_v4_f4_p1
(const uint32_t *rare, size_t lenRare,
 const uint32_t *freq, size_t lenFreq,
 uint32_t *matchOut);


size_t match_v4_f8_p1
(const uint32_t *rare, size_t lenRare,
 const uint32_t *freq, size_t lenFreq,
 uint32_t *matchOut);


size_t match_v4_f2_p2
(const uint32_t *rare, size_t lenRare,
 const uint32_t *freq, size_t lenFreq,
 uint32_t *matchOut);


size_t match_v4_f4_p2
(const uint32_t *rare, size_t lenRare,
 const uint32_t *freq, size_t lenFreq,
 uint32_t *matchOut);


size_t match_v4_f8_p2
(const uint32_t *rare, size_t lenRare,
 const uint32_t *freq, size_t lenFreq,
 uint32_t *matchOut);


size_t match_v4_f2_p3
(const uint32_t *rare, size_t lenRare,
 const uint32_t *freq, size_t lenFreq,
 uint32_t *matchOut);


size_t match_v4_f4_p3
(const uint32_t *rare, size_t lenRare,
 const uint32_t *freq, size_t lenFreq,
 uint32_t *matchOut);

size_t match_v4_f8_p3
(const uint32_t *rare, size_t lenRare,
 const uint32_t *freq, size_t lenFreq,
 uint32_t *matchOut);

#ifdef __cplusplus
} // extern "C"
#endif
