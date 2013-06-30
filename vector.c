// NOTE: cpp options: -C keeps comments, -CC keeps macro comments
//       -P to inhibit line numbering, -dX might be useful
//       -fdirectives-only to skip macro expansion

// NOTE: the same 'symbol' can only be used once per compilation unit
#define COMPILER_ASSERT(test)                           \
    _COMPILER_ASSERT_LINE(test, __LINE__, #test )
#define _COMPILER_ASSERT_LINE(test, line, string)                       \
    _COMPILER_ASSERT_PASTE(test, COMPILER_LINE_, line, string)
#define _COMPILER_ASSERT_PASTE(test, prefix, line, string)             \
    _COMPILER_ASSERT_SYMBOL(test, prefix ## line, string)
#define _COMPILER_ASSERT_SYMBOL(test, symbol, string)          \
    typedef char symbol [ ((test) ? 1 : string) ]

    //    typedef char symbol [ ((test) ? 1 : -1) ]

#include "macro.h"

// VECLEN: 4 for XMM SSE3; 8 for YMM AVX2
COMPILER_ASSERT(VECLEN == 4 || VECLEN == 8);
// COMPILER_ASSERT(VEC_INT_T); // FUTURE: how to check?

// NUMFREQ: number of ints from the "frequent" (longer) source
COMPILER_ASSERT(NUMFREQ >= 4);
COMPILER_ASSERT(MACRO_DIV(NUMFREQ, VECLEN) > 0);

// NUMRARE: number of ints from the "rare" (shorter) source
COMPILER_ASSERT(NUMRARE >= 4);
COMPILER_ASSERT(MACRO_DIV(NUMRARE, VECLEN) > 0);

// GRANFREQ: granularity in ints of checks to advance 'freq'
COMPILER_ASSERT(GRANFREQ > 0);
COMPILER_ASSERT(MACRO_DIV(NUMFREQ, GRANFREQ) > 0));

// GRANRARE: granularity in ints of checks to advance 'rare'
COMPILER_ASSERT(GRANRARE > 0);
COMPILER_ASSERT(MACRO_DIV(NUMRARE, GRANRARE) > 0));

// PEEKFREQ: number of extra GRANFREQ to try to skip in 'freq'
COMPILER_ASSERT(PEEKFREQ + 1);

// PEEKRARE: number of extra GRANRARE to try to skip in 'rare'
COMPILER_ASSERT(PEEKRARE + 1);

// EARLYCOMP: comparison cycles to perform "early" (before advancing second source)
COMPILER_ASSERT(EARLYCOMP <= MACRO_DIV(NUMRARE, GRANRARE));

// PROFILE: check that static 'if' clauses are optimized out        
#define _CHECK_INNER_VECLEN(veclen) CHECK_INNER_VECLEN_ ## veclen
#define CHECK_INNER_VECLEN(veclen) _CHECK_INNER_VECLEN(veclen)

#define CHECK_INNER(freqnum, rarenum)  {                                \
    DEBUG_PRINT("CHECK_INNER " #freqnum " outer " #rarenum);            \
    Freq = NextFreq;                                                    \
    if (freqnum + 1 < MACRO_DIV_INNER(NUMFREQ, VECLEN)) {               \
        VEC_LOAD(NextFreq, freq, freqnum + 1);                          \
    } else if (rarenum + 1 < MACRO_DIV_INNER(NUMRARE, VECLEN)) {        \
        VEC_LOAD(NextFreq, freq, 0);                                    \
    } else if (! lastPass) {                                            \
        VEC_LOAD(NextFreq, nextFreq, 0);                                \
    }                                                                   \
    CHECK_INNER_VECLEN(VECLEN);                                         \
    }

#define VEC_SHUF_ALL(index)                                 \
    (((index) << 6) | ((index) << 4) | ((index) << 2) | (index))

#define CHECK_INNER_VECLEN_4                    \
    VEC_SHUF(F0, Freq, VEC_SHUF_ALL(0));        \
                                                \
    VEC_SHUF(F1, Freq, VEC_SHUF_ALL(1));        \
    VEC_MATCH(F0, Rare);                        \
                                                \
    VEC_SHUF(F2, Freq, VEC_SHUF_ALL(2));        \
    VEC_MATCH(F1, Rare);                        \
    VEC_OR(Match, F0);                          \
                                                \
    VEC_SHUF(F3, Freq, VEC_SHUF_ALL(3));        \
    VEC_MATCH(F2, Rare);                        \
    VEC_OR(Match, F1);                          \
                                                \
    VEC_MATCH(F3, Rare);                        \
    VEC_OR(Match, F2);                          \
                                                \
    VEC_OR(Match, F3)                                   


#define CHECK_INNER_VECLEN_8                    \
    CHECK_INNER_VECLEN_8_NOT_YET_IMPLEMENTED


