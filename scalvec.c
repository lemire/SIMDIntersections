#include <stdint.h>
#include "compiler.h"
#include "vec4.h"
#include "asm.h"

#include <stdio.h>

// #define DEBUG_PRINT(args...) printf(args)
#define DEBUG_PRINT(args...)

#ifdef IACA
#include </opt/intel/iaca-lin32/include/iacaMarks.h>
#endif

// #define COUNTBITS(result, mask) VEC_POPCNT(result, mask)
#define COUNTBITS(result, mask) result = kCountBits[mask];

int kCountBits[] = {
    0, //  0 0000
    1, //  1 0001 
    1, //  2 0010
    2, //  3 0011
    1, //  4 0100
    2, //  5 0101
    2, //  6 0110
    3, //  7 0111
    1, //  8 1000
    2, //  9 1001
    2, // 10 1010
    3, // 11 1011
    2, // 12 1100
    3, // 13 1101
    3, // 14 1110
    4, // 15 1111
};


#ifdef __cplusplus
extern "C" {
    size_t match_scalar(const uint32_t *freq, size_t lenFreq, 
                        const uint32_t *rare, size_t lenRare,
                        uint32_t *matchOut);



    size_t match_scalvec_v4_r1g1_f4g1(const uint32_t *nextFreq, size_t lenFreq, 
                                      const uint32_t *nextRare, size_t lenRare,
                                      uint32_t *matchOut);

    size_t match_scalvec_v4_f16(const uint32_t *freq, size_t lenFreq, 
                                const uint32_t *rare, size_t lenRare,
                                uint32_t *matchOut);

    // v4r for vector len 4 reversed args
    size_t match_scalvec_v4r_f16(const uint32_t *rare, size_t lenRare,
                                 const uint32_t *freq, size_t lenFreq, 
                                 uint32_t *matchOut)  {
        return match_scalvec_v4_f16(freq, lenFreq, rare, lenRare, matchOut);
    }
}
#endif

size_t match_scalar(const uint32_t *A, const size_t lenA,
                    const uint32_t *B, const size_t lenB, uint32_t * out) {

    const uint32_t *initout = out;
    if (lenA == 0 || lenB == 0) return 0;

    const uint32_t *endA = A + lenA;
    const uint32_t *endB = B + lenB;
    
    while (1) {
        while (*A < *B) {
        SKIP_FIRST_COMPARE: 
            if (++A == endA) return (out - initout);
        }
        while (*A > *B) {
            if (++B == endB) return (out - initout);
        }
        if (*A == *B) {
            *out++ = *A;
            if (++A == endA || ++B == endB) return (out - initout);
        } else {
            goto SKIP_FIRST_COMPARE;
        }
    }

    return (out - initout); // NOTREACHED
}


