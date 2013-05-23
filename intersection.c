#include <stdint.h>
#include <stddef.h>
#include <strings.h>

#ifdef IACA
#include </opt/intel/iaca-lin32/include/iacaMarks.h>
#endif  

#include <immintrin.h>
#include <smmintrin.h>


// May help choose order of branches, but mostly comments to reader
// NOTE: for icc, seems like these prevent use of conditional moves
#define usually(x)    __builtin_expect((x),1)
#define likely(x)     __builtin_expect((x),1)
#define often(x)     __builtin_expect((x),1)
#define sometimes(x)  __builtin_expect((x),0)
#define unlikely(x)   __builtin_expect((x),0)
#define rarely(x)     __builtin_expect((x),0)

#include <stdlib.h>
#include <stdio.h>

#define VECTYPE __m128i
#define NUMVECS 8

#define CHUNKINTS (NUMVECS * (sizeof(VECTYPE)/sizeof(uint32_t))) 

#ifdef MAX2
#define FREQSPACE ((2 * CHUNKINTS) - 1)
#else
#define FREQSPACE ((1 * CHUNKINTS) - 1)
#endif // MAX2

#define COMPILER_BARRIER asm volatile("" ::: "memory");

#define ASSERT(x) // do nothing

#define MATCH(destreg, matchreg) _mm_cmpeq_epi32(destreg, matchreg)
#define OR(destreg, otherreg)  _mm_or_si128(destreg, otherreg)
#define LOAD(ptr) _mm_load_si128(ptr)
#define TESTZERO(reg) _mm_testz_si128(reg, reg)

// #define SETALL(reg, int32)  reg = _mm_cvtsi32_si128(int32)
#define SETALL(reg, int32)  reg = _mm_set1_epi32(int32);

#define LOAD_CHUNK(prefix, input)       \
    prefix##0 = LOAD((VECTYPE *)input + 0); \
    prefix##1 = LOAD((VECTYPE *)input + 1); \
    prefix##2 = LOAD((VECTYPE *)input + 2); \
    prefix##3 = LOAD((VECTYPE *)input + 3); \
    prefix##4 = LOAD((VECTYPE *)input + 4); \
    prefix##5 = LOAD((VECTYPE *)input + 5); \
    prefix##6 = LOAD((VECTYPE *)input + 6); \
    prefix##7 = LOAD((VECTYPE *)input + 7); 


