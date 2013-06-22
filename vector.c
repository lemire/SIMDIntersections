// NOTE: cpp options: -C keeps comments, -CC keeps macro comments
//       -P to inhibit line numbering, -dX might be useful
//       -fdirectives-only to skip macro expansion

// COMPILER_ASSERT(EARLYCYCLES < NUMFREQ);

#include "macro.h"

// PROFILE: check that static 'if' clauses are optimized out        

#define _CHECK_INNER_VECLEN(veclen) CHECK_INNER_VECLEN_ ## veclen
#define CHECK_INNER_VECLEN(veclen) _CHECK_INNER_VECLEN(veclen)

#define CHECK_INNER(freqnum, rarenum) {                 \
    Freq = NextFreq;                                    \
    if (freqnum + 1 < NUMFREQ) {                        \
        VLOAD(NextFreq, freq, freqnum + 1);             \
    } else if (rarenum + 1 < NUMRARE) {                 \
        VLOAD(NextFreq, freq, 0);                       \
    } else if (! lastPass) {                            \
        VLOAD(NextFreq, nextFreq, 0);                   \
    }                                                   \
    CHECK_INNER_VECLEN(VECLEN)                          \
    }

#define CHECK_INNER_VECLEN_4                    \
    VSHUF(F0, Freq, 0);                         \
                                                \
    VSHUF(F1, Freq, 1);                         \
    VMATCH(F0, Rare);                           \
                                                \
    VSHUF(F2, Freq, 2);                         \
    VMATCH(F1, Rare);                           \
    VOR(Match, F0);                             \
                                                \
    VSHUF(F3, Freq, 3);                         \
    VMATCH(F2, Rare);                           \
    VOR(Match, F1);                             \
                                                \
    VMATCH(F3, Rare);                           \
    VOR(Match, F2);                             \
                                                \
    VOR(Match, F3)                                   

#define CHECK_INNER_VECLEN_8                    \
    CHECK_INNER_VECLEN_4;                       \
    CHECK_INNER_VECLEN_4                      

#define CHECK_OUTER(rarenum) {                                          \
        Rare = NextRare;                                                \
    if (rarenum + 1 < NUMRARE) {                                        \
        VLOAD(NextRare, rare, rarenum + 1);                             \
    } else if (! lastPass) {                                            \
        VLOAD(NextRare, nextRare, 0);                                   \
    }                                                                   \
    VZERO(Match);                                                       \
    MACRO_REPEAT_ADD_ONE_INNER(CHECK_INNER, NUMFREQ, 0, rarenum);       \
    int bits = _mm_movemask_ps((VECFLOAT) Match);                       \
    HANDLE_RARE_MATCH(Match, rarenum)                                   \
    }

// PROFILE: compare kShuffleCount[] to popcnt()
#ifdef JUSTCOUNT
#define HANDLE_RARE_MATCH(Match, rarenum)       \
    count += kShuffleCount[bits]
#else // WRITE
#define HANDLE_RARE_MATCH(Match, rarenum)                               \
    VSHUF(Rare, kShuffleMatch[bits]);                                   \
    VSTORE(match, Rare);                                                \
    match += kShuffleCount[bits];  
#endif
// FUTURE: optimize for two at a time with BLEND instead of SHUF?

// PROFILE: Compare ADVANCE_BEYOND_NEWMIN and ADVANCE_BEYOND_OLDMAX?
#define ADVANCE_BEYOND_NEWMIN(num, granularity, array, next, newMin) {  \
        int advance = 0;                                                \
        if (newMin > array[num - 1]) {                                  \
            advance = granularity;                                      \
        }                                                               \
        next += advance;                                                \
    }

// NOTE: newMin > array[num - 1] vs oldMax >= array[num - 1]
#define ADVANCE_BEYOND_OLDMAX(num, granularity, array, next, oldMax) {  \
        int advance = 0;                                                \
        if (oldMax >= array[num - 1]) {                                 \
            advance = granularity;                                      \
        }                                                               \
        next += advance;                                                \
    }


#define _FUNC(name, numrare, numfreq) name ## _R ## numrare ## _F ## numfreq 
#define FUNC(name, numrare, numfreq) _FUNC(name, numrare, numfreq)