size_t match_scalvec_v4_r1g1_f4g1(const uint32_t *nextFreq, size_t lenFreq, 
                                  const uint32_t *nextRare, size_t lenRare,
                                  uint32_t *matchOut) {

    const uint32_t *matchOrig = matchOut;
    if (lenFreq == 0 || lenRare == 0) {
        return 0;
    }

    const uint32_t *lastRare = &nextRare[lenRare];
    const uint32_t *lastFreq = &nextFreq[lenFreq];

#define FREQ_SPACE (VECLEN - 1)
#define RARE_SPACE (1)

    const uint32_t *stopFreq = lastFreq - FREQ_SPACE; 
    const uint32_t *stopRare = lastRare - RARE_SPACE; 

    if (COMPILER_RARELY(nextFreq >= stopFreq) || nextRare >= stopRare) {
        goto FINISH_SCALAR;
    }

    //    ASM_REGISTER(VEC_T MatchFreq, xmm0);
    //    ASM_REGISTER(VEC_T NextFreq, xmm1);
    //    ASM_REGISTER(VEC_T MatchRare, xmm2);
    //    ASM_REGISTER(VEC_T NextRare, xmm3);
    //    ASM_REGISTER(VEC_T Advance, xmm4);

    VEC_T MatchFreq;
    VEC_T NextFreq;
    VEC_T MatchRare;
    VEC_T NextRare;

    //    ASM_REGISTER(uint64_t maxNextFreq, r11);
    uint64_t maxFreq;

    //    ASM_REGISTER(uint64_t valNextRare, r9);
    uint64_t valNextRare;

    // ASM_REGISTER(uint64_t mask, r10);
    uint64_t mask;

    //    ASM_REGISTER(uint64_t advanceOut, r12);
    uint64_t advanceOut;

    //    ASM_REGISTER(uint64_t advanceNextFreq, r13);
    uint64_t advanceNextFreq;

    //    ASM_REGISTER(uint64_t advanceNextRare, r14);
    uint64_t advanceNextRare;

    ASM_REGISTER(uint64_t one, r15);
    one = 1L;  // optimize by keeping in register

    VEC_LOAD(MatchFreq, nextFreq);
    //    DEBUG_PRINT("MatchFreq " VEC_FORMAT_DEBUG(MatchFreq));
    maxFreq = nextFreq[VECLEN - 1];  
    valNextRare = nextRare[0]; 
    //    DEBUG_PRINT("maxFreq %ld valRare %ld\n", maxFreq, valNextRare);

    VEC_SET_ALL_TO_INT(MatchRare, valNextRare);
    *matchOut = valNextRare;  // no advance yet -- overwritten if no match
    // DEBUG_PRINT("Writing %ld (%ld)...\n", valNextRare, matchOrig - matchOut);

    uint64_t valNextRarePlusOne;

    // see if rare should advance for second iteration
    if (valNextRare <= maxFreq) {
        nextRare += 1;
        valNextRare = *nextRare;
    }

    VEC_SET_ALL_TO_INT(NextRare, valNextRare);
    VEC_COPY(NextFreq, MatchFreq);

    while (1) {
        //        DEBUG_PRINT("MatchRare: " VEC_FORMAT_DEBUG(MatchRare));
        //        DEBUG_PRINT("MatchFreq: " VEC_FORMAT_DEBUG(MatchFreq));
        VEC_MATCH(MatchFreq, MatchRare);
        VEC_COPY(MatchRare, NextRare);

        // NOTE: follows nextRare in memory but not necessarily iteration
        valNextRarePlusOne = nextRare[1];
        maxFreq = nextFreq[VECLEN - 1];

        // Advance output if match should be preserved
        advanceOut = 0;
        VEC_SET_PTEST(advanceOut, one, MatchFreq);

        // Start to calculate advanceNextFreq
        VEC_CMP_GREATER(NextRare, NextFreq);
        VEC_READ_MASK(mask, NextRare);
        COUNTBITS(advanceNextFreq, mask);

        ASM_LEA_ADD_BASE_MUL(matchOut, advanceOut, sizeof(uint32_t));
        // DEBUG_PRINT("match: %ld (%ld)\n", advanceOut,  matchOut - matchOrig);
        
        VEC_COPY(MatchFreq, NextFreq);

        // FUTURE: better way to specify 32-bit register
        //        ASM_BLOCK(ASM_LINE("movl %c1(%2, %3, %c4), %0d") :    \
        //                  /* writes %0 */ "=r" (maxNextFreq):         \
        //                  /* reads %1 */ "i" (4 * (VECLEN - 1)),      \
        //                  /* reads %2 */ "r" (nextFreq),              \
        //                  /* reads %3 */ "r" (advanceNextFreq),       \
        //                  /* reads %4 */ "i" (4));    
        //        DEBUG_PRINT("mask %ld maxNextFreq %ld valNextRare %ld\n", 
        //                    mask, maxNextFreq, valNextRare);

        //  nextFreq = nextFreq + advanceFreq;  
        ASM_LEA_ADD_BASE_MUL(nextFreq, advanceNextFreq, sizeof(uint32_t));
        DEBUG_PRINT("nextFreq += %ld min %d max %ld\n", advanceNextFreq,    
                    *nextFreq, maxFreq);


        


        if (COMPILER_RARELY(nextFreq >= stopFreq)) {
            goto FINISH_SCALAR;
        }

        VEC_LOAD(NextFreq, nextFreq);  
        // DEBUG_PRINT("NextFreq " VEC_FORMAT_DEBUG(NextFreq));
        
        // Write this next potential match (may be overwritten)
        //DEBUG_PRINT("Writing %ld (%ld)\n", valNextRare, matchOut - matchOrig);
        *matchOut = valNextRare;



        // FUTURE: try to advance Rare farther with a packed compare?

        //    advanceNextRare = 0;
        //    if (valNextRare <= maxFreq) {
        //        advanceNextRare = one; 
        //        valNextRare = valNextRarePlusOne;
        //    }

        ASM_BLOCK(ASM_LINE("xor %1, %1")                          \
                  ASM_LINE("cmp %2, %0")                          \
                  ASM_LINE("cmovbe %3, %1")                       \
                  ASM_LINE("cmovbe %4, %0") :                     \
                  /* writes %0 */ "+r" (valNextRare),            \
                  /* writes %1 */ "+r" (advanceNextRare) :       \
                  /* reads %2 */ "r" (maxFreq),                  \
                  /* reads %3 */ "r" (one),                      \
                  /* reads %4 */ "r" (valNextRarePlusOne) :      \
                  /* clobbers */ "cc");
        // NOTE: "+r" for conditional moves since initial value is live

        ASM_LEA_ADD_BASE_MUL(nextRare, advanceNextRare, sizeof(uint32_t));

        if (COMPILER_RARELY(nextRare > stopRare)) {
            // Perform last match
            VEC_MATCH(MatchRare, MatchFreq);
            advanceOut = 0;
            VEC_SET_PTEST(advanceOut, one, MatchRare);
            ASM_LEA_ADD_BASE_MUL(matchOut, advanceOut, sizeof(uint32_t));
            goto FINISH_SCALAR;
        }

        DEBUG_PRINT("nextRare += %ld val %ld\n", 
                    advanceNextRare, valNextRare);


        VEC_SET_ALL_TO_INT(NextRare, valNextRare);


#ifdef IACA
        IACA_END;
#endif

        DEBUG_PRINT("NextFreq: " VEC_FORMAT_DEBUG(NextFreq));
        DEBUG_PRINT("NextRare: " VEC_FORMAT_DEBUG(NextRare));
    } 


    size_t count;
FINISH_SCALAR:
    count = matchOut - matchOrig;

    DEBUG_PRINT("Pre-tail count %zd\n", count);
    
    lenFreq = stopFreq + FREQ_SPACE - nextFreq;
    lenRare = stopRare + RARE_SPACE - nextRare;
    DEBUG_PRINT("lenFreq %ld lenRare %ld\n", lenFreq, lenRare);

    size_t tail = match_scalar(nextFreq, lenFreq, nextRare, lenRare, matchOut);

    DEBUG_PRINT("Tail count %zd\n", tail);

    return count + tail;
}

