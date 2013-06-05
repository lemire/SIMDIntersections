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
#define VECFLOAT _m128

#define CHUNKINTS (NUMVECS * (sizeof(VECTYPE)/sizeof(uint32_t))) 

#define COMPILER_BARRIER asm volatile("" ::: "memory")
// #define COMPILER_BARRIER


#define COMPILER_ASSERT(x) // do nothing
COMPILER_ASSERT((NUMRARE * VECLEN) % GRANRARE == 0);
COMPILER_ASSERT(NUMFREQ % GRANFREQ == 0);

#define DEBUG_ASSERT(x) // do nothing


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


#if (NUMFREQ < 2 || NUMFREQ > 32)
#error "NUMVECS must be defined and between 2 and 32"
#endif

#define SAFESPACE (CHUNKINTS - 1)

#define TESTZERO(reg) _mm_testz_si128(reg, reg)

// NOTE: Using "asm volatile" required to keep desired ordering
//       Remove to allow compiler to reschedule operations
// #define VOLATILE
#define VOLATILE volatile

// #define REGISTER(reg) 
#define REGISTER(reg) asm(reg)


#define VOR(dest, other)                                        \
    asm VOLATILE("por %1, %0" : "+x" (dest) : "x" (other) );

#define VLOAD(dest, ptr, offset)                      \
    asm VOLATILE("movdqu %c2(%1), %0" : "=x" (dest) : \
                 "g" (ptr), "i" (offset * sizeof(VECTYPE)) );

#define VSETALL(dest, int32)                                            \
    asm VOLATILE("movd %1, %0; pshufd $0, %0, %0" : "=x" (dest) : "g" (int32) ) 

#define VMATCH(dest, other)                                     \
    asm VOLATILE("pcmpeqd %1, %0" : "+x" (dest) : "x" (other) )

// NOTE: doesn't seem needed, can just use "dest = src"?
// #define VMOVE(dest, src) asm VOLATILE("movdqa %1, %0" : "=x" (dest) : "x" (src) );

#define FUNC(name) name ## _R ## NUMRARE ## _F ## NUMFREQ

size_t FUNC(search_tight)(const uint32_t *freq, size_t lenFreq,
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

    register VECTYPE Match REGISTER("xmm4");

    register VECTYPE Rare REGISTER("xmm5");
    register VECTYPE Freq REGISTER("xmm6");

    register VECTYPE NextRare REGISTER("xmm7");
    register VECTYPE NextFreq REGISTER("xmm8");

    VLOAD(NextFreq, freq, 0);
    VLOAD(NextRare, rare, 0);
    
    uint32_t lastPass = 0;  // current pass is always completed
    uint32_t nextRare;
    uint32_t nextFreq;

    while (! lastPass) {
        // Calculate what to do next AFTER we check current freq against current rare
        uint32_t maxFreq = freq[NUMFREQ * VECLEN - 1];  // highest from freq vectors(s)
    
#ifndef NOMAXRARE
        uint32_t maxRare = rare[NUMRARE * VECLEN - 1];  // highest from rare vector(s)
        // NOTE: if clause should be flow controlled; else clause should be branchless
        if (expected(maxFreq > maxRare)) {  // if all of Rare was already checked
            nextRare = rare + NUMRARE * VECLEN;         // jump to the next set of rare
        } else 
#endif
            {         // consider individually and reload (possibly unaligned)
                nextRare = rare;

#define ADVANCE_RARE_BY_INTS(num) {                                     \
                    int advance = 0;                                    \
                    uint32_t minRare = rare[num];                       \
                    if (maxFreq > minRare) {                            \
                        advance = GRANRARE;                             \
                    }                                                   \
                    nextRare += advance;                                \
                }                                                           

#ifndef NOMAXRARE
                // NOTE: no need to recheck maxRare if tested above         
                const uint32_t times = (NUMRARE * VECLEN / GRANRARE) - 1;
#else 
                const uint32_t times = (NUMRARE * VECLEN / GRANRARE);
#endif 
                // PROFILE: verify that these are conditional moves
                REPEAT_INCREMENT(ADVANCE_RARE_BY_INTS, times, 0, GRANRARE);
            }
        
        if (unexpected(nextRare >= stopRare)) {
            lastPass = 1;
            // FIXME: choose a safe suitable minRare here
        } else {
            minRare = nextRare[0];
        }

#if EARLYCYCLES > 0
        COMPILER_ASSERT(EARLYCYCLES < NUMFREQ);
        // CHECK_VECTOR(0) ... CHECK_VECTOR(EARLYCYCLES-1)
        REPEAT_ADDING_ONE(CHECK_VECTOR, EARLYCYCLES, 0);
#endif


#define ADVANCE_FREQ_BY_VECS(num)  {                                    \
            int advance = 0;                                            \
            if (freq[num * VECLEN - 1] < minRare) {                     \
                advance = GRANFREQ * VECLEN;                            \
            }                                                           \
            nextFreq += advance;                                        \
        }                                                                   

        // NOTE: LOOKAHEAD tries to skip over more of freq than was checked
        const uint32_t times = (NUMFREQ / GRANFREQ) + LOOKAHEAD;
        // PROFILE: verify that conditional moves are used
        REPEAT_INCREMENT(ADVANCE_FREQ_BY_VECS, times, GRANFREQ, GRANFREQ);

        // NOTE: NextFreq and NextRare will be loaded in last pass of macros below
        if (unexpected(nextFreq >= stopFreq)) {
            lastPass = 1;
        }

