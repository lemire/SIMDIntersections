#include <stdint.h>
#include <stddef.h>
#include <strings.h>

#ifdef IACA
#include </opt/intel/iaca-lin32/include/iacaMarks.h>
#endif  

#include <immintrin.h>
#include <smmintrin.h>

// May help choose order of branches, but mostly comments to reader
#define usually(x)    __builtin_expect((x),1)
#define likely(x)     __builtin_expect((x),1)
#define often(x)      __builtin_expect((x),1)
#define sometimes(x)  __builtin_expect((x),0)
#define unlikely(x)   __builtin_expect((x),0)
#define rarely(x)     __builtin_expect((x),0)

#include <stdlib.h>
#include <stdio.h>

#define VECTYPE __m128i

#define CHUNKINTS (NUMVECS * (sizeof(VECTYPE)/sizeof(uint32_t))) 

#define COMPILER_BARRIER asm volatile("" ::: "memory")
// #define COMPILER_BARRIER

#define ASSERT(x) // do nothing



size_t finish_scalar(const uint32_t *A, size_t lenA,
                     const uint32_t *B, size_t lenB) {

    size_t count = 0;
    if (lenA == 0 || lenB == 0) return count;
    
    const uint32_t *endA = A + lenA;
    const uint32_t *endB = B + lenB;
    
    
    while (1) {
        while (*A < *B) {
        SKIP_FIRST_COMPARE:
            if (++A == endA) return count; 
        }
        while (*A > *B) {
            if (++B == endB) return count; 
        }
        if (*A == *B) { 
            count++;
            if (++A == endA || ++B == endB) return count;
        }
        else {
            goto SKIP_FIRST_COMPARE;
        }
    }
    
    return count; // NOTREACHED
}

#if ! defined(LOOKAHEAD) || LOOKAHEAD > 8
#error LOOKAHEAD must be defined and one of {0,1,2,3,4,5,6,7,8}
#endif

#if ! (NUMVECS == 1 || NUMVECS == 2 || NUMVECS == 4 ||  NUMVECS == 8  \
       || NUMVECS == 12 || NUMVECS == 16 || NUMVECS == 24 || NUMVECS == 32)
#error NUMVECS must be one of {1, 2, 4, 8, 12, 16, 24, 32}
#endif

#if LOOKAHEAD > 0
#define SAFESPACE (LOOKAHEAD * CHUNKINTS - 1)
#else 
#define SAFESPACE (CHUNKINTS - 1)
#endif

#define TESTZERO(reg) _mm_testz_si128(reg, reg)

// NOTE: Using "asm volatile" required to keep desired ordering
//       Remove to allow compiler to reschedule operations
#define VOLATILE volatile

// #define REGISTER(reg) 
#define REGISTER(reg) asm(reg)

#define VOR(dest, other)                                      \
    asm VOLATILE("por %1, %0" : "+x" (dest) : "x" (other) );

#define VLOAD(dest, ptr, offset)                                        \
    asm VOLATILE("movdqu %c2(%1), %0" : "=x" (dest) : "g" (ptr), "i" (offset * sizeof(VECTYPE)) );

#define VSETALL(dest, int32)                                            \
    asm VOLATILE("movd %1, %0; pshufd $0, %0, %0" : "=x" (dest) : "g" (int32) ) 

#define VMATCH(dest, other)                                   \
    asm VOLATILE("pcmpeqd %1, %0" : "+x" (dest) : "x" (other) )

// NOTE: doesn't seem needed, can just use "dest = src"?
// #define VMOVE(dest, src) asm VOLATILE("movdqa %1, %0" : "=x" (dest) : "x" (src) );

#define NUMVECS 8

