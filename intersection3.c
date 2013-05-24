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
#define SETALL(int32)  _mm_set1_epi32(int32);



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
    
    size_t count = 0;
    if (lenFreq == 0 || lenRare == 0) {
        return 0;
    }

    const uint32_t *lastRare = &rare[lenRare];
    const uint32_t *lastFreq = &rare[lenRare];
    const uint32_t *stopFreq = freq + lenFreq - FREQSPACE;
    
    uint32_t nextMatch = *rare;
    VECTYPE NextMatch = SETALL(nextMatch);
    goto CHECK_FREQ;

    do {
        ASSERT(maxChunk >= nextMatch);

        VECTYPE Q0, Q1, Q2, Q3; // quarters
        VECTYPE S0, S1; // semis
        VECTYPE F0; // final

        VECTYPE Match = NextMatch;
        if (usually(rare < lastRare)) { // PROFILE: verify compare is shared
            nextMatch = rare[1];      // with (rare <= last) in loop header
            NextMatch = SETALL(nextMatch);
        }
        rare += 1;   


#if (MAXCHUNK >= 1)
        uint32_t jump = 0;  // help compiler see the cmov
        if (sometimes(nextMatch > maxChunk)) { // PROFILE: verify cmov
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
        maxChunk = freq[CHUNKINTS - 1];
#if (MAXCHUNK >= 2)
        maxChunk2 = freq[2 * CHUNKINTS - 1]; 
#endif // MAXCHUNK >= 2

        COMPILER_BARRIER;

        M2 = _mm_cmpeq_epi32(M2, Match);
        M3 = _mm_cmpeq_epi32(M3, Match);
        Q1 = _mm_or_si128(M2, M3);
        M0 = _mm_load_si128((VECTYPE *) freq + 0);
        M1 = _mm_load_si128((VECTYPE *) freq + 1);

        COMPILER_BARRIER;

        M4 = _mm_cmpeq_epi32(M4, Match);
        M5 = _mm_cmpeq_epi32(M5, Match); 
        Q2 = _mm_or_si128(M4, M5);
        M2 = _mm_load_si128((VECTYPE *) freq + 2);
        M3 = _mm_load_si128((VECTYPE *) freq + 3);

        COMPILER_BARRIER;

        M6 = _mm_cmpeq_epi32(M6, Match);
        M7 = _mm_cmpeq_epi32(M7, Match);
        Q3 = _mm_or_si128(M6, M7);
        M4 = _mm_load_si128((VECTYPE *) freq + 4);
        M5 = _mm_load_si128((VECTYPE *) freq + 5);

        COMPILER_BARRIER;

        S0 = _mm_or_si128(Q0, Q1);
        S1 = _mm_or_si128(Q2, Q3);
        F0 = _mm_or_si128(S0, S1);
        M6 = _mm_load_si128((VECTYPE *) freq + 6);
        M7 = _mm_load_si128((VECTYPE *) freq + 7);

        COMPILER_BARRIER;

        if (! _mm_testz_si128(F0, F0)) {
            count += 1;             // PROFILE: verify cmov
        }

        // Exit if we have already checked lastRare
        if (rarely(rare > lastRare)) {
            ASSERT(rare = lastRare + 1);
            return count; // no scalar finish after lastRare done
        }

        
    CHECK_FREQ:
        if (rarely(freq >= stopFreq)) {
            const uint32_t lenFreq = lastFreq - freq + 1;
            const uint32_t lenRare = lastRare - rare + 1;
            return count + finish_scalar(freq, lenFreq, rare, lenRare);
        }

        uint32_t maxChunk = freq[CHUNKINTS - 1];

        if (rarely(maxChunk < nextMatch)) {
            freq += CHUNKINTS;
            goto CHECK_FREQ;
        }

        // FIXME:  need load to happen iff loop was taken
        VECTYPE M0 = LOAD((VECTYPE *)freq + 0); 
        VECTYPE M1 = LOAD((VECTYPE *)freq + 1); 
        VECTYPE M2 = LOAD((VECTYPE *)freq + 2); 
        VECTYPE M3 = LOAD((VECTYPE *)freq + 3); 
        VECTYPE M4 = LOAD((VECTYPE *)freq + 4); 
        VECTYPE M5 = LOAD((VECTYPE *)freq + 5); 
        VECTYPE M6 = LOAD((VECTYPE *)freq + 6); 
        VECTYPE M7 = LOAD((VECTYPE *)freq + 7); 
        
#if (MAXCHUNK >= 2)
        uint32_t maxChunk2 = freq[2 * CHUNKINTS - 1]; 
#endif // MAXCHUNK >= 2
        }

    }
    // NOTREACHED
    return count;
}

  
