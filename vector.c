// FUTURE: slightly better us use PACK rather than << and |=?

// PROFILE: check that 'if' clauses are static and optimized out        
#define CHECK_INNER_VECLEN CHECK_INNER_VECLEN_ ## VECLEN
#define CHECK_INNER(rarenum, freqnum)                   \
    Rare = NextRare;                                    \
    if (rarenum + 1 < NUMRARE) {                        \
        VLOAD(NextRare, rare, rarenum + 1);             \
    } else if (freqnum + 1 < NUMFREQ) {                 \
        VLOAD(NextRare, rare, 0);                       \
    } else if (! lastPass) {                            \
        VLOAD(NextRare, nextRare, 0);                   \
    }                                                   \
    CHECK_INNER_VECLEN(rarenum);                        \
    int bits = _mm_movemask_ps((VECFLOAT) Match);       \
    matchBits |= bits << rarenum

#define CHECK_INNER_VECLEN_4(num)               \
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
    VOR(Match, F3);                                   

#define CHECK_INNER_VECLEN_8(num)               \
    CHECK_INNER_VECLEN_4(num);                  \
    CHECK_INNER_VECLEN_4(num);                      

#define CHECK_OUTER(freqnum)                                    \
    Freq = NextFreq;                                            \
    if (freqnum + 1 < NUMFREQ) {                                \
        VLOAD(NextFreq, freq, freqnum);                         \
    } else if (! lastPass) {                                    \
        VLOAD(NextFreq, nextFreq, 0);                           \
    }                                                           \
    REPEAT_ADDING_ONE(CHECK_INNER, NUMRARE, 0, freqnum);        \


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

#define FUNC(name) name ## _R ## NUMRARE ## _F ## NUMFREQ

size_t FUNC(search_tight)(const uint32_t *freq, size_t lenFreq,
                          const uint32_t *rare, size_t lenRare)  {

    size_t count = 0;
    if (lenFreq == 0 || lenRare == 0) {
        return 0;
    }

    const uint32_t *lastRare = &rare[lenRare];
    const uint32_t *lastFreq = &rare[lenRare];

    // FIXME: determine correct limits
    const uint32_t *stopFreq = lastFreq - FREQSPACE;
    const uint32_t *stopRare = lastRare - RARESPACE;
    
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
        uint32_t maxFreq = freq[NUMFREQ * VECLEN - 1];  
        uint32_t maxRare = rare[NUMRARE * VECLEN - 1];  


// FUTURE: add a NONBRANCHING option that always inches both freq and rare
//         rare[0-X] >= maxFreq; freq[0-Y] > newMinRare
//         or vice versa?  which is better?


        if (expected(maxFreq >= maxRare)) {  
            nextRare = rare + NUMRARE * VECLEN;    
            int newRareMin = nextRare[0];

#if EARLYCYCLES > 0
            COMPILER_ASSERT(EARLYCYCLES < NUMFREQ);
            // CHECK_OUTER(0) ... CHECK_OUTER(EARLYCYCLES-1)
            REPEAT_ADDING_ONE(CHECK_OUTER, EARLYCYCLES, 0);
#endif

            int times = (NUMFREQ / GRANFREQ) + FREQLOOK;
            REPEAT_INCREMENT(ADVANCE_BEYOND_NEWMIN, times, GRANFREQ, 
                             GRANFREQ, freq, nextFreq, newRareMin);
        } else {
            nextFreq = freq + NUMFREQ * VECLEN;
            int newFreqMin = nextFreq[0];

#if EARLYCYCLES > 0
            COMPILER_ASSERT(EARLYCYCLES < NUMFREQ);
            // CHECK_OUTER(0) ... CHECK_OUTER(EARLYCYCLES-1)
            REPEAT_ADDING_ONE(CHECK_OUTER, EARLYCYCLES, 0);
#endif

            int times = (NUMRARE / GRANRARE) + RARELOOK;
            REPEAT_INCREMENT(ADVANCE_BEYOND_NEWMIN, times, GRANRARE, 
                             GRANRARE, rare, nextRare, newFreqMin);
        }

        // NOTE: NextRare and NextFreq loaded in last cycle of macros below
        if (unexpected(nextRare >= stopRare) || 
            unexpected(nextFreq >= stopFreq)) {
            lastPass = 1;
        } else {
            // FUTURE: preload maxFreq and maxRare?
            //         if so, need to load for first pass in header
        }

#if EARLYCYCLES > 0
        // CHECK_OUTER(EARLYCYCLES) ... CHECK_OUTER(NUMFREQ-1)
        // calls CHECK_INNER(0 .. NUMRARE-1) for each
        REPEAT_ADDING_ONE(CHECK_OUTER, NUMFREQ - EARLYCYCLES, EARLYCYCLES);
#else
        // CHECK_OUTER(0) ... CHECK_OUTER(NUMFREQ-1)
        // calls CHECK_INNER(0 .. NUMRARE-1) for each
        REPEAT_ADDING_ONE(CHECK_OUTER, NUMFREQ, 0);
#endif

        freq = nextFreq;
        rare = nextRare;

        Uint64_t passCount = _mm_popcnt_u64(matchBits);
        count += passCount;
    }

 FINISH_SCALAR:
    return count; // plus scalar

#undef CHECK_INNER
#undef CHECK_INNER_VECLEN
#undef CHECK_INNER_VECLEN_8
#undef CHECK_INNER_VECLEN_4
#undef CHECK_OUTER
}