size_t search_tight(const uint32_t *freq, size_t lenFreq,
                    const uint32_t *rare, size_t lenRare) {

    size_t count = 0;
    if (lenFreq == 0 || lenRare == 0) {
        return 0;
    }

    const uint32_t *lastRare = &rare[lenRare];
    const uint32_t *lastFreq = &rare[lenRare];
    const uint32_t *stopFreq = lastFreq - SAFESPACE;
    
    // skip straight to scalar if not enough room to load vectors
    if (rarely(freq >= stopFreq)) {
        goto FINISH_SCALAR;
    }

    register VECTYPE F0 REGISTER("xmm0");
    register VECTYPE F1 REGISTER("xmm1");
    register VECTYPE F2 REGISTER("xmm2");
    register VECTYPE F3 REGISTER("xmm3");


    register VECTYPE Freq REGISTER("xmm4");
    register VECTYPE Rare REGISTER("xmm5");
    register VECTYPE NextFreq REGISTER("xmm6");
    register VECTYPE NextRare REGISTER("xmm7");
    // FUTURE: could add NextRare1, NextRare2, NextRare3, NextRare4

    register VECTYPE Match REGISTER("xmm15");


    uint32_t minFreq; 
    uint32_t minRare;

    Freq = NextFreq;
    Rare = NextRare;

    // Calculate next vectors
    // PROFILE: verify branch-free conditional moves

    // NOTE: reload unaligned proceeding as far as we can
    // FUTURE: Use VECTYPE and add granularity control with RAREGRANULARITY1

#if ! RAREADVANCE1 && ! RAREADVANCE2
#error Must define at least one of these
#endif

#if RAREADVANCE1
    minFreq = freq[0];

    if (rare[0] < minFreq) { 
        rare += 1;
    }
    if (rare[1] < minFreq) { 
        rare += 1;
    }
    if (rare[2] < minFreq) { 
        rare += 1;
    }
    if (rare[3] < minFreq) { 
        rare += 1;
    }
    // NOTE: use newly calculated rare[0] for greater advance
#endif // RAREADVANCE1

    minRare = rare[0]; 

    // FUTURE: control granularity with FREQADVANCE
    if (freq[VECLEN - 1] < minRare) {
        freq += VECLEN;
    }
    if (freq[2 * VECLEN - 1] < minRare) {
        freq += VECLEN;
    }
    if (freq[3 * VECLEN - 1] < minRare) {
        freq += VECLEN;
    }
    if (freq[4 * VECLEN - 1] < minRare) {
        freq += VECLEN;
    }


    // Profile: verify conditional moves are used
#if RAREADVANCE2
    minFreq = freq[0];

#if RAREADVANCE2 == 1
    if (rare[0] < minFreq) { 
        rare += RAREADVANCE2;
    }
#endif // 1

#if RAREADVANCE2 == 1 || RAREADVANCE2 == 2
    if (rare[1] < minFreq) { 
        rare += RAREADVANCE2;
    }
#endif // 2

#if RAREADVANCE2 == 1
    if (rare[2] < minFreq) { 
        rare += RAREADVANCE2;
    }
#endif // 1

#if RAREADVANCE2 == 1 || RAREADVANCE2 == 2 || RAREADVANCE2 == 4
    if (rare[3] < minFreq) { 
        rare += RAREADVANCE2;
    }
#endif // 1 || 2 || 4

#endif // RAREADVANCE2

    VLOAD(NextFreq, freq, 0);  
    VLOAD(NextRare, rare, 0);


    // FUTURE: choose among these with NUMVECS

    Source = Freq;
    VLOAD(Freq, freq, 1);  // load freq[1]

    VSHUF(Match, Source, 0);  // F0

    VSHUF(F1, Source, 1);
    VMATCH(Match, Rare);      // F0

    VSHUF(F2, Source, 2);
    VMATCH(F1, Rare);
    // VOR(Match, F0);

    VSHUF(F3, Source, 3);
    VMATCH(F2, Rare);
    VOR(Match, F1);

    Source = Freq;
    VLOAD(Freq, freq, 2);  // load freq[2]

    VSHUF(F0, Source, 0);
    VMATCH(F3, Rare);
    VOR(Match, F2);

    VSHUF(F1, Source, 1);
    VMATCH(F0, Rare);
    VOR(Match, F3);

    VSHUF(F2, Source, 2);
    VMATCH(F1, Rare);
    VOR(Match, F0);

    VSHUF(F3, Source, 3);
    VMATCH(F2, Rare);
    VOR(Match, F1);

    Source = Freq;
    VLOAD(Freq, freq, 3);  // load freq[3]

    VSHUF(F0, Source, 0);
    VMATCH(F3, Rare);
    VOR(Match, F2);

    VSHUF(F1, Source, 1);
    VMATCH(F0, Rare);
    VOR(Match, F3);

    VSHUF(F2, Source, 2);
    VMATCH(F1, Rare);
    VOR(Match, F0);

    VSHUF(F3, Source, 3);
    VMATCH(F2, Rare);
    VOR(Match, F1);

    Source = Freq;
    VLOAD(Freq, freq, 4);  // load freq[4]

    VSHUF(F0, Source, 0);
    VMATCH(F3, Rare);
    VOR(Match, F2);

    VSHUF(F1, Source, 1);
    VMATCH(F0, Rare);
    VOR(Match, F3);

    VSHUF(F2, Source, 2);
    VMATCH(F1, Rare);
    VOR(Match, F0);

    VSHUF(F3, Source, 3);
    VMATCH(F2, Rare);
    VOR(Match, F1);

    Source = Freq;
    VLOAD(Freq, freq, 5);  // load freq[5]

    VSHUF(F0, Source, 0);
    VMATCH(F3, Rare);
    VOR(Match, F2);

    VSHUF(F1, Source, 1);
    VMATCH(F0, Rare);
    VOR(Match, F3);

    VSHUF(F2, Source, 2);
    VMATCH(F1, Rare);
    VOR(Match, F0);

    VSHUF(F3, Source, 3);
    VMATCH(F2, Rare);
    VOR(Match, F1);

    Source = Freq;
    VLOAD(Freq, freq, 6);  // load freq[6]

    VSHUF(F0, Source, 0);
    VMATCH(F3, Rare);
    VOR(Match, F2);

    VSHUF(F1, Source, 1);
    VMATCH(F0, Rare);
    VOR(Match, F3);

    VSHUF(F2, Source, 2);
    VMATCH(F1, Rare);
    VOR(Match, F0);

    VSHUF(F3, Source, 3);
    VMATCH(F2, Rare);
    VOR(Match, F1);

    Source = Freq;
    VLOAD(Freq, freq, 7);  // load freq[7]

    VSHUF(F0, Source, 0);
    VMATCH(F3, Rare);
    VOR(Match, F2);

    VSHUF(F1, Source, 1);
    VMATCH(F0, Rare);
    VOR(Match, F3);

    VSHUF(F2, Source, 2);
    VMATCH(F1, Rare);
    VOR(Match, F0);

    VSHUF(F3, Source, 3);
    VMATCH(F2, Rare);
    VOR(Match, F1);

    //  -------------

    VMATCH(F3, Rare);
    VOR(Match, F2);

    VOR(Match, F3);
    
    matchBits = _mm_movemask_ps((m128) Match);     // FIXME: cast to correct float VECTYPE
    passCount = _mm_popcnt_u32(matchBits);
    count += passCount;

