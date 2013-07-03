#include <stdint.h>
#include "compiler.h"
#include "vec4.h"
#include "asm.h"
#define DEBUG_PRINT(args...) 

#include </opt/intel/iaca-lin32/include/iacaMarks.h>


size_t match_scalar(const uint32_t *freq, size_t lenFreq, 
                    const uint32_t *rare, size_t lenRare,
                    uint32_t *matchOut);

#define FREQ_SPACE (-1)
#define RARE_SPACE (1)

size_t match_scalvec_v4_r1g1_f4g1(const uint32_t *freq, size_t lenFreq, 
                                  const uint32_t *rare, size_t lenRare,
                                  uint32_t *matchOut) {

    const uint32_t *matchOrig = matchOut;
    if (lenFreq == 0 || lenRare == 0) {
        return 0;
    }

    const uint32_t *lastRare = &rare[lenRare];
    const uint32_t *lastFreq = &freq[lenFreq];

    const uint32_t *stopFreq = lastFreq - FREQ_SPACE; // OK to read lastFreq
    const uint32_t *stopRare = lastRare - RARE_SPACE; // Room to read rare[1]

    if (COMPILER_RARELY(freq >= stopFreq) || rare >= stopRare) {
        goto FINISH_SCALAR;
    }

    ASM_REGISTER(VEC_T NextRare, xmm0);
    ASM_REGISTER(VEC_T Rare, xmm1);
    ASM_REGISTER(VEC_T NextRareCopy, xmm2);
    ASM_REGISTER(VEC_T Freq, xmm3);

    VEC_LOAD(Freq, freq);
    ASM_REGISTER(uint64_t maxFreq, r8, = freq[VECLEN - 1]);  

    //    ASM_REGISTER(uint64_t valRare, r9, = rare[0]);
    uint64_t valRare = rare[0];
    VEC_SET_ALL_TO_INT(Rare, valRare);
    ASM_REGISTER(uint64_t valNextRare, r9);

    // see if rare will advance the first time through
    if (valRare <= maxFreq) {
        valNextRare = rare[1];
        VEC_SET_ALL_TO_INT(NextRare, valNextRare);
    }
    else {
        valNextRare = valRare;
        NextRare = Rare;
    }

    ASM_REGISTER(uint64_t mask, r10);
    ASM_REGISTER(uint64_t advanceOut, r10);
    ASM_REGISTER(uint64_t advanceFreq, r11);
    ASM_REGISTER(uint64_t advanceRare, r11);
    ASM_REGISTER(uint64_t one, r15, = 1L);

    do {
#ifdef IACA
        IACA_START;
#endif
        // Advance freq
        VEC_COPY(NextRareCopy, NextRare);
        VEC_CMP_GREATER(NextRare, Freq);

        // Write next potential match (may be overwritten)
        *matchOut = valRare;

        VEC_READ_MASK(mask, NextRare);
        VEC_POPCNT(advanceFreq, mask);

        // Check for match
        VEC_MATCH(Rare, Freq);

        // NOTE: the latency of loading maxFreq is on the critical path
        // PROFILE: make sure maxFreq has priority over other loads
        maxFreq = freq[advanceFreq + VECLEN - 1];
        
        //        freq += advanceFreq;  
        ASM_PTR_ADD(freq, advanceFreq);
        VEC_LOAD(Freq, freq);

        // Advance output if match was found
        advanceOut = 0;
        VEC_SET_PTEST(advanceOut, one, Rare);
        ASM_PTR_ADD(matchOut, advanceOut);

        VEC_COPY(Rare, NextRareCopy);

        advanceRare = 0;
        // Drop to assembly to ensure a conditional move
        // if (valRare <= maxFreq) {
        //    advanceRare = one; 
        //    valRare = valNextRare;
        // }
        ASM_BLOCK(ASM_LINE("cmp %0, %4")                         \
                  ASM_LINE("cmovle %2, %0")                      \
                  ASM_LINE("cmovle %3, %1") :                    \
                  /* writes %0 */ "+r" (advanceRare),            \
                  /* writes %1 */ "+r" (valRare) :               \
                  /* reads %2 */ "r" (one),                      \
                  /* reads %3 */ "r" (valNextRare),              \
                  /* reads %4 */ "r" (maxFreq) :                 \
                  /* clobbers */ "cc");
        // NOTE: "+r" for conditional moves since initial value is live

        VEC_SET_ALL_TO_INT(NextRare, valRare);
        ASM_PTR_ADD(rare, advanceRare);
        valNextRare = rare[1]; // preload

#ifdef IACA
        IACA_END;
#endif


    } while (rare < stopRare && freq < stopFreq);


    size_t count;
FINISH_SCALAR:
    count = matchOut - matchOrig;
    
    lenFreq = stopFreq + FREQ_SPACE - freq;
    lenRare = stopRare + RARE_SPACE - rare;
    size_t tail = match_scalar(freq, lenFreq, rare, lenRare, matchOut);

    return count + tail;
}
