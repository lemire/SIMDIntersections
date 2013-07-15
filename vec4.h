#pragma once

#include <immintrin.h>
#include <smmintrin.h>
#include <nmmintrin.h>

#define VEC_T __m128i
#define VEC_FLOAT_T __m128

// NOTE: Using "asm volatile" required to keep desired ordering
//       Remove to allow compiler to reschedule operations
// #define VEC_VOLATILE
#define VEC_VOLATILE volatile

#define VEC_LOAD(xmm, ptr)                      \
    VEC_LOAD_OFFSET(xmm, ptr, 0)                            

#define VEC_LOAD_OFFSET(xmm, ptr, bytes)                    \
    __asm VEC_VOLATILE("movdqu %c2(%1), %0" : "=x" (xmm) :  \
                   "g" (ptr), "i" (bytes))

#define VEC_LOAD_OFFSET_BASE(xmm, ptr, bytes, base)     \
    VEC_LOAD_OFFSET_BASE_MUL(xmm, ptr, bytes, base, 1)            

#define VEC_LOAD_OFFSET_BASE_MUL(xmm, ptr, bytes, base, mul)            \
    __asm VEC_VOLATILE("movdqu %c2(%1, %3, %c4), %0" : "=x" (xmm) :     \
                   "g" (ptr), "i" (bytes), "g" (base), "i" (mul) )


#define VEC_STORE(ptr, src)                                     \
    __asm VEC_VOLATILE("movdqu %1, %0" : "=m" (*ptr) : "x" (src))
    
#define VEC_SET_ALL_TO_INT(reg, int32)                                  \
    __asm VEC_VOLATILE("movd %1, %0; pshufd $0, %0, %0"                 \
                       : "=x" (reg) : "g" (int32) ) 

#define VEC_MATCH(dest, other)                                          \
    __asm VEC_VOLATILE("pcmpeqd %1, %0" : "+x" (dest) : "x" (other))

#define VEC_SET_ALL_FROM_INDEX(dest, src, index)                        \
    __asm VEC_VOLATILE("pshufd %2, %1, %0" : "=x" (dest) : "x" (src),   \
                       "i" (index << 6 | index << 4 | index << 2 | index))
    
#define VEC_SHUFFLE_BYTES(dest, shuf)                                   \
    __asm VEC_VOLATILE("pshufb %1, %0" : "+x" (dest) : "m" (shuf))       
    
#define VEC_ZERO(reg)                                   \
    __asm VEC_VOLATILE("pxor %0, %0" : "=x" (reg))                              
    
#define VEC_READ_MASK(mask, xmm)                                        \
    __asm VEC_VOLATILE("movmskps %1, %0" : "=r" (mask) : "x" (xmm))
    
// NOTE: sets dest[n] if signed dest[n] > signed other[n]
#define VEC_CMP_GREATER(dest, other)                                    \
    __asm VEC_VOLATILE("pcmpgtd %1, %0" : "+x" (dest) : "x" (other))


//  Port 1|5, Latency 1, Throughput 2
#define VEC_CMP_EQUAL(dest, other)                                      \
    __asm VEC_VOLATILE("pcmpeqd %1, %0" : "+x" (dest) : "x" (other))
    
#define VEC_IS_ZERO(xmm) VEC_PTEST_ALL_ZERO(xmm, xmm)

#define VEC_PTEST_ALL_ZERO(xmm1, xmm2) _mm_testz_si128(xmm1, xmm2)
    
#define VEC_PTEST_ALL_SET(xmm1, xmm2)  _mm_testc_si128(xmm1, xmm2)

#define VEC_POPCNT(count, var)                                          \
    __asm VEC_VOLATILE("popcnt %1, %0" : "=r" (count) : "g" (var))       

// PTEST P1-L1-T1, CMOVcc 2P0|1|5-L2-T1
#define VEC_SET_PTEST(var, val, xmm)                                  \
    __asm VEC_VOLATILE("ptest %2, %2; cmovnz %1, %0" :                \
                       /* writes %0 */ "+r" (var) :                   \
                       /* reads  %1, %2 */ "r" (val), "x" (xmm) : \
                       /* clobbers */ "cc")
// NOTE: "+r" (var) since the initial value is needed too

#define VEC_ADD_PTEST(var, add, xmm)      {                             \
        typeof(var) _new = var + add;                                   \
        __asm VEC_VOLATILE("ptest %2, %2\n\t"                           \
                           "cmovnz %1, %0\n\t"                          \
                           : /* writes */ "+r" (var)                    \
                           : /* reads */  "r" (_new), "x" (xmm)         \
                           : /* clobbers */ "cc");                      \
    }

#define VEC_COPY(xmm_dest, xmm_src)                                     \
    __asm VEC_VOLATILE("movdqa %1, %0" : "=x" (xmm_dest) : "x" (xmm_src))       

#define VEC_COPY_LOW(reg_dest, xmm_src)                                 \
    __asm VEC_VOLATILE("movd %1, %0" : "=r" (reg_dest) : "x" (xmm_src))       


#define VEC_ADD(xmm_dest, xmm_src)                                      \
    __asm VEC_VOLATILE("paddd %1, %0" : "=x" (xmm_dest) : "x" (xmm_src))       

//  Port 0|1|5, Latency 1, Throughput 3
#define VEC_OR(dest, other)                                             \
    __asm VEC_VOLATILE("por %1, %0" : "+x" (dest) : "x" (other) )

#define VEC_FORMAT_DEBUG(xmm)                                           \
    "%d %d %d %d\n",                                                    \
        _mm_extract_epi32(xmm, 0),                                \
        _mm_extract_epi32(xmm, 1),                                \
        _mm_extract_epi32(xmm, 2),                                \
        _mm_extract_epi32(xmm, 3)