#define CYCLE_RARE_VECLEN CYCLE_RARE_ ## VECLEN

        // PROFILE: check that 'if' clauses are static and optimized out        
#define CYCLE_RARE(rarenum, freqnum)                                \
        Rare = NextRare;                                            \
        if (rarenum + 1 < NUMRARE) {                                \
            VLOAD(NextRare, rare, num + 1);                         \
        } else if (freqnum + 1 < NUMFREQ) {                         \
            VLOAD(NextRare, rare, 0);                               \
        } else if (! lastPass) {                                    \
            VLOAD(NextRare, nextRare, 0);                           \
        }                                                           \
        CYCLE_RARE_VECLEN(rarenum);                                 \
        int bits = _mm_movemask_ps((VECFLOAT) Match);               \
        matchBits |= bits << num

#define CYCLE_RARE_4(num)                       \
        VSHUF(F0, Freq, 0);                     \
                                                \
        VSHUF(F1, Freq, 1);                     \
        VMATCH(F0, Rare);                       \
                                                \
        VSHUF(F2, Freq, 2);                     \
        VMATCH(F1, Rare);                       \
        VOR(Match, F0);                         \
                                                \
        VSHUF(F3, Freq, 3);                     \
        VMATCH(F2, Rare);                       \
        VOR(Match, F1);                         \
                                                \
        VMATCH(F3, Rare);                       \
        VOR(Match, F2);                         \
                                                \
        VOR(Match, F3);                                   

#define CYCLE_RARE_8(num)                       \
        CYCLE_RARE_4(num);                      \
        CYCLE_RARE_4(num);                      


#define CHECK_VECTOR(freqnum)                                    \
        Freq = NextFreq;                                         \
        if (freqnum + 1 < NUMFREQ) {                             \
            VLOAD(NextFreq, freq, freqnum);                      \
        } else if (! lastPass) {                                 \
            VLOAD(NextFreq, nextFreq, 0);                        \
        }                                                        \
        REPEAT_ADDING_ONE(CYCLE_RARE, NUMRARE, 0, freqnum);      \


#if EARLYCYCLES > 0
        // CHECK_VECTOR(EARLYCYCLES) ... CHECK_VECTOR(NUMFREQ-1)
        REPEAT_ADDING_ONE(CHECK_VECTOR, NUMFREQ - EARLYCYCLES, EARLYCYCLES);
#else
        // CHECK_VECTOR(0) ... CHECK_VECTOR(NUMFREQ-1)
        REPEAT_ADDING_ONE(CHECK_VECTOR, NUMFREQ, 0);
#endif

        freq = nextFreq;
        rare = nextRare;

        uint64_t passCount = _mm_popcnt_u64(matchBits);
        count += passCount;
    }

 FINISH_SCALAR:
    return count; // plus scalar

#undef CYCLE_RARE
#undef CYCLE_RARE_VECLEN
#undef CYCLE_RARE_4
#undef CYCLE_RARE_8
#undef CHECK_VECTOR
}

