
#ifndef MEDIUMINTERSECTION_H_
#define MEDIUMINTERSECTION_H_

#define VEC __m128i
#define VECLEN (sizeof(VEC)/sizeof(uint32_t))
#define VECMAX (VECLEN - 1)

size_t nate_count_scalar(const uint32_t *A, const size_t lenA,
        const uint32_t *B, const size_t lenB) {

    size_t count = 0;
    if (lenA == 0 || lenB == 0)
        return count;

    const uint32_t *endA = A + lenA;
    const uint32_t *endB = B + lenB;

    while (1) {
        while (*A < *B) {
            SKIP_FIRST_COMPARE: if (++A == endA)
                return count;
        }
        while (*A > *B) {
            if (++B == endB)
                return count;
        }
        if (*A == *B) {
            count++;
            if (++A == endA || ++B == endB)
                return count;
        } else {
            goto SKIP_FIRST_COMPARE;
        }
    }

    return count; // NOTREACHED
}

size_t nate_count_medium(const uint32_t *rare, const size_t lenRare,
        const uint32_t *freq, const size_t lenFreq) {

    // FUTURE: could swap freq and rare if inverted

    size_t count = 0;
    if (lenFreq == 0 || lenRare == 0)
        return count;

#define FREQSPACE (8 * VECLEN)
#define RARESPACE 1
    const uint32_t *stopFreq = freq + lenFreq - FREQSPACE;
    const uint32_t *stopRare = rare + lenRare - RARESPACE;

    if (freq > stopFreq) {
        return nate_count_scalar(freq, lenFreq, rare, lenRare);
    }

    uint32_t nextRare = *rare;
    uint32_t maxFreq = freq[7 * VECLEN + VECMAX];
    VEC M0, M1, M2, M3, M4, M5, M6, M7;
    VEC A0, A1, A2, A3;
    A0 = _mm_load_si128((VEC *) freq + 0);
    A1 = _mm_load_si128((VEC *) freq + 1);
    A2 = _mm_load_si128((VEC *) freq + 2);
    A3 = _mm_load_si128((VEC *) freq + 3);

    while (maxFreq < nextRare) { // advance freq to a possible match
        freq += VECLEN * 8; // NOTE: loop below requires this
        if (freq > stopFreq)
            goto FINISH_SCALAR;
        maxFreq = freq[VECLEN * 7 + VECMAX];
        A0 = _mm_load_si128((VEC *) freq + 0);
        A1 = _mm_load_si128((VEC *) freq + 1);
        A2 = _mm_load_si128((VEC *) freq + 2);
        A3 = _mm_load_si128((VEC *) freq + 3);
    }

    while (rare < stopRare) {
        uint32_t matchRare = nextRare;
        VEC Match = _mm_set1_epi32(matchRare);
        if (maxFreq < matchRare) { // if no match possible
            freq += VECLEN * 8; // advance 8 vectors
            if (freq > stopFreq)
                goto FINISH_SCALAR;
            maxFreq = freq[VECLEN * 7 + VECMAX];
            while (maxFreq < matchRare) { // if still no match possible
                freq += VECLEN * 8; // advance another 8 vectors
                if (freq > stopFreq)
                    goto FINISH_SCALAR;
                maxFreq = freq[VECLEN * 7 + VECMAX];
                // printf("FREQ_ADVANCE2: %d %d\n", matchRare, maxFreq);
                M0 = _mm_load_si128((VEC *) freq + 0);
                M1 = _mm_load_si128((VEC *) freq + 1);
                M2 = _mm_load_si128((VEC *) freq + 2);
                M3 = _mm_load_si128((VEC *) freq + 3);
            }
            A0 = M0; // ratchet original vectors forward
            A1 = M1;
            A2 = M2;
            A3 = M3;
        } else { // if a match is still possible
            M0 = A0; // revert to original vectors
            M1 = A1;
            M2 = A2;
            M3 = A3;
        }

        rare += 1;
        nextRare = *rare;
        // NOTE: At this point M0, M1, M2, M3 must match freq + 0,1,2,3
        VEC Q0, Q1, Q2, Q3; // quarters
        VEC S0, S1; // semis
        VEC F0; // final

        M4 = _mm_load_si128((VEC *) freq + 4);
        M5 = _mm_load_si128((VEC *) freq + 5);
        M6 = _mm_load_si128((VEC *) freq + 6);
        M7 = _mm_load_si128((VEC *) freq + 7);

        M0 = _mm_cmpeq_epi32(M0, Match);
        M1 = _mm_cmpeq_epi32(M1, Match);
        Q0 = _mm_or_si128(M0, M1);
        M0 = _mm_load_si128((VEC *) freq + 8);
        M1 = _mm_load_si128((VEC *) freq + 9);

        M2 = _mm_cmpeq_epi32(M2, Match);
        M3 = _mm_cmpeq_epi32(M3, Match);
        Q1 = _mm_or_si128(M2, M3);
        M2 = _mm_load_si128((VEC *) freq + 10);
        M3 = _mm_load_si128((VEC *) freq + 11);

        M4 = _mm_cmpeq_epi32(M4, Match);
        M5 = _mm_cmpeq_epi32(M5, Match);
        Q2 = _mm_or_si128(M4, M5);

        M6 = _mm_cmpeq_epi32(M6, Match);  
        M7 = _mm_cmpeq_epi32(M7, Match);
        Q3 = _mm_or_si128(M6, M7);

        S0 = _mm_or_si128(Q0, Q1);
        S1 = _mm_or_si128(Q2, Q3);
        F0 = _mm_or_si128(S0, S1);

        // PROFILE: check that this compiles to a CMOV
        if (_mm_testz_si128(F0, F0)) {
            // printf("NOT_FOUND: %d %d\n", matchRare, maxFreq);
        } else {
            // printf("FOUND: %d %d\n", matchRare, maxFreq);
            count += 1;
        }
    }

    FINISH_SCALAR: return count + nate_count_scalar(freq,
            stopFreq + FREQSPACE - freq, rare, stopRare + RARESPACE - rare);
}