size_t match_scalvec_v4_f16(const uint32_t *freq, size_t lenFreq, 
                           const uint32_t *rare, size_t lenRare,
                           uint32_t *matchOut) {

    const uint32_t *matchOrig = matchOut;
    if (lenFreq == 0 || lenRare == 0) {
        return 0;
    }

    const uint32_t *lastRare = &rare[lenRare];
    const uint32_t *lastFreq = &freq[lenFreq];

#undef FREQ_SPACE
#undef RARE_SPACE
#define FREQ_SPACE (4 * VECLEN - 1)
#define RARE_SPACE (1)

    const uint32_t *stopFreq = lastFreq - FREQ_SPACE; 
    const uint32_t *stopRare = lastRare - RARE_SPACE; 

    register uint64_t one = 1; // convenience constant
    uint64_t valRare = *rare;

    if (COMPILER_RARELY(freq >= stopFreq) 
        || COMPILER_RARELY(rare >= stopRare)) {
        goto FINISH_SCALAR;
    }

    VEC_T F0, F1, F2, F3;
    VEC_T X1, X2, X3;
    VEC_T RSame;


    VEC_LOAD_OFFSET(F0, freq, 0 * sizeof(VEC_T));
    VEC_LOAD_OFFSET(F1, freq, 1 * sizeof(VEC_T));
    VEC_LOAD_OFFSET(F2, freq, 2 * sizeof(VEC_T));
    VEC_LOAD_OFFSET(F3, freq, 3 * sizeof(VEC_T));

    while (1) {
        // NOTE: expects valRare, F0-F3
        DEBUG_PRINT("valRare: %ld\n", valRare);
        DEBUG_PRINT("F0: " VEC_FORMAT_DEBUG(F0));
        DEBUG_PRINT("F1: " VEC_FORMAT_DEBUG(F1));
        DEBUG_PRINT("F2: " VEC_FORMAT_DEBUG(F2));
        DEBUG_PRINT("F3: " VEC_FORMAT_DEBUG(F3));

#ifdef IACA
        IACA_START;
#endif

        VEC_SET_ALL_TO_INT(RSame, valRare);
        uint64_t minFreq = freq[0];

        VEC_COPY(X1, RSame);
        VEC_COPY(X2, RSame);
        VEC_COPY(X3, RSame);

        VEC_CMP_GREATER(X1, F1);
        VEC_CMP_GREATER(X2, F2);
        VEC_CMP_GREATER(X3, F3);

        VEC_CMP_EQUAL(F0, RSame);
        VEC_CMP_EQUAL(F1, RSame);
        VEC_CMP_EQUAL(F2, RSame);
        VEC_CMP_EQUAL(F3, RSame);

        VEC_OR(F0, F1);
        VEC_OR(F2, F3);
        VEC_OR(F0, F2);

        // if (valRare > f[1 * VECLEN]) advanceFreq = 1
        // else advanceFreq = 0;
        int32_t low;
        VEC_COPY_LOW(low, X1);
        DEBUG_PRINT(" advanceFreq: X1 +%d,", -low);
        int64_t advanceFreq = (-low);

        // if (valRare > f[2 * VECLEN]) advanceFreq += 1
        VEC_COPY_LOW(low, X2);
        DEBUG_PRINT(" X2 +%d,", -low);
        advanceFreq -= low;

        // if (valRare > f[3 * VECLEN]) advanceFreq += 1
        VEC_COPY_LOW(low, X3);
        DEBUG_PRINT(" X3 +%d,", -low);
        advanceFreq -= low;

        // convert to {0, 1, 2, 3} to {0, 4, 8, 12}
        advanceFreq *= VECLEN;

        // preload for below
        uint64_t valRarePlusOne = rare[1];

        uint64_t maxFreq = freq[4 * VECLEN - 1];

        // if (valRare > f[0]) advanceFreq += 1;
        ASM_ADD_CMOV(advanceFreq, 1, valRare, minFreq, cmovg);
        DEBUG_PRINT(" f0 (%ld > %ld) +%d,", 
                    valRare, minFreq, (valRare > minFreq) ? 1 : 0);


        // if (valRare == [anyF]) advanceMatch = 1
        uint64_t advanceMatch = 0;
        VEC_SET_PTEST(advanceMatch, one, F0);

        // write potential match
        *matchOut = valRare;

        // advance freq one more if match was found
        advanceFreq += advanceMatch;
        DEBUG_PRINT(" match +%ld\n", advanceMatch);
        freq += advanceFreq;

        // advance matchOut if there was a match
        matchOut += advanceMatch;

        DEBUG_PRINT("advanceFreq: %ld advanceMatch: %ld advanceRare: %d\n",
                    advanceFreq, advanceMatch, 
                    (valRare <= maxFreq) ? 1 : 0);

        if (COMPILER_RARELY(freq >= stopFreq)) {
            if (valRare <= maxFreq) rare += 1;
            goto FINISH_SCALAR;
        }

        // load the upcoming vectors as quickly as we can
        VEC_LOAD_OFFSET(F1, freq, 1 * sizeof(VEC_T));
        VEC_LOAD_OFFSET(F2, freq, 2 * sizeof(VEC_T));
        VEC_LOAD_OFFSET(F3, freq, 3 * sizeof(VEC_T));
        VEC_LOAD_OFFSET(F0, freq, 0 * sizeof(VEC_T));

        if (COMPILER_RARELY(rare >= stopRare)) {
            if (valRare <= maxFreq) rare += 1;
            goto FINISH_SCALAR;
        }
            
        //    if (valRare <= maxFreq) {
        //        rare += 1; 
        //        valRare = valRarePlusOne;
        //    }
        // PROFILE: faster as flow control?
        const uint32_t *rarePlusOne = rare + 1;
        ASM_BLOCK(ASM_LINE("cmp %2, %0")                                \
                  ASM_LINE("cmovle %3, %1")                               \
                  ASM_LINE("cmovle %4, %0") :                             \
                  /* writes %0 */ "+r" (valRare),                         \
                  /* writes %1 */ "+r" (rare) :                           \
                  /* reads %2 */ "r" (maxFreq),                           \
                  /* reads %3 */ "r" (rarePlusOne),                       \
                  /* reads %4 */ "r" (valRarePlusOne) :                   \
                  /* clobbers */ "cc");

#ifdef IACA
        IACA_END;
#endif
    }

    size_t count;
 FINISH_SCALAR:
    count = matchOut - matchOrig;
    
    DEBUG_PRINT("Pre-tail count %zd\n", count);
    
    lenFreq = stopFreq + FREQ_SPACE - freq;
    lenRare = stopRare + RARE_SPACE - rare;
    DEBUG_PRINT("lenFreq %ld lenRare %ld\n", lenFreq, lenRare);
    
    size_t tail = match_scalar(freq, lenFreq, rare, lenRare, matchOut);
    
    DEBUG_PRINT("Tail count %zd\n", tail);
    
    return count + tail;
}