#ifdef JUSTCOUNT
size_t FUNC(count_vector, NUMRARE, NUMFREQ)(const uint32_t *freq, size_t lenFreq,
                                            const uint32_t *rare, size_t lenRare)  {
#else // WRITE
size_t FUNC(match_vector, NUMRARE, NUMFREQ)(const uint32_t *freq, size_t lenFreq,
                                            const uint32_t *rare, size_t lenRare,
                                            uint32_t *matchOut)  {
    const uint32_t *matchOrig = matchOut;
#endif
    
    size_t count = 0;
    if (lenFreq == 0 || lenRare == 0) {
        return 0;
    }

    const uint32_t *lastRare = &rare[lenRare];
    const uint32_t *lastFreq = &rare[lenRare];

    // FIXME: determine correct limits
    const uint32_t *stopFreq = lastFreq - FREQSPACE;
    const uint32_t *stopRare = lastRare - RARESPACE;
    
    // use scalar if not enough room to load vectors
    if (rarely(freq >= stopFreq) || rare >= stopRare) {
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

    uint32_t maxFreq = freq[NUMFREQ * VECLEN - 1];  
    uint32_t maxRare = rare[NUMRARE * VECLEN - 1];  
    
    uint32_t lastPass = 0;  // current pass always completed
    uint32_t nextRare, nextFreq; // used outside loop for scalar finish

    do {

#if BRANCHLESS
        {
#define TIMES MACRO_ADD(MACRO_DIV(NUMRARE, GRANRARE), RARELOOK)
            MACRO_REPEAT_ADDING(ADVANCE_BEYOND_OLDMAX, TIMES, GRANRARE, 
                                GRANRARE, rare, nextRare, maxFreq);
#undef TIMES
            int newRareMin = nextRare[0];

#if EARLYCYCLES > 0
            // CHECK_OUTER(0) ... CHECK_OUTER(EARLYCYCLES-1)
            MACRO_REPEAT_ADDING_ONE(CHECK_OUTER, EARLYCYCLES, 0);
#endif 
#define TIMES MACRO_ADD(MACRO_DIV(NUMFREQ, GRANFREQ), FREQLOOK)
            MACRO_REPEAT_ADDING(ADVANCE_BEYOND_NEWMIN, times, GRANFREQ, 
                                GRANFREQ, freq, nextFreq, newRareMin);
#undef TIMES
        }
#else // if not BRANCHLESS
        if (expected(maxFreq >= maxRare)) {  
            nextRare = rare + NUMRARE * VECLEN;    
            int newRareMin = nextRare[0];

#if EARLYCYCLES > 0
            // CHECK_OUTER(0) ... CHECK_OUTER(EARLYCYCLES-1)
            MACRO_REPEAT_ADDING_ONE(CHECK_OUTER, EARLYCYCLES, 0);
#endif

#define TIMES MACRO_ADD(MACRO_DIV(NUMFREQ, GRANFREQ), FREQLOOK)
            MACRO_REPEAT_ADDING(ADVANCE_BEYOND_NEWMIN, TIMES, GRANFREQ, GRANFREQ, 
                                GRANFREQ, freq, nextFreq, newRareMin);
#undef TIMES
        } else {
            nextFreq = freq + NUMFREQ * VECLEN;
            int newFreqMin = nextFreq[0];

#if EARLYCYCLES > 0
            // CHECK_OUTER(0) ... CHECK_OUTER(EARLYCYCLES-1)
            MACRO_REPEAT_ADDING_ONE(CHECK_OUTER, EARLYCYCLES, 0);
#endif

#define TIMES MACRO_ADD(MACRO_DIV(NUMRARE, GRANRARE), RARELOOK)
            MACRO_REPEAT_ADDING(ADVANCE_BEYOND_NEWMIN, TIMES, GRANRARE, GRANRARE, 
                                GRANRARE, rare, nextRare, newFreqMin);
#undef TIMES
        }
#endif // end not BRANCHLESS

        if (unexpected(nextRare >= stopRare || nextFreq >= stopFreq)) {
            lastPass = 1;
        } else {
            // preload next maxFreq and maxRare
            maxFreq = nextFreq[NUMFREQ * VECLEN - 1];  
            maxRare = nextRare[NUMRARE * VECLEN - 1];  
            // NextRare and NextFreq loaded in macros below
        }

#if EARLYCYCLES > 0
        // CHECK_OUTER(EARLYCYCLES) ... CHECK_OUTER(NUMRARE-1)
        // calls CHECK_INNER(0 .. NUMFREQ-1) for each
#define TIMES MACRO_SUB(NUMRARE, EARLYCYCLES)
        MACRO_REPEAT_ADDING_ONE(CHECK_OUTER, TIMES, EARLYCYCLES);
#undef TIMES
#else
        // CHECK_OUTER(0) ... CHECK_OUTER(NUMRARE-1)
        // calls CHECK_INNER(0 .. FREQ-1) for each
        MACRO_REPEAT_ADDDING_ONE(CHECK_OUTER, NUMRARE, 0);
#endif

        freq = nextFreq;
        rare = nextRare;
    } while (! lastPass);

 FINISH_SCALAR:
    return count; // plus scalar

#undef CHECK_INNER
#undef CHECK_INNER_VECLEN
#undef CHECK_INNER_VECLEN_8
#undef CHECK_INNER_VECLEN_4
#undef HANDLE_RARE_MATCH
#undef CHECK_OUTER
#undef ADVANCE_BEYOND_NEWMIN
#undef ADVANCE_BEYOND_OLDMAX
#undef FUNC
}