size_t nate_count_simple16(const uint32_t *rare, const size_t lenRare,
                          const uint32_t *freq, const size_t lenFreq) {
    size_t count = 0;
    if (lenFreq == 0 || lenRare == 0)
        return count;
    typedef __m128i vec;
    const uint32_t veclen = sizeof(VEC) / sizeof(uint32_t);
    const size_t vecmax = veclen - 1;
    const size_t freqspace = 16 * veclen;
    const size_t rarespace = 1;

    const uint32_t *stopFreq = freq + lenFreq - freqspace;
    const uint32_t *stopRare = rare + lenRare - rarespace;
    if (freq > stopFreq) {
        return nate_count_scalar(freq, lenFreq, rare, lenRare);
    }
    while (freq[veclen * 15 + vecmax] < *rare) {
        freq += veclen * 16;
        if (freq > stopFreq)
            goto FINISH_SCALAR;
    }


    for (; rare < stopRare; ++rare) {
        const uint32_t matchRare = *rare;
        const vec Match = _mm_set1_epi32(matchRare);
        while (freq[veclen * 15 + vecmax] < matchRare) { // if no match possible
            freq += veclen * 16; // advance 16 vectors
            if (freq > stopFreq)
                goto FINISH_SCALAR;
        }

        VEC M0, M1, M2, M3, M4, M5, M6, M7;
        VEC Q0, Q1, Q2, Q3; // quarters
        VEC S0, S1; // semis
        VEC F0, F1; // final

        M0 = _mm_load_si128((VEC *) freq + 0);
        M1 = _mm_load_si128((VEC *) freq + 1);
        M2 = _mm_load_si128((VEC *) freq + 2);
        M3 = _mm_load_si128((VEC *) freq + 3);
        M4 = _mm_load_si128((VEC *) freq + 4);
        M5 = _mm_load_si128((VEC *) freq + 5);
        M6 = _mm_load_si128((VEC *) freq + 6);
        M7 = _mm_load_si128((VEC *) freq + 7);

        M0 = _mm_cmpeq_epi32(M0, Match);
        M1 = _mm_cmpeq_epi32(M1, Match);
        Q0 = _mm_or_si128(M0, M1);
        M0 = _mm_load_si128((VEC *) freq + 8);
        M1 = _mm_load_si128((VEC *) freq + 9);

        M2 = _mm_cmpeq_epi32(M2, Match);
        M3 = _mm_cmpeq_epi32(M3, Match);
        Q1 = _mm_or_si128(M2, M3);
        M2 = _mm_load_si128((VEC *) freq + 10);
        M3 = _mm_load_si128((VEC *) freq + 11);

        M4 = _mm_cmpeq_epi32(M4, Match);
        M5 = _mm_cmpeq_epi32(M5, Match);
        Q2 = _mm_or_si128(M4, M5);
        M4 = _mm_load_si128((VEC *) freq + 12);
        M5 = _mm_load_si128((VEC *) freq + 13);

        M6 = _mm_cmpeq_epi32(M6, Match);
        M7 = _mm_cmpeq_epi32(M7, Match);
        Q3 = _mm_or_si128(M6, M7);
        M6 = _mm_load_si128((VEC *) freq + 14);
        M7 = _mm_load_si128((VEC *) freq + 15);

        S0 = _mm_or_si128(Q0, Q1);
        S1 = _mm_or_si128(Q2, Q3);
        F0 = _mm_or_si128(S0, S1);

        // M0 - M7 have been reloaded 

        M0 = _mm_cmpeq_epi32(M0, Match);
        M1 = _mm_cmpeq_epi32(M1, Match);
        Q0 = _mm_or_si128(M0, M1);

        M2 = _mm_cmpeq_epi32(M2, Match);
        M3 = _mm_cmpeq_epi32(M3, Match);
        Q1 = _mm_or_si128(M2, M3);

        M4 = _mm_cmpeq_epi32(M4, Match);
        M5 = _mm_cmpeq_epi32(M5, Match);
        Q2 = _mm_or_si128(M4, M5);

        M6 = _mm_cmpeq_epi32(M6, Match);
        M7 = _mm_cmpeq_epi32(M7, Match);
        Q3 = _mm_or_si128(M6, M7);

        S0 = _mm_or_si128(Q0, Q1);
        S1 = _mm_or_si128(Q2, Q3);
        F1 = _mm_or_si128(S0, S1);

        F0 = _mm_or_si128(F0, F1);

        if (_mm_testz_si128(F0, F0)) {
        } else {
            count += 1;
        }
    }

    FINISH_SCALAR: return count + nate_count_scalar(freq,
            stopFreq + freqspace - freq, rare, stopRare + rarespace - rare);
}

