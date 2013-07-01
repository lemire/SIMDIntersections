#pragma once

#include <immintrin.h>
#include <smmintrin.h>

#define VEC_T __m128i
#define VEC_FLOAT_T __m128

// NOTE: Using "asm volatile" required to keep desired ordering
//       Remove to allow compiler to reschedule operations
// #define VOLATILE
#define VOLATILE volatile

#define VEC_OR(dest, other)                                     \
    asm VOLATILE("por %1, %0" : "+x" (dest) : "x" (other) )

#define VEC_LOAD(dest, ptr, offset)                             \
    asm VOLATILE("movdqu %c2(%1), %0" : "=x" (dest) :           \
                 "g" (ptr), "i" ((offset) * sizeof(VEC_T)) )

#define VEC_STORE(ptr, src)                                     \
    asm VOLATILE("movdqu %1, %0" : "=m" (*ptr) : "x" (src))

#define VEC_SET_ALL_TO_INT(dest, int32)                                 \
    asm VOLATILE("movd %1, %0; pshufd $0, %0, %0" : "=x" (dest) : "g" (int32) ) 

#define VEC_MATCH(dest, other)                                  \
    asm VOLATILE("pcmpeqd %1, %0" : "+x" (dest) : "x" (other) : "cc" )

#define VEC_SET_ALL_FROM_INDEX(dest, src, index)                        \
    asm VOLATILE("pshufd %2, %1, %0" : "=x" (dest) : "x" (src),       \
                 "i" (index << 6 | index << 4 | index << 2 | index))

#define VEC_SHUFFLE_BYTES(dest, shuf)                                   \
    asm VOLATILE("pshufb %1, %0" : "+x" (dest) : "m" (shuf))       

#define VEC_ZERO(reg)                                                   \
    asm VOLATILE("pxor %0, %0" : "=x" (reg))                              

#define VEC_READ_MASK(reg) _mm_movemask_ps((VEC_FLOAT_T) reg)
