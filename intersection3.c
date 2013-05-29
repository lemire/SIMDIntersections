#ifdef TODO
// Make more explicitly a template file (split off, multiple includes with different params)

// Change finish_scalar() to be lopsided for freq vs rare (figure out external linkage?)

// Submit Intel bug and figure out workarounds for more vectors.

// Use MM0-MM7 for LOOKAHEADS to avoid register pressure on general registers
// Add higher numbers of (virtual) vectors: 24, 32 (performance test first)

// Switch "VECTYPE X = M0" to assembly compatible macro. (skip it?)

// Improve TESTZERO to allow better load interleaves?  (Probably not needed)

// Adjust search split based on number of LOOKAHEADS (only after performance testing)

// Add support for different size vectors (Keep planning for, but do later)

// Utilize save and load from stack to save register-register moves (low priority, unhelpful IB+) 
// Create assembly.S template also  (No, seems possible to stick with C and macros)
#endif // TODO

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

// #define MATCH(destreg, matchreg) _mm_cmpeq_epi32(destreg, matchreg)
// #define OR(destreg, otherreg)  _mm_or_si128(destreg, otherreg)
// #define LOAD(ptr) _mm_load_si128(ptr)
#define TESTZERO(reg) _mm_testz_si128(reg, reg)
// #define SETALL(int32)  _mm_set1_epi32(int32)

// Syntax examples:
//  __asm__ __volatile__ ("psllq %0, %0" : "=x" (vec.full128b) : "x"  (vec.full128b));
// asm volatile("movdqu %0, %%xmm0" :  : "m" (xmm_reg) : "%xmm0"  );

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

// NOTE: Using "asm volatile" required to keep desired ordering
//       Remove to allow compiler to reschedule operations
#define VOLATILE volatile

// FIXME: icc 13.0.1 generates non-working nonsense for NUMVECS=16 if there are 15 assigned registers

// FIXME: icc generates bad code for NUMVECS=16 with unassigned registers
// FIXME: gcc 4.7 spills a register for NUMVECS=16 unless registers are assigned
// FIXME: gcc generates extra moves unless registers are assigned
// #if defined(__ICC) || defined(__INTEL_COMPILER)
// #define REGISTER(reg) 
#define REGISTER(reg) asm(reg)

// FIXME: Technically incorrect, and produces bad code for icc 13.0.1
// #define VOR(dest, other) asm VOLATILE("por %1, %0" : "=x" (dest) : "x" (other) );
// Correct, synonymous with "+x"
// #define VOR(dest, other) asm VOLATILE("por %1, %0" : "=x" (dest) : "x" (other), "0" (dest) );

//  +x since it's read, but this confuses GCC into adding extra moves unless registers assigned
#define VOR(dest, other)                                      \
    asm VOLATILE("por %1, %0" : "+x" (dest) : "x" (other) );

#define VLOAD(dest, ptr, offset)                                        \
    asm VOLATILE("movdqu %c2(%1), %0" : "=x" (dest) : "g" (ptr), "i" (offset * sizeof(VECTYPE)) );

#define VSETALL(dest, int32)                                            \
    asm VOLATILE("movd %1, %0; pshufd $0, %0, %0" : "=x" (dest) : "g" (int32) ) 

// #define VMATCH(dest, other) dest = _mm_cmpeq_epi32(dest, other)
#define VMATCH(dest, other)                                   \
    asm VOLATILE("pcmpeqd %1, %0" : "+x" (dest) : "x" (other) )

// NOTE: doesn't seem needed, can just use "dest = src"?
// #define VMOVE(dest, src) asm VOLATILE("movdqa %1, %0" : "=x" (dest) : "x" (src) );