/**
 * Version hacked by D. Lemire, original by Nathan Kurz
 */
size_t natedan_count_medium(const uint32_t *rare, const size_t lenRare,
        const uint32_t *freq, const size_t lenFreq) {
    // FUTURE: could swap freq and rare if inverted
    size_t count = 0;
    if (lenFreq == 0 || lenRare == 0)
        return count;

    typedef __m128i vec;
    const uint32_t veclen = sizeof(VEC) / sizeof(uint32_t);
    const size_t vecmax = veclen - 1;
    const size_t freqspace = 8 * veclen;
    const size_t rarespace = 1;

    const uint32_t *stopFreq = freq + lenFreq - freqspace;
    const uint32_t *stopRare = rare + lenRare - rarespace;
    if (freq > stopFreq) {
        return nate_count_scalar(freq, lenFreq, rare, lenRare);
    }
    uint32_t maxFreq = freq[7 * veclen + vecmax];
    vec M0, M1, M2, M3, M4, M5, M6, M7;

    while (maxFreq < *rare) { // advance freq to a possible match
        freq += veclen * 8; // NOTE: loop below requires this
        if (freq > stopFreq)
            goto FINISH_SCALAR;
        maxFreq = freq[veclen * 7 + vecmax];
    }
    M0 = _mm_load_si128((vec *) freq + 0);
    M1 = _mm_load_si128((vec *) freq + 1);
    M2 = _mm_load_si128((vec *) freq + 2);
    M3 = _mm_load_si128((vec *) freq + 3);
    M4 = _mm_load_si128((vec *) freq + 4);
    M5 = _mm_load_si128((vec *) freq + 5);
    M6 = _mm_load_si128((vec *) freq + 6);
    M7 = _mm_load_si128((vec *) freq + 7);

    for (; rare < stopRare; ++rare) {
        const uint32_t matchRare = *rare;//nextRare;
        const vec Match = _mm_set1_epi32(matchRare);
        if (maxFreq < matchRare) { // if no match possible
            freq += veclen * 8; // advance 8 vectors
            if (freq > stopFreq)
                goto FINISH_SCALAR;
            maxFreq = freq[veclen * 7 + vecmax];
            while (maxFreq < matchRare) { // if still no match possible
                freq += veclen * 8; // advance another 8 vectors
                if (freq > stopFreq)
                    goto FINISH_SCALAR;
                maxFreq = freq[veclen * 7 + vecmax];
            }
            M0 = _mm_load_si128((vec *) freq + 0);
            M1 = _mm_load_si128((vec *) freq + 1);
            M2 = _mm_load_si128((vec *) freq + 2);
            M3 = _mm_load_si128((vec *) freq + 3);
            M4 = _mm_load_si128((vec *) freq + 4);
            M5 = _mm_load_si128((vec *) freq + 5);
            M6 = _mm_load_si128((vec *) freq + 6);
            M7 = _mm_load_si128((vec *) freq + 7);

        }
        const vec Q0 = _mm_or_si128(_mm_cmpeq_epi32(M0, Match),
                _mm_cmpeq_epi32(M1, Match));
        const vec Q1 = _mm_or_si128(_mm_cmpeq_epi32(M2, Match),
                _mm_cmpeq_epi32(M3, Match));
        const vec Q2 = _mm_or_si128(_mm_cmpeq_epi32(M4, Match),
                _mm_cmpeq_epi32(M5, Match));
        const vec Q3 = _mm_or_si128(_mm_cmpeq_epi32(M6, Match),
                _mm_cmpeq_epi32(M7, Match));
        const vec S0 = _mm_or_si128(Q0, Q1);
        const vec S1 = _mm_or_si128(Q2, Q3);
        const vec F0 = _mm_or_si128(S0, S1);
        if (_mm_testz_si128(F0, F0)) {
        } else {
            count += 1;
        }
    }

    FINISH_SCALAR: return count + nate_count_scalar(freq,
            stopFreq + freqspace - freq, rare, stopRare + rarespace - rare);
}