#define CHECK_OUTER(rarenum) {                                          \
        DEBUG_PRINT("CHECKOUTER " #rarenum);                            \
    Rare = NextRare;                                                    \
    if (rarenum + 1 < MACRO_DIV_INNER(NUMRARE, VECLEN)) {               \
        VEC_LOAD(NextRare, rare, rarenum + 1);                          \
    } else if (! lastPass) {                                            \
        VEC_LOAD(NextRare, nextRare, 0);                                \
    }                                                                   \
    VEC_ZERO(Match);                                                    \
    MACRO_REPEAT_ADDING_ONE_INNER(CHECK_INNER, MACRO_DIV_INNER(NUMFREQ, VECLEN), \
                                  0, rarenum);                          \
    int bits = VEC_MOVEMASK(Match);                                     \
    HANDLE_RARE_MATCH(Match, rarenum)                                   \
    }

// PROFILE: compare kShuffleCount[] to popcnt()
#if JUSTCOUNT
#define HANDLE_RARE_MATCH(Match, rarenum)       \
    count += kShuffleCount[bits]
#else // WRITE
#define HANDLE_RARE_MATCH(Match, rarenum)                               \
    VEC_SHUF(Rare, kShuffleMatch[bits]);                                \
    VEC_STORE(match, Rare);                                             \
    match += kShuffleCount[bits];  
#endif
// PROFILE: optimize for two at a time with BLEND instead of SHUF?

// PROFILE: Maybe always ADVANCE_BEYOND_OLDMAX instead of ADVANCE_BEYOND_NEWMIN?
#define ADVANCE_BEYOND_NEWMIN(num, granularity, array, next, newMin) {  \
        int advance = 0;                                                \
        if (newMin > array[num - 1]) advance = granularity;             \
        next += advance;                                                \
    }

// NOTE: newMin > array[num - 1] vs oldMax >= array[num - 1]
#define ADVANCE_BEYOND_OLDMAX(num, granularity, array, next, oldMax) {  \
        int advance = 0;                                                \
        if (oldMax >= array[num - 1]) advance = granularity;            \
        next += advance;                                                \
    }

#if BRANCHLESS
#define BRANCH() B
#else
#define BRANCH()
#endif

#if REVERSED
#define REVERSE() R
#else
#define REVERSE()
#endif

#define FUNC(args...) _FUNC(args)
#define _FUNC(name, veclen, numrare, granrare, peekrare, numfreq, granfreq, \
              peekfreq, earlycomp, branch, reversed)                    \
    name ## V ## veclen ## _R ## numrare ## G ## granrare ## P         \
        ## peekrare ##  _F ## numfreq ## G ## granfreq ## P ## peekfreq \
        ##  _E ## earlycomp ## branch ## reversed


#if JUSTCOUNT
size_t FUNC(count, VECLEN, NUMRARE, GRANRARE, PEEKRARE, NUMFREQ, GRANFREQ, \
            PEEKFREQ, EARLYCOMP, BRANCH(), REVERSED())