size_t search_chunks(const uint32_t *freq, size_t lenFreq,
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

    uint32_t maxChunk = freq[CHUNKINTS - 1];
    uint32_t nextMatch = *rare;

    while (sometimes(maxChunk < nextMatch)) {
    RELOAD_FREQ:
        if (rarely(freq + CHUNKINTS >= stopFreq)) {
            goto FINISH_SCALAR;
        }
        freq += CHUNKINTS;
        maxChunk = freq[CHUNKINTS - 1];
    }

    register VECTYPE M0 REGISTER("xmm0");
    VLOAD(M0, freq, 0); 
#if NUMVECS > 1 
    register VECTYPE M1 REGISTER("xmm1");
    VLOAD(M1, freq, 1); 
#if NUMVECS > 2 
    register VECTYPE M2 REGISTER("xmm2");
    VLOAD(M2, freq, 2); 
    register VECTYPE M3 REGISTER("xmm3");
    VLOAD(M3, freq,  3); 
#if NUMVECS > 4 
    register VECTYPE M4 REGISTER("xmm4");
    VLOAD(M4, freq, 4); 
    register VECTYPE M5 REGISTER("xmm5");
    VLOAD(M5, freq, 5); 
    register VECTYPE M6 REGISTER("xmm6");
    VLOAD(M6, freq, 6); 
    register VECTYPE M7 REGISTER("xmm7");
    VLOAD(M7, freq, 7); 
#if NUMVECS > 8
    register VECTYPE M8 REGISTER("xmm8");
    VLOAD(M8, freq, 8); 
    register VECTYPE M9 REGISTER("xmm9");
    VLOAD(M9, freq, 9); 
    register VECTYPE M10 REGISTER("xmm10");
    VLOAD(M10, freq, 10); 
    register VECTYPE M11 REGISTER("xmm11");
    VLOAD(M11, freq, 11); 
#if NUMVECS > 12  
    register VECTYPE M12 REGISTER("xmm12");
    VLOAD(M12, freq, 12); 
    // NOTE: further vectors are reloaded
#endif // > 12 
#endif // > 8
#endif // > 4
#endif // > 2
#endif // > 1

// FUTURE: use MM0-MM7 for extra storage space beyond spill point (more than 6?)
        
#if (LOOKAHEAD >= 2)
    uint32_t maxChunk2 = freq[2 * CHUNKINTS - 1]; 
#if (LOOKAHEAD >= 3)
    uint32_t maxChunk3 = freq[3 * CHUNKINTS - 1]; 
#if (LOOKAHEAD >= 4)
    uint32_t maxChunk4 = freq[4 * CHUNKINTS - 1]; 
#if (LOOKAHEAD >= 5)
    uint32_t maxChunk5 = freq[5 * CHUNKINTS - 1]; 
#if (LOOKAHEAD >= 6)
    uint32_t maxChunk6 = freq[6 * CHUNKINTS - 1]; 
#if (LOOKAHEAD >= 7)
    uint32_t maxChunk7 = freq[7 * CHUNKINTS - 1]; 
#if (LOOKAHEAD >= 8)
    uint32_t maxChunk8 = freq[8 * CHUNKINTS - 1]; 
#endif // 8
#endif // 7
#endif // 6
#endif // 5
#endif // 4
#endif // 3
#endif // 2


#ifdef IACA
    IACA_START;
#endif
    register VECTYPE Match REGISTER("xmm15");
 CHECK_NEXT_RARE:
    VSETALL(Match, nextMatch);
    ASSERT(maxChunk >= nextMatch);
    if (usually(rare < lastRare)) { 
        nextMatch = rare[1]; // CAUTION: only can load nextMatch after checking rare < lastRare
    }
    // NOTE: safe to leave nextMatch set to *rare on last iteration
    // NOTE: Four cycles until nextMatch is loaded for LOOKAHEAD comparisons

    COMPILER_BARRIER;

#if NUMVECS == 1
    register VECTYPE X REGISTER("xmm14");
    VMATCH(M0, Match);  
    X = M0;
#elif NUMVECS == 2
    register VECTYPE X REGISTER("xmm14");
    VMATCH(M0, Match);
    VMATCH(M1, Match); 

    VOR(M1, M0);
    X = M1;
#elif NUMVECS == 4
    register VECTYPE X REGISTER("xmm14");
    VMATCH(M0, Match);
    VMATCH(M1, Match); 

    VOR(M1, M0);
    VMATCH(M2, Match);
    VMATCH(M3, Match);

    VOR(M3, M2);

    VOR(M3, M1);
    X = M3;
#elif NUMVECS == 8 || NUMVECS == 12 
    register VECTYPE X REGISTER("xmm14");
    VMATCH(M0, Match);
    VMATCH(M1, Match); 

    VOR(M1, M0);
    VMATCH(M2, Match);
    VMATCH(M3, Match);

    VOR(M3, M2);

    VOR(M3, M1);
    X = M3;
#elif NUMVECS >= 16
    VMATCH(M0, Match);
    VMATCH(M2, Match); 

    VMATCH(M1, Match);
    VMATCH(M3, Match);
    VOR(M2, M0);
    // NOTE: M0, M1, M2 continue loading from "this" freq instead of reloading for nextFreq
    VLOAD(M0, freq, 13);   // 0 doubles as 13 

    VMATCH(M4, Match);
    VMATCH(M5, Match);
    VOR(M3, M1);
    VLOAD(M1, freq, 14);   // 1 doubles as 14

    VOR(M3, M2);
    VLOAD(M2, freq, 15);   // 2 doubles as 15
#endif

    COMPILER_BARRIER;

    // PROFILE: faster to swap nextFreq calculations and first half of match?
    const uint32_t *nextFreq = freq; // location to reload next set vectors (possibly unchanged)

#if (LOOKAHEAD >= 1)
    uint32_t jump = 0;  // convince compiler we really want a cmov
    if (sometimes(nextMatch > maxChunk)) { // PROFILE: verify cmov
        jump = CHUNKINTS;
    }        
    nextFreq += jump;
#endif // LOOKAHEAD >= 1

#if (LOOKAHEAD >= 2)
    jump = 0;
    if (unlikely(nextMatch > maxChunk2)) { // PROFILE: verify cmov
        jump = CHUNKINTS;  
    }        
    nextFreq += jump;
#endif // LOOKAHEAD >= 2

#if (LOOKAHEAD >= 3)
    jump = 0;
    if (rarely(nextMatch > maxChunk3)) { // PROFILE: verify cmov
        jump = CHUNKINTS;  
    }        
    nextFreq += jump;
#endif // LOOKAHEAD >= 3

#if (LOOKAHEAD >= 4)
    jump = 0;
    if (rarely(nextMatch > maxChunk4)) { // PROFILE: verify cmov
        jump = CHUNKINTS;  
    }        
    nextFreq += jump;
#endif // LOOKAHEAD >= 4

#if (LOOKAHEAD >= 5)
    jump = 0;
    if (rarely(nextMatch > maxChunk5)) { // PROFILE: verify cmov
        jump = CHUNKINTS;  
    }        
    nextFreq += jump;
#endif // LOOKAHEAD >= 5

#if (LOOKAHEAD >= 6)
    jump = 0;
    if (rarely(nextMatch > maxChunk6)) { // PROFILE: verify cmov
        jump = CHUNKINTS;  
    }        
    nextFreq += jump;
#endif // LOOKAHEAD >= 6

#if (LOOKAHEAD >= 7)
    jump = 0;
    if (rarely(nextMatch > maxChunk7)) { // PROFILE: verify cmov
        jump = CHUNKINTS;  
    }        
    nextFreq += jump;
#endif // LOOKAHEAD >= 7

#if (LOOKAHEAD >= 8)
    jump = 0;
    if (rarely(nextMatch > maxChunk8)) { // PROFILE: verify cmov
        jump = CHUNKINTS;  
    }        
    nextFreq += jump;
#endif // LOOKAHEAD >= 8

    // CAUTION: No nextFreq reloads before here (waiting on LOOKAHEAD to calculate nextFreq)

#if NUMVECS == 1
    if (! TESTZERO(X)) {
        count += 1;
    }
    VLOAD(M0, nextFreq, 0);
#elif NUMVECS == 2
    if (! TESTZERO(X)) {
        count += 1;
    }
    VLOAD(M0, nextFreq, 0);
    VLOAD(M1, nextFreq, 1);
#elif NUMVECS == 4
    VLOAD(M0, nextFreq, 0);
    VLOAD(M1, nextFreq, 1);
    VLOAD(M2, nextFreq, 2);
    VLOAD(M3, nextFreq, 3);
    if (! TESTZERO(X)) {
        count += 1;
    }
#elif NUMVECS == 8
    VMATCH(M4, Match);
    VMATCH(M5, Match); 
    VLOAD(M0, nextFreq, 0);
    VLOAD(M1, nextFreq, 1);

    VMATCH(M6, Match);
    VMATCH(M7, Match);
    VOR(M5, M4);
    VLOAD(M2, nextFreq, 2);
    VLOAD(M3, nextFreq, 3);

    VOR(M7, M6);
    VOR(X, M5);
    VLOAD(M4, nextFreq, 4);
    VLOAD(M5, nextFreq, 5);

    VOR(X, M7);
    VLOAD(M6, nextFreq, 6);
    VLOAD(M7, nextFreq, 7);

    if (! TESTZERO(X)) {
        count += 1;             // PROFILE: verify cmov
    }

#elif NUMVECS == 12
    VMATCH(M4, Match);
    VMATCH(M5, Match); 
    VLOAD(M0, nextFreq, 0);
    VLOAD(M1, nextFreq, 1);

    VMATCH(M6, Match);
    VMATCH(M7, Match);
    VOR(M5, M4);
    VLOAD(M2, nextFreq, 2);
    VLOAD(M3, nextFreq, 3);

    VMATCH(M8, Match);
    VMATCH(M9, Match);
    VOR(M7, M6);
    VLOAD(M4, nextFreq, 4);

    VMATCH(M10, Match);
    VMATCH(M11, Match);
    VOR(X, M5);
    VLOAD(M5, nextFreq, 5);
    VLOAD(M6, nextFreq, 6);

    VOR(M9, M8);
    VOR(M11, M10);
    VOR(X, M7);
    VLOAD(M7, nextFreq, 7);
    VLOAD(M8, nextFreq, 8);

    VOR(M11, M9);
    VLOAD(M9, nextFreq, 9);
    VLOAD(M10, nextFreq, 10);

    VOR(X, M11);
    VLOAD(M7, nextFreq, 7);
    VLOAD(M10, nextFreq, 10);

    VLOAD(M11, nextFreq, 11);
    if (! TESTZERO(X)) {
        count += 1;             // PROFILE: verify cmov
    }
#elif NUMVECS == 16
    VMATCH(M6, Match);
    VMATCH(M7, Match);
    VOR(M5, M4);
    VLOAD(M4, nextFreq, 4);

    VMATCH(M8, Match);
    VMATCH(M9, Match);
    VOR(M7, M6);
    VLOAD(M6, nextFreq, 6);

    VMATCH(M10, Match);
    VMATCH(M11, Match);
    VOR(M9, M8);
    VLOAD(M8, nextFreq, 8);

    VMATCH(M12, Match);
    VOR(M2, M0);   // OR("15", "13")
    VOR(M3, M1);  // OR(0|1|2|3, "14")
    VLOAD(M0, nextFreq, 0);  
    VLOAD(M1, nextFreq, 1);

    VOR(M7, M2); 
    VOR(M9, M3); // OR(9, 0|1|2|3|14)
    VOR(M11, M10);
    VLOAD(M2, nextFreq, 2);
    VLOAD(M3, nextFreq, 3);

    VOR(M11, M5);
    VOR(M12, M7);
    VLOAD(M5, nextFreq, 5);
    VLOAD(M7, nextFreq, 7);

    VOR(M11, M9);
    VLOAD(M9, nextFreq, 9);
    VLOAD(M10, nextFreq, 10);

    VOR(M12, M11);
    VLOAD(M11, nextFreq, 11);

    if (! TESTZERO(M12)) {
        count += 1;
    }
    VLOAD(M12, nextFreq, 12);

#elif NUMVECS == 24
    VMATCH(M6, Match);
    VMATCH(M7, Match);
    VOR(M5, M3);
    VLOAD(M3, freq, 16);

    VMATCH(M8, Match);
    VMATCH(M9, Match);
    VOR(M6, M4);
    VLOAD(M4, freq, 17);

    VMATCH(M10, Match);
    VMATCH(M11, Match);
    VOR(M6, M5);
    VLOAD(M5, freq, 18);

    VMATCH(M12, Match);
    VOR(M0, M6);
    VOR(M1, M7);
    VLOAD(M6, freq, 19);
    VLOAD(M7, freq, 20);

    VMATCH(M3, Match);
    VOR(M10, M8);
    VOR(M11, M9);
    VLOAD(M8, freq, 21);
    VLOAD(M9, freq, 22);

    VMATCH(M4, Match);
    VOR(M1, M0);
    VOR(M11, M10);
    VLOAD(M10, freq, 23);
    VLOAD(M0, nextFreq, 0);  

    VMATCH(M5, Match);
    VOR(M3, M1);
    VOR(M11, M2); 
    VLOAD(M1, nextFreq, 1);  
    VLOAD(M2, nextFreq, 2);  

    VMATCH(M6, Match);
    VOR(M11, M3);
    VOR(M5, M4);
    VLOAD(M3, nextFreq, 3);  
    VLOAD(M4, nextFreq, 4);  

    VMATCH(M7, Match);
    VOR(M11, M5);
    VOR(M12, M6);
    VLOAD(M5, nextFreq, 5);  
    VLOAD(M6, nextFreq, 6);  
    
    VMATCH(M8, Match);
    VMATCH(M9, Match);
    VOR(M12, M7);
    VLOAD(M7, nextFreq, 7);

    VMATCH(M10, Match);
    VOR(M11, M8);
    VOR(M12, M9);
    VLOAD(M8, nextFreq, 8);
    VLOAD(M9, nextFreq, 9);
    
    VOR(M12, M10);
    VLOAD(M10, nextFreq, 10);

    VOR(M12, M11);
    VLOAD(M11, nextFreq, 11);

    if (! TESTZERO(M12)) {
        count += 1;
    }
    VLOAD(M12, nextFreq, 12);
#endif

    COMPILER_BARRIER;

    // NOTE: nextFreq is needed to reload vectors below but 
    //       maxChunks aren't needed until the next iteration
    maxChunk = nextFreq[CHUNKINTS - 1];
#if (LOOKAHEAD >= 2)
    maxChunk2 = nextFreq[2 * CHUNKINTS - 1]; 
#endif // 2

#if (LOOKAHEAD >= 3)
    maxChunk3 = nextFreq[3 * CHUNKINTS - 1]; 
#endif // 3

#if (LOOKAHEAD >= 4)
    maxChunk4 = nextFreq[4 * CHUNKINTS - 1]; 
#endif // 4

#if (LOOKAHEAD >= 5)
    maxChunk5 = nextFreq[5 * CHUNKINTS - 1]; 
#endif // 5

#if (LOOKAHEAD >= 6)
    maxChunk6 = nextFreq[6 * CHUNKINTS - 1]; 
#endif // 6

#if (LOOKAHEAD >= 7)
    maxChunk7 = nextFreq[7 * CHUNKINTS - 1]; 
#endif // 7

#if (LOOKAHEAD >= 8)
    maxChunk8 = nextFreq[8 * CHUNKINTS - 1]; 
#endif // 8


    // NOTE: Freq is finalized and best effort has been made so that vectors are reloaded.
    //       if maxChunk >= nextMatch, they are loaded correctly.  If not, reload freq.
    freq = nextFreq;  // PROFILE: verify that this isn't causing a register to spill

    // completely done if we have already checked lastRare
    if (rarely(rare >= lastRare)) {
        ASSERT(rare == lastRare);
        return count; // no scalar finish after lastRare done
    }
    rare += 1;  

    if (rarely(freq >= stopFreq)) {
        // FUTURE: try to add one more pass for preloaded vectors?
        goto FINISH_SCALAR;
    }        

    if (usually(maxChunk >= nextMatch)) {
        goto CHECK_NEXT_RARE; // preload worked and vectors are ready
    } 


#ifdef IACA
        IACA_END;
#endif

    goto RELOAD_FREQ;  // need to scan farther along in freq 
       
    
 FINISH_SCALAR:
    lenFreq = lastFreq - freq + 1;
    lenRare = lastRare - rare + 1;
    return count + finish_scalar(freq, lenFreq, rare, lenRare);
}


#ifdef NOTES

PSHUFD 0|5 1
P(AND|OR)D  0|1|5 1
PSL(L|R)D 1  1
PCMP(EQ|GT)D 0|5 1
PMOVMSKB  0 2
PMOVMSKPS 0 2
POPCNT 1 3
PEXTRD 2*(0|1|5) 2
BS(R|F) 0|1|5 3
CMP 0|1|5 1

// PROFILE: ASSUME 16 cycle penalty for misprediction

.25 cycle/int = 10Bint/s  (16Bint/s theoretical at 4GHz);
.5 cycle/int = 5Bint/s    (8.0Bint/s)
1 cycle/int = 2.5Bint/s   (4.0Bint/s);
2 cycle/int = 1.2Bint/s   (2.0Bint/s);
3 cycle/int = 0.8Bint/s   (1.3Bint/s);
4 cycle/int = 0.6Bint/s   (1.0Bint/s);


#endif // NOTES

  