/**
 * Version hacked by D. Lemire, original by Nathan Kurz
 */
size_t natedanalt_count_medium(const uint32_t *rare, const size_t lenRare,
        const uint32_t *freq, const size_t lenFreq) {
    // FUTURE: could swap freq and rare if inverted
    size_t count = 0;
    if (lenFreq == 0 || lenRare == 0)
        return count;

    typedef __m128i vec;
    const uint32_t veclen = sizeof(VEC) / sizeof(uint32_t);
    const size_t vecmax = veclen - 1;
    const size_t freqspace = 8 * veclen;
    const size_t rarespace = 1;

    const uint32_t *stopFreq = freq + lenFreq - freqspace;
    const uint32_t *stopRare = rare + lenRare - rarespace;
    if (freq > stopFreq) {
        return nate_count_scalar(freq, lenFreq, rare, lenRare);
    }
    uint32_t maxFreq = freq[7 * veclen + vecmax];

    while (maxFreq < *rare) { // advance freq to a possible match
        freq += veclen * 8; // NOTE: loop below requires this
        if (freq > stopFreq)
            goto FINISH_SCALAR;
        maxFreq = freq[veclen * 7 + vecmax];
    }
    for (; rare < stopRare; ++rare) {
        const uint32_t matchRare = *rare;//nextRare;
        const vec Match = _mm_set1_epi32(matchRare);
        while (maxFreq < matchRare) { // if no match possible
            freq += veclen * 8; // advance 8 vectors
            if (freq > stopFreq)
                goto FINISH_SCALAR;
            maxFreq = freq[veclen * 7 + vecmax];
        }
        vec F0;
        if(freq[veclen * 3 + vecmax] < matchRare  ) {
            const vec Q2 = _mm_or_si128(_mm_cmpeq_epi32(_mm_load_si128((vec *) freq + 4), Match),
                            _mm_cmpeq_epi32(_mm_load_si128((vec *) freq + 5), Match));
            const vec Q3 = _mm_or_si128(_mm_cmpeq_epi32(_mm_load_si128((vec *) freq + 6), Match),
                            _mm_cmpeq_epi32(_mm_load_si128((vec *) freq + 7), Match));
            F0 = _mm_or_si128(Q2, Q3);
        } else {
            const vec Q0 = _mm_or_si128(_mm_cmpeq_epi32(_mm_load_si128((vec *) freq + 0), Match),
                    _mm_cmpeq_epi32(_mm_load_si128((vec *) freq + 1), Match));
            const vec Q1 = _mm_or_si128(_mm_cmpeq_epi32(_mm_load_si128((vec *) freq + 2), Match),
                    _mm_cmpeq_epi32(_mm_load_si128((vec *) freq + 3), Match));
            F0 = _mm_or_si128(Q0, Q1);
        }
        if (_mm_testz_si128(F0, F0)) {
        } else {
            count += 1;
        }
    }

    FINISH_SCALAR: return count + nate_count_scalar(freq,
            stopFreq + freqspace - freq, rare, stopRare + rarespace - rare);
}