#if REVERSED  // swap rare and freq
    (const uint32_t *rare, size_t lenRare, const uint32_t *freq, size_t lenFreq)  {
#else
    (const uint32_t *freq, size_t lenFreq, const uint32_t *rare, size_t lenRare)  {
#endif // REVERSED
#else // WRITE
size_t FUNC(match, VECLEN, NUMRARE, GRANRARE, PEEKRARE, NUMFREQ, GRANFREQ, \
            PEEKFREQ, EARLYCOMP, BRANCH(), REVERSE())
#if REVERSED
    (const uint32_t *rare, size_t lenRare, const uint32_t *freq, size_t lenFreq,  
     uint32_t matchOut)  {
#else
    (const uint32_t *freq, size_t lenFreq, const uint32_t *rare, size_t lenRare, 
     uint32_t matchOut)  {
#endif // REVERSED
    const uint32_t *matchOrig = matchOut;
#endif // WRITE

    size_t count = 0;
    if (lenFreq == 0 || lenRare == 0) {
        return 0;
    }

    const uint32_t *lastRare = &rare[lenRare];
    const uint32_t *lastFreq = &rare[lenRare];

    // FUTURE: check that these limits are correct
    const uint32_t *stopFreq = lastFreq - NUMFREQ - (PEEKFREQ * GRANFREQ) - 1;
    const uint32_t *stopRare = lastRare - NUMRARE - (PEEKRARE * GRANRARE) - 1;
    
    // use scalar if not enough room to load vectors
    if (rarely(freq >= stopFreq) || rare >= stopRare) {
        goto FINISH_SCALAR;
    }

    register VEC_INT_T F0 REGISTER("xmm0");
    register VEC_INT_T F1 REGISTER("xmm1");
    register VEC_INT_T F2 REGISTER("xmm2");
    register VEC_INT_T F3 REGISTER("xmm3");

    register VEC_INT_T Match REGISTER("xmm4");

    register VEC_INT_T Rare REGISTER("xmm5");
    register VEC_INT_T Freq REGISTER("xmm6");

    register VEC_INT_T NextRare REGISTER("xmm7");
    register VEC_INT_T NextFreq REGISTER("xmm8");

    VEC_LOAD(NextFreq, freq, 0);
    VEC_LOAD(NextRare, rare, 0);

    uint32_t maxFreq = freq[NUMFREQ - 1];  
    uint32_t maxRare = rare[NUMRARE - 1];  
    
    uint32_t lastPass = 0;  // current pass always completed
    uint32_t nextRare, nextFreq; // used outside loop for scalar finish

    do {  // while (! lastPass)

#if BRANCHLESS
        {
#define TIMES MACRO_ADD(MACRO_DIV(NUMRARE, GRANRARE), PEEKRARE)
            MACRO_REPEAT_ADDING(ADVANCE_BEYOND_OLDMAX, TIMES, GRANRARE, GRANRARE,
                                GRANRARE, rare, nextRare, maxFreq);
#undef TIMES // NUMRARE/GRANRARE + PEEKRARE
            int newRareMin = nextRare[0];

#if EARLYCOMP > 0
            // CHECK_OUTER(0) ... CHECK_OUTER(EARLYCOMP-1)
            MACRO_REPEAT_ADDING_ONE(CHECK_OUTER, EARLYCOMP, 0);
#endif // EARLYCOMP

#define TIMES MACRO_ADD(MACRO_DIV(NUMFREQ, GRANFREQ), PEEKFREQ)
            MACRO_REPEAT_ADDING(ADVANCE_BEYOND_NEWMIN, TIMES, GRANFREQ, GRANFREQ,
                                GRANFREQ, freq, nextFreq, newRareMin);
#undef TIMES // NUMFREQ/GRANFREQ + PEEKFREQ
        }
#else // if not BRANCHLESS
        if (expected(maxFreq >= maxRare)) {  
            nextRare = rare + NUMRARE;    
            int newRareMin = nextRare[0];

#if EARLYCOMP > 0
            // CHECK_OUTER(0) ... CHECK_OUTER(EARLYCOMP-1)
            MACRO_REPEAT_ADDING_ONE(CHECK_OUTER, EARLYCOMP, 0);
#endif // EARLYCOMP

#define TIMES MACRO_ADD(MACRO_DIV(NUMFREQ, GRANFREQ), PEEKFREQ) 
            MACRO_REPEAT_ADDING(ADVANCE_BEYOND_NEWMIN, TIMES, GRANFREQ, GRANFREQ, 
                                GRANFREQ, freq, nextFreq, newRareMin);
#undef TIMES // NUMFREQ/GRANFREQ + PEEKFREQ
        } else {
            nextFreq = freq + NUMFREQ;
            int newFreqMin = nextFreq[0];

#if EARLYCOMP > 0
            // CHECK_OUTER(0) ... CHECK_OUTER(EARLYCOMP-1)
            MACRO_REPEAT_ADDING_ONE(CHECK_OUTER, EARLYCOMP, 0);
#endif

#define TIMES MACRO_ADD(MACRO_DIV(NUMRARE, GRANRARE), PEEKRARE)
            MACRO_REPEAT_ADDING(ADVANCE_BEYOND_NEWMIN, TIMES, GRANRARE, GRANRARE, 
                                GRANRARE, rare, nextRare, newFreqMin);
#undef TIMES // NUMRARE/GRANRARE + PEEKRARE
        }
#endif // end not BRANCHLESS

        if (unexpected(nextRare >= stopRare || nextFreq >= stopFreq)) {
            lastPass = 1;
        } else {
            // preload next maxFreq and maxRare
            maxFreq = nextFreq[NUMFREQ - 1];  
            maxRare = nextRare[NUMRARE - 1];  
            // NextRare and NextFreq loaded in macros below
        }

#if EARLYCOMP > 0
        // CHECK_OUTER(EARLYCOMP) ... CHECK_OUTER(NUMRAREVECS-1)
        // calls CHECK_INNER(0 .. NUMFREQVECS-1) for each
#define TIMES MACRO_SUB(MACRO_DIV(NUMRARE, VECLEN), EARLYCOMP)
        MACRO_REPEAT_ADDING_ONE(CHECK_OUTER, TIMES, EARLYCOMP);
#undef TIMES // NUMRARE - EARLYCOMP
#else
        // CHECK_OUTER(0) ... CHECK_OUTER(NUMRAREVECS-1)
        // calls CHECK_INNER(0 .. NUMFREQVECS-1) for each
        MACRO_REPEAT_ADDING_ONE(CHECK_OUTER, MACRO_DIV(NUMRARE, VECLEN), 0);
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
}