#ifdef TEST_SCALVEC


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFSIZE 2048


int test_identical() {
    
    uint32_t freq[BUFSIZE];
    uint32_t rare[BUFSIZE];
    uint32_t out[BUFSIZE];

    uint64_t lenFreq;
    uint64_t lenRare;

    for (int i = 0; i < BUFSIZE; i++) {
        freq[i] = i;
        rare[i] = i;
        out[i] = 0;
    }
    lenFreq = BUFSIZE;
    lenRare = BUFSIZE;
   
    size_t count;
    count = match_scalvec_v4_r1g1_f4g1(freq, lenFreq, rare, lenRare, out);
    
    printf("Count: %zd (%d)\n", count, BUFSIZE/2);

    //    for (int i = 0; i < BUFSIZE; i++) {
    //        printf("%d: %d\n", i, out[i]);
    //    }

    return 0;
}

int test_half() {
    
    uint32_t freq[BUFSIZE * 2];
    uint32_t rare[BUFSIZE];
    uint32_t out[BUFSIZE];

    uint64_t lenFreq = BUFSIZE;
    uint64_t lenRare = BUFSIZE / 2;

    for (int i = 0; i < BUFSIZE; i++) {
        out[i] = 0;
        rare[i] = 2 * i;
        freq[2 * i] = 2 * i;
        freq[2 * i + 1] = 2 * i + 1;
    }

    size_t count;

    count = match_scalvec_v4_r1g1_f4g1(freq, lenFreq, rare, lenRare, out);
    printf("Count: %zd (%d)\n", count, BUFSIZE/2);
    //    for (int i = 0; i <= count; i++) {
    //        printf("%d: %d\n", i, out[i]);
    //    }

    count = match_scalvec_v4_r1g1_f4g1(rare, lenRare, freq, lenFreq, out);
    printf("Reversed count: %zd (%d)\n", count, BUFSIZE/2);
    //    for (int i = 0; i <= count; i++) {
    //        printf("%d: %d\n", i, out[i]);
    //    }

    
    return 0;
}