/**
 * Version hacked by D. Lemire, original by Nathan Kurz
 */
size_t danfar_count_medium(const uint32_t *rare, const size_t lenRare,
        const uint32_t *freq, const size_t lenFreq) {
    size_t count = 0;
    if (lenFreq == 0 || lenRare == 0)
        return count;
    typedef __m128i vec;
    const uint32_t veclen = sizeof(VEC) / sizeof(uint32_t);
    const size_t vecmax = veclen - 1;
    const size_t freqspace = 16 * veclen;
    const size_t rarespace = 1;

    const uint32_t *stopFreq = freq + lenFreq - freqspace;
    const uint32_t *stopRare = rare + lenRare - rarespace;
    if (freq > stopFreq) {
        return nate_count_scalar(freq, lenFreq, rare, lenRare);
    }
    while (freq[veclen * 15 + vecmax] < *rare) {
        freq += veclen * 16;
        if (freq > stopFreq)
            goto FINISH_SCALAR;
    }
    for (; rare < stopRare; ++rare) {
        const uint32_t matchRare = *rare;//nextRare;
        const vec Match = _mm_set1_epi32(matchRare);
        while (freq[veclen * 15 + vecmax] < matchRare) { // if no match possible
            freq += veclen * 16; // advance 8 vectors
            if (freq > stopFreq)
                goto FINISH_SCALAR;
        }
        vec Q0,Q1,Q2,Q3;
        if(freq[veclen * 7 + vecmax] < matchRare  ) {
            //if(freq[veclen * 11 + vecmax] < matchRare) {
                Q0 = _mm_or_si128(_mm_cmpeq_epi32(_mm_load_si128((vec *) freq + 12), Match),
                                _mm_cmpeq_epi32(_mm_load_si128((vec *) freq + 13), Match));
                Q1 = _mm_or_si128(_mm_cmpeq_epi32(_mm_load_si128((vec *) freq + 14), Match),
                                _mm_cmpeq_epi32(_mm_load_si128((vec *) freq + 15), Match));

            //} else {
                Q2 = _mm_or_si128(_mm_cmpeq_epi32(_mm_load_si128((vec *) freq + 8), Match),
                                _mm_cmpeq_epi32(_mm_load_si128((vec *) freq + 9), Match));
                Q3 = _mm_or_si128(_mm_cmpeq_epi32(_mm_load_si128((vec *) freq + 10), Match),
                                _mm_cmpeq_epi32(_mm_load_si128((vec *) freq + 11), Match));
            //}
        } else {
            //if(freq[veclen * 3 + vecmax] < matchRare) {
                Q0 = _mm_or_si128(_mm_cmpeq_epi32(_mm_load_si128((vec *) freq + 4), Match),
                                _mm_cmpeq_epi32(_mm_load_si128((vec *) freq + 5), Match));
                Q1 = _mm_or_si128(_mm_cmpeq_epi32(_mm_load_si128((vec *) freq + 6), Match),
                                _mm_cmpeq_epi32(_mm_load_si128((vec *) freq + 7), Match));
            //} else {
                Q2 = _mm_or_si128(_mm_cmpeq_epi32(_mm_load_si128((vec *) freq + 0), Match),
                                _mm_cmpeq_epi32(_mm_load_si128((vec *) freq + 1), Match));
                Q3 = _mm_or_si128(_mm_cmpeq_epi32(_mm_load_si128((vec *) freq + 2), Match),
                                _mm_cmpeq_epi32(_mm_load_si128((vec *) freq + 3), Match));
            //}
        }
        const vec F0 = _mm_or_si128(_mm_or_si128(Q0, Q1),_mm_or_si128(Q2, Q3));
        if (_mm_testz_si128(F0, F0)) {
        } else {
            count += 1;
        }
    }

    FINISH_SCALAR: return count + nate_count_scalar(freq,
            stopFreq + freqspace - freq, rare, stopRare + rarespace - rare);
}

