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
#define expect(x)     __builtin_expect((x),1)
#define expected(x)   __builtin_expect((x),1)
#define sometimes(x)  __builtin_expect((x),0)
#define unlikely(x)   __builtin_expect((x),0)
#define rarely(x)     __builtin_expect((x),0)
#define unexpected(x) __builtin_expect((x),0)

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


#if (NUMVECS < 2 || NUMVECS > 32)
#error "NUMVECS must be defined anqd between 2 and 32"
#endif

#define SAFESPACE (CHUNKINTS - 1)

#define TESTZERO(reg) _mm_testz_si128(reg, reg)

// NOTE: Using "asm volatile" required to keep desired ordering
//       Remove to allow compiler to reschedule operations
#define VOLATILE volatile

// #define REGISTER(reg) 
#define REGISTER(reg) asm(reg)


#define VOR(dest, other)                                        \
    asm VOLATILE("por %1, %0" : "+x" (dest) : "x" (other) );

#define VLOAD(dest, ptr, offset)                                        \
    asm VOLATILE("movdqu %c2(%1), %0" : "=x" (dest) : "g" (ptr), "i" (offset * sizeof(VECTYPE)) );

#define VSETALL(dest, int32)                                            \
    asm VOLATILE("movd %1, %0; pshufd $0, %0, %0" : "=x" (dest) : "g" (int32) ) 

#define VMATCH(dest, other)                                     \
    asm VOLATILE("pcmpeqd %1, %0" : "+x" (dest) : "x" (other) )

// NOTE: doesn't seem needed, can just use "dest = src"?
// #define VMOVE(dest, src) asm VOLATILE("movdqa %1, %0" : "=x" (dest) : "x" (src) );



#define NUMVECS 8

size_t search_tight(const uint32_t *freq, size_t lenFreq,
                    const uint32_t *rare, size_t lenRare)  {

    size_t count = 0;
    if (lenFreq == 0 || lenRare == 0) {
        return 0;
    }

    const uint32_t *lastRare = &rare[lenRare];
    const uint32_t *lastFreq = &rare[lenRare];
    const uint32_t *stopFreq = lastFreq - SAFESPACE;
    
    // skip straight to scalar if not enough room to load vectors
    if (rarely(freq >= stopFreq) || rarely(rare >= stopRare)) {
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
    // FUTURE: could add Rare/NextRare1, Rare/NextRare2, Rare/NextRare3, NextRare4

    register VECTYPE Match REGISTER("xmm15");

    VLOAD(NextFreq, freq, 0);
    VLOAD(NextRare, rare, 0);
    
    uint32_t lastPass = 0;  // Next pass is tested but current pass is always completed
    uint32_t nextRare;
    uint32_t nextFreq;
 
    // FUTURE: have separate FREQVECS and RAREVECS
   
    while (! lastPass) {
        Freq = NextFreq;
        Rare = NextRare;

        // Calculate what to do next AFTER we check current freq against current rare
        uint32_t maxFreq = freq[NUMVECS * VECLEN - 1];  // highest from multiple freq vectors
        uint32_t maxRare = rare[VECLEN - 1];            // highest from single rare vector
    
        uint32_t advance;
    
        // NOTE: if clause should be flow controlled, else clause should be branchless
        if (expected(maxFreq > maxRare)) {  // but if all of Rare was already checked
            nextRare = rare + VECLEN;           // jump to the next full vector
        } else {                            // otherwise consider individually and RELOAD UNALIGNED
            nextRare = rare;
            if (maxFreq > rare[0]) { 
                nextRare += 1;   // PROFILE: cmov
            }

            if (maxFreq > rare[1]) { 
                nextRare += 1;   // PROFILE: cmov
            }

            if (maxFreq > rare[2]) { 
                nextRare += 1;   // PROFILE: cmov
            }

            COMPILE_TIME_ASSERT(VECLEN == 4);  // FUTURE: generalize for VECTYPE
        }

        // PROFILE:  is it beneficial to start the real work at this point?
        Source = Freq;
        VLOAD(Freq, freq, 1);  // load freq[1]

        VSHUF(Match, Source, 0);  

        VSHUF(F1, Source, 1);
        VMATCH(Match, Rare);      

        VSHUF(F2, Source, 2);
        VMATCH(F1, Rare);
        // VOR(Match, F0);

        VSHUF(F3, Source, 3);
        VMATCH(F2, Rare);
        VOR(Match, F1);

        // Jump over the loads if this is going to be the final pass
        if (expected(nextRare < stopRare) && expected(nextFreq < stopFreq)) {
#define GRANULAR_ADVANCE(vecnum)                                        \
            if (NUMVECS > vecnum && vecnum % GRANULARITY == 0) {        \
                if (freq[vecnum * VECLEN - 1] < nextRare[0]) {          \
                    nextFreq += GRANULARITY * VECLEN;                   \
                }                                                       \
            }                                                           \
        
            // PROFILE: verify conditional moves are used
            // GRANULAR_ADVANCE(1) ... GRANULAR_ADVANCE(16)
            REPEAT_ADD(GRANULAR_ADVANCE, 16, 1, 1);
        
#undef GRANULAR_ADVANCE
        
            // PROFILE: are these two always ready for next pass?
            VLOAD(NextFreq, nextFreq, 0);  
            VLOAD(NextRare, nextRare, 0);
        } else {
            lastPass = 1;
        }
    
#define CHECK_VECTOR(vecnum)                    \
        if (NUMVECS > vecnum) {                 \
            Source = Freq;                      \
            VLOAD(Freq, freq, vecnum);          \
                                                \
            VSHUF(F0, Source, 0);               \
            VMATCH(F3, Rare);                   \
            VOR(Match, F2);                     \
                                                \
            VSHUF(F1, Source, 1);               \
            VMATCH(F0, Rare);                   \
            VOR(Match, F3);                     \
                                                \
            VSHUF(F2, Source, 2);               \
            VMATCH(F1, Rare);                   \
            VOR(Match, F0);                     \
                                                \
            VSHUF(F3, Source, 3);               \
            VMATCH(F2, Rare);                   \
            VOR(Match, F1);                     \
        }

        // CHECK_VECTOR(2) ... CHECK_VECTOR(32)
        REPEAT_ADD(CHECK_VECTOR, 30, 2, 1);

#undef CHECK_VECTOR(vecnum)

        VMATCH(F3, Rare);
        VOR(Match, F2);

        VOR(Match, F3);

        freq = nextFreq;
        rare = nextRare;
    
        matchBits = _mm_movemask_ps((m128) Match);     // FIXME: cast to correct float VECTYPE
        passCount = _mm_popcnt_u32(matchBits);
        count += passCount;
    }

 FINISH_SCALAR:
    return count; // plus scalar
}