int test_random(uint32_t increment) {
    
    uint32_t freq[BUFSIZE * 2];
    uint32_t rare[BUFSIZE];

    uint32_t out_forward[BUFSIZE];
    uint32_t out_reverse[BUFSIZE];
    uint32_t out_scalar[BUFSIZE];

    uint64_t lenFreq = BUFSIZE;
    uint64_t lenRare = BUFSIZE / 2;

    freq[0] = rand() % increment;
    out_forward[0] = 0;
    out_reverse[0] = 0;
    out_scalar[0] = 0;
    for (int i = 1; i < BUFSIZE; i++) {
        freq[i] = freq[i - 1] + rand() % increment + 1; 
        out_forward[i] = 0;
        out_reverse[i] = 0;
        out_scalar[i] = 0;
    }

    rare[0] = rand() % increment;
    for (int i = 1; i < BUFSIZE/2; i++) {
        rare[i] = rare[i - 1] + rand() % increment + 1; 
    }



    size_t count_forward = 
        match_scalvec_v4_r1g1_f4g1(freq, lenFreq, rare, lenRare, out_forward);

    size_t count_reverse =
        match_scalvec_v4_r1g1_f4g1(rare, lenRare, freq, lenFreq, out_reverse);

    size_t count_scalar = 
        match_scalar(freq, lenFreq, rare, lenRare, out_scalar);

    printf("Count scalar: %zd\n", count_scalar);
    printf("Count forward: %zd\n", count_forward);
    printf("Count reverse: %zd\n", count_reverse);
    
    if (memcmp(out_scalar, out_forward, count_scalar * sizeof(uint32_t)) ||
        memcmp(out_scalar, out_reverse, count_scalar * sizeof(uint32_t))) {
        printf("Different output\n# scalar forward reverse\n");
        for (int i = 0; i < count_scalar; i++) {
            printf("%d: %d %d %d\n", i, out_scalar[i], 
                   out_forward[i], out_reverse[i]);
        }
    }
    else {
        printf("Output identical.\n");
    }

        

    return 0;
}

int main(void) {
    //    test_identical();
    //    test_half();
    for (int i = 1; i < 20; i++) {
        for (int j = 0; j < 10; j++) test_random(i);
    }
}

#endif // TEST_SCALVEC