size_t danfarfine_count_medium(const uint32_t *rare, const size_t lenRare,
        const uint32_t *freq, const size_t lenFreq) {
    size_t count = 0;
    if (lenFreq == 0 || lenRare == 0)
        return count;
    typedef __m128i vec;
    const uint32_t veclen = sizeof(VEC) / sizeof(uint32_t);
    const size_t vecmax = veclen - 1;
    const size_t freqspace = 16 * veclen;
    const size_t rarespace = 1;

    const uint32_t *stopFreq = freq + lenFreq - freqspace;
    const uint32_t *stopRare = rare + lenRare - rarespace;
    if (freq > stopFreq) {
        return nate_count_scalar(freq, lenFreq, rare, lenRare);
    }
    while (freq[veclen * 15 + vecmax] < *rare) {
        freq += veclen * 16;
        if (freq > stopFreq)
            goto FINISH_SCALAR;
    }
    for (; rare < stopRare; ++rare) {
        const uint32_t matchRare = *rare;//nextRare;
        const vec Match = _mm_set1_epi32(matchRare);
        while (freq[veclen * 15 + vecmax] < matchRare) { // if no match possible
            freq += veclen * 16; // advance 8 vectors
            if (freq > stopFreq)
                goto FINISH_SCALAR;
        }
        vec Q0,Q1;
        if(freq[veclen * 7 + vecmax] < matchRare  ) {
            if(freq[veclen * 11 + vecmax] < matchRare) {
                Q0 = _mm_or_si128(_mm_cmpeq_epi32(_mm_load_si128((vec *) freq + 12), Match),
                                _mm_cmpeq_epi32(_mm_load_si128((vec *) freq + 13), Match));
                Q1 = _mm_or_si128(_mm_cmpeq_epi32(_mm_load_si128((vec *) freq + 14), Match),
                                _mm_cmpeq_epi32(_mm_load_si128((vec *) freq + 15), Match));

            } else {
                Q0 = _mm_or_si128(_mm_cmpeq_epi32(_mm_load_si128((vec *) freq + 8), Match),
                                _mm_cmpeq_epi32(_mm_load_si128((vec *) freq + 9), Match));
                Q1 = _mm_or_si128(_mm_cmpeq_epi32(_mm_load_si128((vec *) freq + 10), Match),
                                _mm_cmpeq_epi32(_mm_load_si128((vec *) freq + 11), Match));
            }
        } else {
            if(freq[veclen * 3 + vecmax] < matchRare) {
                Q0 = _mm_or_si128(_mm_cmpeq_epi32(_mm_load_si128((vec *) freq + 4), Match),
                                _mm_cmpeq_epi32(_mm_load_si128((vec *) freq + 5), Match));
                Q1 = _mm_or_si128(_mm_cmpeq_epi32(_mm_load_si128((vec *) freq + 6), Match),
                                _mm_cmpeq_epi32(_mm_load_si128((vec *) freq + 7), Match));
            } else {
                Q0 = _mm_or_si128(_mm_cmpeq_epi32(_mm_load_si128((vec *) freq + 0), Match),
                                _mm_cmpeq_epi32(_mm_load_si128((vec *) freq + 1), Match));
                Q1 = _mm_or_si128(_mm_cmpeq_epi32(_mm_load_si128((vec *) freq + 2), Match),
                                _mm_cmpeq_epi32(_mm_load_si128((vec *) freq + 3), Match));
            }
        }
        const vec F0 = _mm_or_si128(Q0, Q1);
        if (_mm_testz_si128(F0, F0)) {
        } else {
            count += 1;
        }
    }

    FINISH_SCALAR: return count + nate_count_scalar(freq,
            stopFreq + freqspace - freq, rare, stopRare + rarespace - rare);
}




#endif /* MEDIUMINTERSECTION_H_ */