size_t finish_scalar(const uint32_t *A, const size_t lenA,
                     const uint32_t *B, const size_t lenB) {

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

size_t search_chunks(const uint32_t *freq, const size_t lenFreq,
                     const uint32_t *rare, const size_t lenRare) {
    
    if (lenFreq == 0 || lenRare == 0) {
        return 0;
    }

    const uint32_t *lastRare = &rare[lenRare];
    const uint32_t *lastFreq = &rare[lenRare];
    const uint32_t scalarTailFreq = lenFreq % CHUNKINTS;  // convenience var
    const uint32_t *stopFreq = lastFreq - scalarTailFreq - FREQSPACE;

    // FUTURE: avoid this duplication while considering match might start at 0?
    uint32_t match, nextMatch;
    VECTYPE Match, NextMatch;
    size_t count = 0;

#if (MAXCHUNK >= 1)
        uint32_t maxChunk1;
#if (MAXCHUNK >= 2)
        uint32_t maxChunk2;
#endif // MAXCHUNK >= 2
#endif // MAXCHUNK >= 1

    match = *rare;
    SETALL(Match, match);
    if (usually(rare < lastRare)) { 
        nextMatch = rare[1];  
        SETALL(NextMatch, nextMatch);
    }

    goto FIRST_TIME; 

    while (usually(rare <= lastRare)) {  
        // NOTE: Compilation affected if declarations are moved outside loop
        VECTYPE M0, M1, M2, M3, M4, M5, M6, M7;
        VECTYPE Q0, Q1, Q2, Q3; // quarters
        VECTYPE S0, S1; // semis
        VECTYPE F0; // final

        match = nextMatch;
        Match = NextMatch;
        if (usually(rare < lastRare)) { // PROFILE: verify combined compare
            nextMatch = rare[1];  // PROFILE: probably best if not cmov
            SETALL(NextMatch, nextMatch);
        }

        // NOTE: the correct vectors usually are already reloaded 
        //       this loop is the fallback if they are not (or first time)
        if (rarely(maxChunk1 < match)) {  
            do {
                freq += CHUNKINTS;  
            FIRST_TIME: // Load initial vectors here first time through
                if (freq >= stopFreq) {
                    const uint32_t lenFreq = lastFreq - freq + 1;
                    const uint32_t lenRare = lastRare - rare + 1;
                    return count + 
                        finish_scalar(freq, lenFreq, rare, lenRare);
                }
                maxChunk1 = freq[1 * CHUNKINTS - 1];
            }  while (maxChunk1 < match);
#if (MAXCHUNK >= 2)
            maxChunk2 = freq[2 * CHUNKINTS - 1]; 
#endif // MAXCHUNK >= 2
            LOAD_CHUNK(M, freq);
        }

        rare += 1;   // Delayed for simpler math in previous block

#if (MAXCHUNK >= 1)
        uint32_t jump = 0;  // help compiler see the cmov
        if (sometimes(nextMatch > maxChunk1)) { // PROFILE: verify cmov
            jump = CHUNKINTS;  
        }        
        freq += jump;

#if (MAXCHUNK >= 2)
        jump = 0;
        if (unlikely(nextMatch > maxChunk2)) { // PROFILE: verify cmov
            jump = CHUNKINTS;  
        }        
        freq += jump;
#endif // MAXCHUNK >= 2
#endif // MAXCHUNK >= 1

        COMPILER_BARRIER;

        M0 = _mm_cmpeq_epi32(M0, Match);
        M1 = _mm_cmpeq_epi32(M1, Match);
        Q0 = _mm_or_si128(M0, M1);
#if (MAXCHUNK >= 1)
        maxChunk1 = freq[1 * CHUNKINTS - 1];
#if (MAXCHUNK >= 2)
        maxChunk2 = freq[2 * CHUNKINTS - 1]; 
#endif // MAXCHUNK >= 2
#endif // MAXCHUNK >= 1

        M2 = _mm_cmpeq_epi32(M2, Match);
        M3 = _mm_cmpeq_epi32(M3, Match);
        Q1 = _mm_or_si128(M2, M3);
        M0 = _mm_load_si128((VECTYPE *) freq + 0);
        M1 = _mm_load_si128((VECTYPE *) freq + 1);

        M4 = _mm_cmpeq_epi32(M4, Match);
        M5 = _mm_cmpeq_epi32(M5, Match);
        Q2 = _mm_or_si128(M4, M5);
        M2 = _mm_load_si128((VECTYPE *) freq + 2);
        M3 = _mm_load_si128((VECTYPE *) freq + 3);

        M6 = _mm_cmpeq_epi32(M6, Match);
        M7 = _mm_cmpeq_epi32(M7, Match);
        Q3 = _mm_or_si128(M6, M7);
        M4 = _mm_load_si128((VECTYPE *) freq + 4);
        M5 = _mm_load_si128((VECTYPE *) freq + 5);

        S0 = _mm_or_si128(Q0, Q1);
        S1 = _mm_or_si128(Q2, Q3);
        F0 = _mm_or_si128(S0, S1);
        M6 = _mm_load_si128((VECTYPE *) freq + 6);
        M7 = _mm_load_si128((VECTYPE *) freq + 7);

        if (! _mm_testz_si128(F0, F0)) {
            count += 1;             // PROFILE: verify cmov
        }
    }

    ASSERT(rare = lastRare + 1);
    return count; // no scalar finish since reached only after lastRare done
}
  
