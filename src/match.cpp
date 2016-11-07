#include "match.h"

size_t match_scalar(const uint32_t *A, const size_t lenA,
                    const uint32_t *B, const size_t lenB,
                    uint32_t *out) {

    const uint32_t *initout = out;
    if (lenA == 0 || lenB == 0) return 0;

    const uint32_t *endA = A + lenA;
    const uint32_t *endB = B + lenB;

    while (1) {
        while (*A < *B) {
        SKIP_FIRST_COMPARE:
            if (++A == endA) goto FINISH;
        }
        while (*A > *B) {
            if (++B == endB) goto FINISH;
        }
        if (*A == *B) {
            *out++ = *A;
            if (++A == endA || ++B == endB) goto FINISH;
        } else {
            goto SKIP_FIRST_COMPARE;
        }
    }

 FINISH:
    return (out - initout);
}


// like match_v4_f2_p0 but more portable
size_t V1
(const uint32_t *rare, size_t lenRare,
 const uint32_t *freq, size_t lenFreq,
 uint32_t *matchOut) {
    assert(lenRare <= lenFreq);
    const uint32_t *matchOrig = matchOut;
    if (lenFreq == 0 || lenRare == 0) return 0;

    const uint64_t kFreqSpace = 2 * 4 * (0 + 1) - 1;
    const uint64_t kRareSpace = 0;

    const uint32_t *stopFreq = &freq[lenFreq] - kFreqSpace;
    const uint32_t *stopRare = &rare[lenRare] - kRareSpace;

    __m128i  Rare;

    __m128i F0, F1;

    if (COMPILER_RARELY( (rare >= stopRare) || (freq >= stopFreq) )) goto FINISH_SCALAR;
    uint32_t valRare;
    valRare = rare[0];
    Rare = _mm_set1_epi32(valRare);

    uint64_t maxFreq;
    maxFreq = freq[2 * 4 - 1];
    F0 = _mm_lddqu_si128(reinterpret_cast<const __m128i *>(freq));
    F1 = _mm_lddqu_si128(reinterpret_cast<const __m128i *>(freq+4));


    if (COMPILER_RARELY(maxFreq < valRare)) goto ADVANCE_FREQ;

ADVANCE_RARE:
    do {
        *matchOut = valRare;
        valRare = rare[1]; // for next iteration
        rare += 1;
        if (COMPILER_RARELY(rare >= stopRare)) {
            rare -= 1;
            goto FINISH_SCALAR;
        }
        F0 =  _mm_cmpeq_epi32(F0,Rare); 
        F1 =  _mm_cmpeq_epi32(F1,Rare);
        Rare = _mm_set1_epi32(valRare);
        F0 = _mm_or_si128 (F0,F1);
#ifdef __SSE4_1__
        if(_mm_testz_si128(F0,F0) == 0)
          matchOut ++;
#else
        if(_mm_movemask_epi8(F0))
          matchOut ++;
#endif
        F0 = _mm_lddqu_si128(reinterpret_cast<const __m128i *>(freq));
        F1 = _mm_lddqu_si128(reinterpret_cast<const __m128i *>(freq+4));

    } while (maxFreq >= valRare);

    uint64_t maxProbe;

ADVANCE_FREQ:
    do {
        const uint64_t kProbe = (0 + 1) * 2 * 4;
        const uint32_t *probeFreq = freq + kProbe;
        maxProbe = freq[(0 + 2) * 2 * 4 - 1];

        if (COMPILER_RARELY(probeFreq >= stopFreq)) {
            goto FINISH_SCALAR;
        }

        freq = probeFreq;

    } while (maxProbe < valRare);

    maxFreq = maxProbe;

    F0 = _mm_lddqu_si128(reinterpret_cast<const __m128i *>(freq));
    F1 = _mm_lddqu_si128(reinterpret_cast<const __m128i *>(freq+4));
 

    goto ADVANCE_RARE;

    size_t count;
FINISH_SCALAR:
    count = matchOut - matchOrig;

    lenFreq = stopFreq + kFreqSpace - freq;
    lenRare = stopRare + kRareSpace - rare;

    size_t tail = match_scalar(freq, lenFreq, rare, lenRare, matchOut);

    return count + tail;
}

#ifdef __AVX2__
#include <immintrin.h>

// like match_v4_f2_p0 but more portable
size_t V1AVX
(const uint32_t *rare, size_t lenRare,
 const uint32_t *freq, size_t lenFreq,
 uint32_t *matchOut) {
    assert(lenRare <= lenFreq);
    const uint32_t *matchOrig = matchOut;
    if (lenFreq == 0 || lenRare == 0) return 0;

    const uint64_t kFreqSpace = 2 * 4 * (0 + 1) - 1;
    const uint64_t kRareSpace = 0;

    const uint32_t *stopFreq = &freq[lenFreq] - kFreqSpace;
    const uint32_t *stopRare = &rare[lenRare] - kRareSpace;

    __m256i  Rare;

    __m256i F;

    if (COMPILER_RARELY( (rare >= stopRare) || (freq >= stopFreq) )) goto FINISH_SCALAR;
    uint32_t valRare;
    valRare = rare[0];
    Rare = _mm256_set1_epi32(valRare);

    uint64_t maxFreq;
    maxFreq = freq[2 * 4 - 1];
    F = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(freq));


    if (COMPILER_RARELY(maxFreq < valRare)) goto ADVANCE_FREQ;

ADVANCE_RARE:
    do {
        *matchOut = valRare;
        valRare = rare[1]; // for next iteration
        rare += 1;
        if (COMPILER_RARELY(rare >= stopRare)) {
            rare -= 1;
            goto FINISH_SCALAR;
        }
        F =  _mm256_cmpeq_epi32(F,Rare);
        Rare = _mm256_set1_epi32(valRare);
        if(_mm256_testz_si256(F,F) == 0)
          matchOut ++;
        F = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(freq));

    } while (maxFreq >= valRare);

    uint64_t maxProbe;

ADVANCE_FREQ:
    do {
        const uint64_t kProbe = (0 + 1) * 2 * 4;
        const uint32_t *probeFreq = freq + kProbe;
        maxProbe = freq[(0 + 2) * 2 * 4 - 1];

        if (COMPILER_RARELY(probeFreq >= stopFreq)) {
            goto FINISH_SCALAR;
        }

        freq = probeFreq;

    } while (maxProbe < valRare);

    maxFreq = maxProbe;

    F = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(freq));


    goto ADVANCE_RARE;

    size_t count;
FINISH_SCALAR:
    count = matchOut - matchOrig;

    lenFreq = stopFreq + kFreqSpace - freq;
    lenRare = stopRare + kRareSpace - rare;

    size_t tail = match_scalar(freq, lenFreq, rare, lenRare, matchOut);

    return count + tail;
}

#endif
size_t match_v4_f2_p0
(const uint32_t *rare, size_t lenRare,
 const uint32_t *freq, size_t lenFreq,
 uint32_t *matchOut) {
    assert(lenRare <= lenFreq);
    const uint32_t *matchOrig = matchOut;
    if (lenFreq == 0 || lenRare == 0) return 0;

    const uint64_t kFreqSpace = 2 * 4 * (0 + 1) - 1;
    const uint64_t kRareSpace = 0;

    const uint32_t *stopFreq = &freq[lenFreq] - kFreqSpace;
    const uint32_t *stopRare = &rare[lenRare] - kRareSpace;

    VEC_T Rare;

    VEC_T F0, F1;

    if (COMPILER_RARELY( (rare >= stopRare) || (freq >= stopFreq) )) goto FINISH_SCALAR;
    uint32_t valRare;
    valRare = rare[0];
    VEC_SET_ALL_TO_INT(Rare, valRare);

    uint64_t maxFreq;
    maxFreq = freq[2 * 4 - 1];
    VEC_LOAD_OFFSET(F0, freq, 0 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F1, freq, 1 * sizeof(VEC_T));

    if (COMPILER_RARELY(maxFreq < valRare)) goto ADVANCE_FREQ;

ADVANCE_RARE:
    do {
        *matchOut = valRare;
        valRare = rare[1]; // for next iteration
        ASM_LEA_ADD_BYTES(rare, sizeof(*rare)); // rare += 1;

        if (COMPILER_RARELY(rare >= stopRare)) {
            rare -= 1;
            goto FINISH_SCALAR;
        }

        VEC_CMP_EQUAL(F0, Rare) ;
        VEC_CMP_EQUAL(F1, Rare);

        VEC_SET_ALL_TO_INT(Rare, valRare);

        VEC_OR(F0, F1);

        VEC_ADD_PTEST(matchOut, 1, F0);

        VEC_LOAD_OFFSET(F0, freq, 0 * sizeof(VEC_T)) ;
        VEC_LOAD_OFFSET(F1, freq, 1 * sizeof(VEC_T));

    } while (maxFreq >= valRare);

    uint64_t maxProbe;

ADVANCE_FREQ:
    do {
        const uint64_t kProbe = (0 + 1) * 2 * 4;
        const uint32_t *probeFreq = freq + kProbe;
        maxProbe = freq[(0 + 2) * 2 * 4 - 1];

        if (COMPILER_RARELY(probeFreq >= stopFreq)) {
            goto FINISH_SCALAR;
        }

        freq = probeFreq;

    } while (maxProbe < valRare);

    maxFreq = maxProbe;

    VEC_LOAD_OFFSET(F0, freq, 0 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F1, freq, 1 * sizeof(VEC_T));

    goto ADVANCE_RARE;

    size_t count;
FINISH_SCALAR:
    count = matchOut - matchOrig;

    lenFreq = stopFreq + kFreqSpace - freq;
    lenRare = stopRare + kRareSpace - rare;

    size_t tail = match_scalar(freq, lenFreq, rare, lenRare, matchOut);

    return count + tail;
}


size_t match_v4_f4_p0
(const uint32_t *rare, size_t lenRare,
 const uint32_t *freq, size_t lenFreq,
 uint32_t *matchOut) {

    const uint32_t *matchOrig = matchOut;
    if (lenFreq == 0 || lenRare == 0) return 0;

    const uint64_t kFreqSpace = 4 * 4 * (0 + 1) - 1;
    const uint64_t kRareSpace = 0;

    const uint32_t *stopFreq = &freq[lenFreq] - kFreqSpace;
    const uint32_t *stopRare = &rare[lenRare] - kRareSpace;

    VEC_T Rare;

    VEC_T F0, F1, F2, F3;

    if (COMPILER_RARELY( (rare >= stopRare) || (freq >= stopFreq) )) goto FINISH_SCALAR;
    uint32_t valRare;
    valRare = rare[0];
    VEC_SET_ALL_TO_INT(Rare, valRare);

    uint64_t maxFreq;
    maxFreq = freq[4 * 4 - 1];
    VEC_LOAD_OFFSET(F0, freq, 0 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F1, freq, 1 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F2, freq, 2 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F3, freq, 3 * sizeof(VEC_T));

    if (COMPILER_RARELY(maxFreq < valRare)) goto ADVANCE_FREQ;

ADVANCE_RARE:
    do {
        *matchOut = valRare;
        valRare = rare[1]; // for next iteration
        ASM_LEA_ADD_BYTES(rare, sizeof(*rare)); // rare += 1;

        if (COMPILER_RARELY(rare >= stopRare)) {
            rare -= 1;
            goto FINISH_SCALAR;
        }

        VEC_CMP_EQUAL(F0, Rare) ;
        VEC_CMP_EQUAL(F1, Rare) ;
        VEC_CMP_EQUAL(F2, Rare) ;
        VEC_CMP_EQUAL(F3, Rare);

        VEC_SET_ALL_TO_INT(Rare, valRare);

        VEC_OR(F0, F1);
        VEC_OR(F2, F3);
        VEC_OR(F0, F2);;

        VEC_ADD_PTEST(matchOut, 1, F0);

        VEC_LOAD_OFFSET(F0, freq, 0 * sizeof(VEC_T)) ;
        VEC_LOAD_OFFSET(F1, freq, 1 * sizeof(VEC_T)) ;
        VEC_LOAD_OFFSET(F2, freq, 2 * sizeof(VEC_T)) ;
        VEC_LOAD_OFFSET(F3, freq, 3 * sizeof(VEC_T));

    } while (maxFreq >= valRare);

    uint64_t maxProbe;

ADVANCE_FREQ:
    do {
        const uint64_t kProbe = (0 + 1) * 4 * 4;
        const uint32_t *probeFreq = freq + kProbe;
        maxProbe = freq[(0 + 2) * 4 * 4 - 1];

        if (COMPILER_RARELY(probeFreq >= stopFreq)) {
            goto FINISH_SCALAR;
        }

        freq = probeFreq;

    } while (maxProbe < valRare);

    maxFreq = maxProbe;

    VEC_LOAD_OFFSET(F0, freq, 0 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F1, freq, 1 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F2, freq, 2 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F3, freq, 3 * sizeof(VEC_T));

    goto ADVANCE_RARE;

    size_t count;
FINISH_SCALAR:
    count = matchOut - matchOrig;

    lenFreq = stopFreq + kFreqSpace - freq;
    lenRare = stopRare + kRareSpace - rare;

    size_t tail = match_scalar(freq, lenFreq, rare, lenRare, matchOut);

    return count + tail;
}

size_t match_v4_f8_p0
(const uint32_t *rare, size_t lenRare,
 const uint32_t *freq, size_t lenFreq,
 uint32_t *matchOut) {

    const uint32_t *matchOrig = matchOut;
    if (lenFreq == 0 || lenRare == 0) return 0;

    const uint64_t kFreqSpace = 8 * 4 * (0 + 1) - 1;
    const uint64_t kRareSpace = 0;

    const uint32_t *stopFreq = &freq[lenFreq] - kFreqSpace;
    const uint32_t *stopRare = &rare[lenRare] - kRareSpace;

    VEC_T Rare;

    VEC_T F0, F1, F2, F3, F4, F5, F6, F7;

    if (COMPILER_RARELY( (rare >= stopRare) || (freq >= stopFreq) )) goto FINISH_SCALAR;
    uint64_t valRare;
    valRare = rare[0];
    VEC_SET_ALL_TO_INT(Rare, valRare);

    uint64_t maxFreq;
    maxFreq = freq[8 * 4 - 1];
    VEC_LOAD_OFFSET(F0, freq, 0 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F1, freq, 1 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F2, freq, 2 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F3, freq, 3 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F4, freq, 4 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F5, freq, 5 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F6, freq, 6 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F7, freq, 7 * sizeof(VEC_T));

    if (COMPILER_RARELY(maxFreq < valRare)) goto ADVANCE_FREQ;


ADVANCE_RARE:
    do {
        *matchOut = static_cast<uint32_t>(valRare);
        valRare = rare[1]; // for next iteration
        ASM_LEA_ADD_BYTES(rare, sizeof(*rare)); // rare += 1;

        if (COMPILER_RARELY(rare >= stopRare)) {
            rare -= 1;
            goto FINISH_SCALAR;
        }

        VEC_CMP_EQUAL(F0, Rare) ;
        VEC_CMP_EQUAL(F1, Rare) ;
        VEC_CMP_EQUAL(F2, Rare) ;
        VEC_CMP_EQUAL(F3, Rare) ;
        VEC_CMP_EQUAL(F4, Rare) ;
        VEC_CMP_EQUAL(F5, Rare) ;
        VEC_CMP_EQUAL(F6, Rare) ;
        VEC_CMP_EQUAL(F7, Rare);


        VEC_SET_ALL_TO_INT(Rare, valRare);

        VEC_OR(F0, F1);
        VEC_OR(F2, F3);
        VEC_OR(F4, F5);
        VEC_OR(F6, F7);
        VEC_OR(F0, F2);
        VEC_OR(F4, F6);
        VEC_OR(F0, F4);

        VEC_ADD_PTEST(matchOut, 1, F0);

        VEC_LOAD_OFFSET(F0, freq, 0 * sizeof(VEC_T)) ;
        VEC_LOAD_OFFSET(F1, freq, 1 * sizeof(VEC_T)) ;
        VEC_LOAD_OFFSET(F2, freq, 2 * sizeof(VEC_T)) ;
        VEC_LOAD_OFFSET(F3, freq, 3 * sizeof(VEC_T)) ;
        VEC_LOAD_OFFSET(F4, freq, 4 * sizeof(VEC_T)) ;
        VEC_LOAD_OFFSET(F5, freq, 5 * sizeof(VEC_T)) ;
        VEC_LOAD_OFFSET(F6, freq, 6 * sizeof(VEC_T)) ;
        VEC_LOAD_OFFSET(F7, freq, 7 * sizeof(VEC_T));

    } while (maxFreq >= valRare);

    uint64_t maxProbe;

ADVANCE_FREQ:
    do {
        const uint64_t kProbe = (0 + 1) * 8 * 4;
        const uint32_t *probeFreq = freq + kProbe;
        maxProbe = freq[(0 + 2) * 8 * 4 - 1];

        if (COMPILER_RARELY(probeFreq >= stopFreq)) {
            goto FINISH_SCALAR;
        }

        freq = probeFreq;

    } while (maxProbe < valRare);

    maxFreq = maxProbe;

    VEC_LOAD_OFFSET(F0, freq, 0 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F1, freq, 1 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F2, freq, 2 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F3, freq, 3 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F4, freq, 4 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F5, freq, 5 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F6, freq, 6 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F7, freq, 7 * sizeof(VEC_T));

    goto ADVANCE_RARE;

    size_t count;
FINISH_SCALAR:
    count = matchOut - matchOrig;

    lenFreq = stopFreq + kFreqSpace - freq;
    lenRare = stopRare + kRareSpace - rare;

    size_t tail = match_scalar(freq, lenFreq, rare, lenRare, matchOut);
    return count + tail;
}



// unsafe, assumes signed ints
size_t match_v4_f2_p1
(const uint32_t *rare, size_t lenRare,
 const uint32_t *freq, size_t lenFreq,
 uint32_t *matchOut) {

    const uint32_t *matchOrig = matchOut;
    if (lenFreq == 0 || lenRare == 0) return 0;

    const uint64_t kFreqSpace = 2 * 4 * (1 + 1) - 1;
    const uint64_t kRareSpace = 0;

    const uint32_t *stopFreq = &freq[lenFreq] - kFreqSpace;
    const uint32_t *stopRare = &rare[lenRare] - kRareSpace;

    VEC_T Rare;
    VEC_T F0, F1;

    if (COMPILER_RARELY( (rare >= stopRare) || (freq >= stopFreq) )) goto FINISH_SCALAR;
    uint64_t valRare;
    valRare = rare[0];
    VEC_SET_ALL_TO_INT(Rare, valRare);

    uint64_t maxFreq;
    maxFreq = freq[2 * 4 - 1];
    VEC_LOAD_OFFSET(F0, freq, 0 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F1, freq, 1 * sizeof(VEC_T));

    if (COMPILER_RARELY(maxFreq < valRare)) goto ADVANCE_FREQ;

ADVANCE_RARE:
    do {
        *matchOut = static_cast<uint32_t>(valRare);
        valRare = rare[1]; // for next iteration
        ASM_LEA_ADD_BYTES(rare, sizeof(*rare)); // rare += 1;

        if (COMPILER_RARELY(rare >= stopRare)) {
            rare -= 1;
            goto FINISH_SCALAR;
        }

        VEC_CMP_EQUAL(F0, Rare) ;
        VEC_CMP_EQUAL(F1, Rare);

        VEC_SET_ALL_TO_INT(Rare, valRare);

        VEC_OR(F0, F1);

        VEC_ADD_PTEST(matchOut, 1, F0);

        VEC_LOAD_OFFSET(F0, freq, 0 * sizeof(VEC_T)) ;
        VEC_LOAD_OFFSET(F1, freq, 1 * sizeof(VEC_T));

    } while (maxFreq >= valRare);


    VEC_T P0;
    uint64_t maxProbe;

ADVANCE_FREQ:
    do {
        const uint64_t kProbe = (1 + 1) * 2 * 4;
        const uint32_t *probeFreq = freq + kProbe;
        maxProbe = freq[(1 + 2) * 2 * 4 - 1];

        VEC_LOAD_OFFSET(P0, freq, (0 + 2) * 2 * sizeof(VEC_T));

        if (COMPILER_RARELY(probeFreq >= stopFreq)) {
            goto FINISH_SCALAR;
        }

        freq = probeFreq;

    } while (maxProbe < valRare);

    VEC_CMP_GREATER(P0, Rare);

    int32_t back;
    back = 0; // NOTE: back must be signed
    {
        int32_t _probe;
        VEC_COPY_LOW(_probe, P0);
        back += _probe;
    };

    back *= 2 * 4;
    freq += back;
    maxFreq = freq[2 * 4 - 1];

    VEC_LOAD_OFFSET(F0, freq, 0 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F1, freq, 1 * sizeof(VEC_T));

    goto ADVANCE_RARE;

    size_t count;
FINISH_SCALAR:
    count = matchOut - matchOrig;

    lenFreq = stopFreq + kFreqSpace - freq;
    lenRare = stopRare + kRareSpace - rare;

    size_t tail = match_scalar(freq, lenFreq, rare, lenRare, matchOut);

    return count + tail;
}


// unsafe, assumes signed ints
size_t match_v4_f4_p1
(const uint32_t *rare, size_t lenRare,
 const uint32_t *freq, size_t lenFreq,
 uint32_t *matchOut) {

    const uint32_t *matchOrig = matchOut;
    if (lenFreq == 0 || lenRare == 0) return 0;

    const uint64_t kFreqSpace = 4 * 4 * (1 + 1) - 1;
    const uint64_t kRareSpace = 0;

    const uint32_t *stopFreq = &freq[lenFreq] - kFreqSpace;
    const uint32_t *stopRare = &rare[lenRare] - kRareSpace;

    VEC_T Rare;

    VEC_T F0, F1, F2, F3;

    if (COMPILER_RARELY( (rare >= stopRare) || (freq >= stopFreq) )) goto FINISH_SCALAR;
    uint64_t valRare;
    valRare = rare[0];
    VEC_SET_ALL_TO_INT(Rare, valRare);

    uint64_t maxFreq;
    maxFreq = freq[4 * 4 - 1];
    VEC_LOAD_OFFSET(F0, freq, 0 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F1, freq, 1 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F2, freq, 2 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F3, freq, 3 * sizeof(VEC_T));

    if (COMPILER_RARELY(maxFreq < valRare)) goto ADVANCE_FREQ;

ADVANCE_RARE:
    do {

        *matchOut = static_cast<uint32_t>(valRare);
        valRare = rare[1]; // for next iteration
        ASM_LEA_ADD_BYTES(rare, sizeof(*rare)); // rare += 1;

        if (COMPILER_RARELY(rare >= stopRare)) {
            rare -= 1;
            goto FINISH_SCALAR;
        }

        VEC_CMP_EQUAL(F0, Rare) ;
        VEC_CMP_EQUAL(F1, Rare) ;
        VEC_CMP_EQUAL(F2, Rare) ;
        VEC_CMP_EQUAL(F3, Rare);

        VEC_SET_ALL_TO_INT(Rare, valRare);

        // FUTURE: use a list macro
        VEC_OR(F0, F1);
        VEC_OR(F2, F3);
        VEC_OR(F0, F2);;

        VEC_ADD_PTEST(matchOut, 1, F0);

        VEC_LOAD_OFFSET(F0, freq, 0 * sizeof(VEC_T)) ;
        VEC_LOAD_OFFSET(F1, freq, 1 * sizeof(VEC_T)) ;
        VEC_LOAD_OFFSET(F2, freq, 2 * sizeof(VEC_T)) ;
        VEC_LOAD_OFFSET(F3, freq, 3 * sizeof(VEC_T));

    } while (maxFreq >= valRare);


    VEC_T P0;
    uint64_t maxProbe;

ADVANCE_FREQ:
    do {
        const uint64_t kProbe = (1 + 1) * 4 * 4;
        const uint32_t *probeFreq = freq + kProbe;
        maxProbe = freq[(1 + 2) * 4 * 4 - 1];

        VEC_LOAD_OFFSET(P0, freq, (0 + 2) * 4 * sizeof(VEC_T));

        if (COMPILER_RARELY(probeFreq >= stopFreq)) {
            goto FINISH_SCALAR;
        }

        freq = probeFreq;

    } while (maxProbe < valRare);

    VEC_CMP_GREATER(P0, Rare);

    int32_t back;
    back = 0; // NOTE: back must be signed
    {
        int32_t _probe;
        VEC_COPY_LOW(_probe, P0);
        back += _probe;
    };

    back *= 4 * 4;
    freq += back;
    maxFreq = freq[4 * 4 - 1];

    VEC_LOAD_OFFSET(F0, freq, 0 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F1, freq, 1 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F2, freq, 2 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F3, freq, 3 * sizeof(VEC_T));

    goto ADVANCE_RARE;

    size_t count;
FINISH_SCALAR:
    count = matchOut - matchOrig;

    lenFreq = stopFreq + kFreqSpace - freq;
    lenRare = stopRare + kRareSpace - rare;

    size_t tail = match_scalar(freq, lenFreq, rare, lenRare, matchOut);

    return count + tail;
}

// unsafe, assumes signed ints
size_t match_v4_f8_p1
(const uint32_t *rare, size_t lenRare,
 const uint32_t *freq, size_t lenFreq,
 uint32_t *matchOut) {

    const uint32_t *matchOrig = matchOut;
    if (lenFreq == 0 || lenRare == 0) return 0;

    const uint64_t kFreqSpace = 8 * 4 * (1 + 1) - 1;
    const uint64_t kRareSpace = 0;

    const uint32_t *stopFreq = &freq[lenFreq] - kFreqSpace;
    const uint32_t *stopRare = &rare[lenRare] - kRareSpace;

    VEC_T Rare;

    VEC_T F0, F1, F2, F3, F4, F5, F6, F7;

    if (COMPILER_RARELY( (rare >= stopRare) || (freq >= stopFreq) )) goto FINISH_SCALAR;
    uint64_t valRare;
    valRare = rare[0];
    VEC_SET_ALL_TO_INT(Rare, valRare);

    uint64_t maxFreq;
    maxFreq = freq[8 * 4 - 1];
    VEC_LOAD_OFFSET(F0, freq, 0 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F1, freq, 1 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F2, freq, 2 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F3, freq, 3 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F4, freq, 4 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F5, freq, 5 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F6, freq, 6 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F7, freq, 7 * sizeof(VEC_T));

    if (COMPILER_RARELY(maxFreq < valRare)) goto ADVANCE_FREQ;

ADVANCE_RARE:
    do {
        *matchOut = static_cast<uint32_t>(valRare);
        valRare = rare[1]; // for next iteration
        ASM_LEA_ADD_BYTES(rare, sizeof(*rare)); // rare += 1;

        if (COMPILER_RARELY(rare >= stopRare)) {
            rare -= 1;
            goto FINISH_SCALAR;
        }

        VEC_CMP_EQUAL(F0, Rare) ;
        VEC_CMP_EQUAL(F1, Rare) ;
        VEC_CMP_EQUAL(F2, Rare) ;
        VEC_CMP_EQUAL(F3, Rare) ;
        VEC_CMP_EQUAL(F4, Rare) ;
        VEC_CMP_EQUAL(F5, Rare) ;
        VEC_CMP_EQUAL(F6, Rare) ;
        VEC_CMP_EQUAL(F7, Rare);

        VEC_SET_ALL_TO_INT(Rare, valRare);

        VEC_OR(F0, F1);
        VEC_OR(F2, F3);
        VEC_OR(F4, F5);
        VEC_OR(F6, F7);
        VEC_OR(F0, F2);
        VEC_OR(F4, F6);
        VEC_OR(F0, F4);

        VEC_ADD_PTEST(matchOut, 1, F0);

        VEC_LOAD_OFFSET(F0, freq, 0 * sizeof(VEC_T)) ;
        VEC_LOAD_OFFSET(F1, freq, 1 * sizeof(VEC_T)) ;
        VEC_LOAD_OFFSET(F2, freq, 2 * sizeof(VEC_T)) ;
        VEC_LOAD_OFFSET(F3, freq, 3 * sizeof(VEC_T)) ;
        VEC_LOAD_OFFSET(F4, freq, 4 * sizeof(VEC_T)) ;
        VEC_LOAD_OFFSET(F5, freq, 5 * sizeof(VEC_T)) ;
        VEC_LOAD_OFFSET(F6, freq, 6 * sizeof(VEC_T)) ;
        VEC_LOAD_OFFSET(F7, freq, 7 * sizeof(VEC_T));

    } while (maxFreq >= valRare);


    VEC_T P0;
    uint64_t maxProbe;

ADVANCE_FREQ:
    do {
        const uint64_t kProbe = (1 + 1) * 8 * 4;
        const uint32_t *probeFreq = freq + kProbe;
        maxProbe = freq[(1 + 2) * 8 * 4 - 1];

        VEC_LOAD_OFFSET(P0, freq, (0 + 2) * 8 * sizeof(VEC_T));

        if (COMPILER_RARELY(probeFreq >= stopFreq)) {
            goto FINISH_SCALAR;
        }

        freq = probeFreq;

    } while (maxProbe < valRare);

    VEC_CMP_GREATER(P0, Rare);

    int32_t back;
    back = 0; // NOTE: back must be signed
    {
        int32_t _probe;
        VEC_COPY_LOW(_probe, P0);
        back += _probe;
    };

    back *= 8 * 4;
    freq += back;
    maxFreq = freq[8 * 4 - 1];

    VEC_LOAD_OFFSET(F0, freq, 0 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F1, freq, 1 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F2, freq, 2 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F3, freq, 3 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F4, freq, 4 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F5, freq, 5 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F6, freq, 6 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F7, freq, 7 * sizeof(VEC_T));

    goto ADVANCE_RARE;

    size_t count;
FINISH_SCALAR:
    count = matchOut - matchOrig;

    lenFreq = stopFreq + kFreqSpace - freq;
    lenRare = stopRare + kRareSpace - rare;

    size_t tail = match_scalar(freq, lenFreq, rare, lenRare, matchOut);

    return count + tail;
}

// unsafe, assumes signed ints
size_t match_v4_f2_p2
(const uint32_t *rare, size_t lenRare,
 const uint32_t *freq, size_t lenFreq,
 uint32_t *matchOut) {

    const uint32_t *matchOrig = matchOut;
    if (lenFreq == 0 || lenRare == 0) return 0;

    const uint64_t kFreqSpace = 2 * 4 * (2 + 1) - 1;
    const uint64_t kRareSpace = 0;

    const uint32_t *stopFreq = &freq[lenFreq] - kFreqSpace;
    const uint32_t *stopRare = &rare[lenRare] - kRareSpace;

    VEC_T Rare;

    VEC_T F0, F1;

    if (COMPILER_RARELY( (rare >= stopRare) || (freq >= stopFreq) )) goto FINISH_SCALAR;
    uint64_t valRare;
    valRare = rare[0];
    VEC_SET_ALL_TO_INT(Rare, valRare);

    uint64_t maxFreq;
    maxFreq = freq[2 * 4 - 1];
    VEC_LOAD_OFFSET(F0, freq, 0 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F1, freq, 1 * sizeof(VEC_T));

    if (COMPILER_RARELY(maxFreq < valRare)) goto ADVANCE_FREQ;

ADVANCE_RARE:
    do {
        *matchOut = static_cast<uint32_t>(valRare);
        valRare = rare[1]; // for next iteration
        ASM_LEA_ADD_BYTES(rare, sizeof(*rare)); // rare += 1;

        if (COMPILER_RARELY(rare >= stopRare)) {
            rare -= 1;
            goto FINISH_SCALAR;
        }

        VEC_CMP_EQUAL(F0, Rare) ;
        VEC_CMP_EQUAL(F1, Rare);

        VEC_SET_ALL_TO_INT(Rare, valRare);

        VEC_OR(F0, F1);

        VEC_ADD_PTEST(matchOut, 1, F0);

        VEC_LOAD_OFFSET(F0, freq, 0 * sizeof(VEC_T)) ;
        VEC_LOAD_OFFSET(F1, freq, 1 * sizeof(VEC_T));

    } while (maxFreq >= valRare);


    VEC_T P0 ;
    VEC_T P1;
    uint64_t maxProbe;

ADVANCE_FREQ:
    do {
        const uint64_t kProbe = (2 + 1) * 2 * 4;
        const uint32_t *probeFreq = freq + kProbe;
        maxProbe = freq[(2 + 2) * 2 * 4 - 1];

        VEC_LOAD_OFFSET(P0, freq, (0 + 2) * 2 * sizeof(VEC_T)) ;
        VEC_LOAD_OFFSET(P1, freq, (1 + 2) * 2 * sizeof(VEC_T));

        if (COMPILER_RARELY(probeFreq >= stopFreq)) {
            goto FINISH_SCALAR;
        }

        freq = probeFreq;

    } while (maxProbe < valRare);

    VEC_CMP_GREATER(P0, Rare) ;
    VEC_CMP_GREATER(P1, Rare);

    int32_t back;
    back = 0; // NOTE: back must be signed
    {
        int32_t _probe;
        VEC_COPY_LOW(_probe, P0);
        back += _probe;
    } ;
    {
        int32_t _probe;
        VEC_COPY_LOW(_probe, P1);
        back += _probe;
    };

    back *= 2 * 4;
    freq += back;
    maxFreq = freq[2 * 4 - 1];

    VEC_LOAD_OFFSET(F0, freq, 0 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F1, freq, 1 * sizeof(VEC_T));

    goto ADVANCE_RARE;

    size_t count;
FINISH_SCALAR:
    count = matchOut - matchOrig;

    lenFreq = stopFreq + kFreqSpace - freq;
    lenRare = stopRare + kRareSpace - rare;

    size_t tail = match_scalar(freq, lenFreq, rare, lenRare, matchOut);

    return count + tail;
}


// unsafe, assumes signed ints
size_t match_v4_f4_p2
(const uint32_t *rare, size_t lenRare,
 const uint32_t *freq, size_t lenFreq,
 uint32_t *matchOut) {

    const uint32_t *matchOrig = matchOut;
    if (lenFreq == 0 || lenRare == 0) return 0;

    const uint64_t kFreqSpace = 4 * 4 * (2 + 1) - 1;
    const uint64_t kRareSpace = 0;

    const uint32_t *stopFreq = &freq[lenFreq] - kFreqSpace;
    const uint32_t *stopRare = &rare[lenRare] - kRareSpace;

    VEC_T Rare;

    VEC_T F0, F1, F2, F3;

    if (COMPILER_RARELY( (rare >= stopRare) || (freq >= stopFreq) )) goto FINISH_SCALAR;
    uint64_t valRare;
    valRare = rare[0];
    VEC_SET_ALL_TO_INT(Rare, valRare);

    uint64_t maxFreq;
    maxFreq = freq[4 * 4 - 1];
    VEC_LOAD_OFFSET(F0, freq, 0 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F1, freq, 1 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F2, freq, 2 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F3, freq, 3 * sizeof(VEC_T));

    if (COMPILER_RARELY(maxFreq < valRare)) goto ADVANCE_FREQ;

ADVANCE_RARE:
    do {
        *matchOut = static_cast<uint32_t>(valRare);
        valRare = rare[1]; // for next iteration
        ASM_LEA_ADD_BYTES(rare, sizeof(*rare)); // rare += 1;

        if (COMPILER_RARELY(rare >= stopRare)) {
            rare -= 1;
            goto FINISH_SCALAR;
        }

        VEC_CMP_EQUAL(F0, Rare) ;
        VEC_CMP_EQUAL(F1, Rare) ;
        VEC_CMP_EQUAL(F2, Rare) ;
        VEC_CMP_EQUAL(F3, Rare);

        VEC_SET_ALL_TO_INT(Rare, valRare);

        VEC_OR(F0, F1);
        VEC_OR(F2, F3);
        VEC_OR(F0, F2);;

        VEC_ADD_PTEST(matchOut, 1, F0);

        VEC_LOAD_OFFSET(F0, freq, 0 * sizeof(VEC_T)) ;
        VEC_LOAD_OFFSET(F1, freq, 1 * sizeof(VEC_T)) ;
        VEC_LOAD_OFFSET(F2, freq, 2 * sizeof(VEC_T)) ;
        VEC_LOAD_OFFSET(F3, freq, 3 * sizeof(VEC_T));

    } while (maxFreq >= valRare);

    VEC_T P0, P1;
    uint64_t maxProbe;

ADVANCE_FREQ:
    do {
        const uint64_t kProbe = (2 + 1) * 4 * 4;
        const uint32_t *probeFreq = freq + kProbe;
        maxProbe = freq[(2 + 2) * 4 * 4 - 1];

        VEC_LOAD_OFFSET(P0, freq, (0 + 2) * 4 * sizeof(VEC_T)) ;
        VEC_LOAD_OFFSET(P1, freq, (1 + 2) * 4 * sizeof(VEC_T));

        if (COMPILER_RARELY(probeFreq >= stopFreq)) {
            goto FINISH_SCALAR;
        }

        freq = probeFreq;

    } while (maxProbe < valRare);

    VEC_CMP_GREATER(P0, Rare) ;
    VEC_CMP_GREATER(P1, Rare);

    int32_t back;
    back = 0; // NOTE: back must be signed
    {
        int32_t _probe;
        VEC_COPY_LOW(_probe, P0);
        back += _probe;
    } ;
    {
        int32_t _probe;
        VEC_COPY_LOW(_probe, P1);
        back += _probe;
    };

    back *= 4 * 4;
    freq += back;
    maxFreq = freq[4 * 4 - 1];

    VEC_LOAD_OFFSET(F0, freq, 0 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F1, freq, 1 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F2, freq, 2 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F3, freq, 3 * sizeof(VEC_T));

    goto ADVANCE_RARE;

    size_t count;
FINISH_SCALAR:
    count = matchOut - matchOrig;

    lenFreq = stopFreq + kFreqSpace - freq;
    lenRare = stopRare + kRareSpace - rare;

    size_t tail = match_scalar(freq, lenFreq, rare, lenRare, matchOut);

    return count + tail;
}

// unsafe, assumes signed ints
size_t match_v4_f8_p2
(const uint32_t *rare, size_t lenRare,
 const uint32_t *freq, size_t lenFreq,
 uint32_t *matchOut) {

    const uint32_t *matchOrig = matchOut;
    if (lenFreq == 0 || lenRare == 0) return 0;

    const uint64_t kFreqSpace = 8 * 4 * (2 + 1) - 1;
    const uint64_t kRareSpace = 0;

    const uint32_t *stopFreq = &freq[lenFreq] - kFreqSpace;
    const uint32_t *stopRare = &rare[lenRare] - kRareSpace;

    VEC_T Rare;

    VEC_T F0, F1, F2, F3, F4, F5, F6, F7;

    if (COMPILER_RARELY( (rare >= stopRare) || (freq >= stopFreq) )) goto FINISH_SCALAR;
    uint64_t valRare;
    valRare = rare[0];
    VEC_SET_ALL_TO_INT(Rare, valRare);

    uint64_t maxFreq;
    maxFreq = freq[8 * 4 - 1];
    VEC_LOAD_OFFSET(F0, freq, 0 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F1, freq, 1 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F2, freq, 2 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F3, freq, 3 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F4, freq, 4 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F5, freq, 5 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F6, freq, 6 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F7, freq, 7 * sizeof(VEC_T));

    if (COMPILER_RARELY(maxFreq < valRare)) goto ADVANCE_FREQ;

ADVANCE_RARE:
    do {
        *matchOut = static_cast<uint32_t>(valRare);
        valRare = rare[1]; // for next iteration
        ASM_LEA_ADD_BYTES(rare, sizeof(*rare)); // rare += 1;

        if (COMPILER_RARELY(rare >= stopRare)) {
            rare -= 1;
            goto FINISH_SCALAR;
        }

        VEC_CMP_EQUAL(F0, Rare) ;
        VEC_CMP_EQUAL(F1, Rare) ;
        VEC_CMP_EQUAL(F2, Rare) ;
        VEC_CMP_EQUAL(F3, Rare) ;
        VEC_CMP_EQUAL(F4, Rare) ;
        VEC_CMP_EQUAL(F5, Rare) ;
        VEC_CMP_EQUAL(F6, Rare) ;
        VEC_CMP_EQUAL(F7, Rare);

        VEC_SET_ALL_TO_INT(Rare, valRare);

        VEC_OR(F0, F1);
        VEC_OR(F2, F3);
        VEC_OR(F4, F5);
        VEC_OR(F6, F7);
        VEC_OR(F0, F2);
        VEC_OR(F4, F6);
        VEC_OR(F0, F4);

        VEC_ADD_PTEST(matchOut, 1, F0);

        VEC_LOAD_OFFSET(F0, freq, 0 * sizeof(VEC_T)) ;
        VEC_LOAD_OFFSET(F1, freq, 1 * sizeof(VEC_T)) ;
        VEC_LOAD_OFFSET(F2, freq, 2 * sizeof(VEC_T)) ;
        VEC_LOAD_OFFSET(F3, freq, 3 * sizeof(VEC_T)) ;
        VEC_LOAD_OFFSET(F4, freq, 4 * sizeof(VEC_T)) ;
        VEC_LOAD_OFFSET(F5, freq, 5 * sizeof(VEC_T)) ;
        VEC_LOAD_OFFSET(F6, freq, 6 * sizeof(VEC_T)) ;
        VEC_LOAD_OFFSET(F7, freq, 7 * sizeof(VEC_T));

    } while (maxFreq >= valRare);

    VEC_T P0, P1;
    uint64_t maxProbe;

ADVANCE_FREQ:
    do {
        const uint64_t kProbe = (2 + 1) * 8 * 4;
        const uint32_t *probeFreq = freq + kProbe;
        maxProbe = freq[(2 + 2) * 8 * 4 - 1];

        VEC_LOAD_OFFSET(P0, freq, (0 + 2) * 8 * sizeof(VEC_T)) ;
        VEC_LOAD_OFFSET(P1, freq, (1 + 2) * 8 * sizeof(VEC_T));

        if (COMPILER_RARELY(probeFreq >= stopFreq)) {
            goto FINISH_SCALAR;
        }

        freq = probeFreq;

    } while (maxProbe < valRare);

    VEC_CMP_GREATER(P0, Rare) ;
    VEC_CMP_GREATER(P1, Rare);

    int32_t back;
    back = 0; // NOTE: back must be signed
    {
        int32_t _probe;
        VEC_COPY_LOW(_probe, P0);
        back += _probe;
    } ;
    {
        int32_t _probe;
        VEC_COPY_LOW(_probe, P1);
        back += _probe;
    };

    back *= 8 * 4;
    freq += back;
    maxFreq = freq[8 * 4 - 1];

    VEC_LOAD_OFFSET(F0, freq, 0 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F1, freq, 1 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F2, freq, 2 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F3, freq, 3 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F4, freq, 4 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F5, freq, 5 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F6, freq, 6 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F7, freq, 7 * sizeof(VEC_T));

    goto ADVANCE_RARE;

    size_t count;
FINISH_SCALAR:
    count = matchOut - matchOrig;

    lenFreq = stopFreq + kFreqSpace - freq;
    lenRare = stopRare + kRareSpace - rare;

    size_t tail = match_scalar(freq, lenFreq, rare, lenRare, matchOut);
    return count + tail;
}

// unsafe, assumes signed ints
size_t match_v4_f2_p3
(const uint32_t *rare, size_t lenRare,
 const uint32_t *freq, size_t lenFreq,
 uint32_t *matchOut) {

    const uint32_t *matchOrig = matchOut;
    if (lenFreq == 0 || lenRare == 0) return 0;

    const uint64_t kFreqSpace = 2 * 4 * (3 + 1) - 1;
    const uint64_t kRareSpace = 0;

    const uint32_t *stopFreq = &freq[lenFreq] - kFreqSpace;
    const uint32_t *stopRare = &rare[lenRare] - kRareSpace;

    VEC_T Rare;

    VEC_T F0, F1;

    if (COMPILER_RARELY( (rare >= stopRare) || (freq >= stopFreq) )) goto FINISH_SCALAR;
    uint64_t valRare;
    valRare = rare[0];
    VEC_SET_ALL_TO_INT(Rare, valRare);

    uint64_t maxFreq;
    maxFreq = freq[2 * 4 - 1];
    VEC_LOAD_OFFSET(F0, freq, 0 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F1, freq, 1 * sizeof(VEC_T));

    if (COMPILER_RARELY(maxFreq < valRare)) goto ADVANCE_FREQ;

ADVANCE_RARE:
    do {
        *matchOut = static_cast<uint32_t>(valRare);
        valRare = rare[1]; // for next iteration
        ASM_LEA_ADD_BYTES(rare, sizeof(*rare)); // rare += 1;

        if (COMPILER_RARELY(rare >= stopRare)) {
            rare -= 1;
            goto FINISH_SCALAR;
        }

        VEC_CMP_EQUAL(F0, Rare) ;
        VEC_CMP_EQUAL(F1, Rare);

        VEC_SET_ALL_TO_INT(Rare, valRare);

        VEC_OR(F0, F1);

        VEC_ADD_PTEST(matchOut, 1, F0);

        VEC_LOAD_OFFSET(F0, freq, 0 * sizeof(VEC_T)) ;
        VEC_LOAD_OFFSET(F1, freq, 1 * sizeof(VEC_T));

    } while (maxFreq >= valRare);

    VEC_T P0, P1, P2;
    uint64_t maxProbe;

ADVANCE_FREQ:
    do {
        const uint64_t kProbe = (3 + 1) * 2 * 4;
        const uint32_t *probeFreq = freq + kProbe;
        maxProbe = freq[(3 + 2) * 2 * 4 - 1];

        VEC_LOAD_OFFSET(P0, freq, (0 + 2) * 2 * sizeof(VEC_T)) ;
        VEC_LOAD_OFFSET(P1, freq, (1 + 2) * 2 * sizeof(VEC_T)) ;
        VEC_LOAD_OFFSET(P2, freq, (2 + 2) * 2 * sizeof(VEC_T));

        if (COMPILER_RARELY(probeFreq >= stopFreq)) {
            goto FINISH_SCALAR;
        }

        freq = probeFreq;

    } while (maxProbe < valRare);

    VEC_CMP_GREATER(P0, Rare) ;
    VEC_CMP_GREATER(P1, Rare) ;
    VEC_CMP_GREATER(P2, Rare);

    int32_t back;
    back = 0; // NOTE: back must be signed
    {
        int32_t _probe;
        VEC_COPY_LOW(_probe, P0);
        back += _probe;
    } ;
    {
        int32_t _probe;
        VEC_COPY_LOW(_probe, P1);
        back += _probe;
    } ;
    {
        int32_t _probe;
        VEC_COPY_LOW(_probe, P2);
        back += _probe;
    };

    back *= 2 * 4;
    freq += back;
    maxFreq = freq[2 * 4 - 1];

    VEC_LOAD_OFFSET(F0, freq, 0 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F1, freq, 1 * sizeof(VEC_T));

    goto ADVANCE_RARE;

    size_t count;
FINISH_SCALAR:
    count = matchOut - matchOrig;

    lenFreq = stopFreq + kFreqSpace - freq;
    lenRare = stopRare + kRareSpace - rare;

    size_t tail = match_scalar(freq, lenFreq, rare, lenRare, matchOut);

    return count + tail;
}

// unsafe, assumes signed ints
size_t match_v4_f4_p3
(const uint32_t *rare, size_t lenRare,
 const uint32_t *freq, size_t lenFreq,
 uint32_t *matchOut) {

    const uint32_t *matchOrig = matchOut;
    if (lenFreq == 0 || lenRare == 0) return 0;

    const uint64_t kFreqSpace = 4 * 4 * (3 + 1) - 1;
    const uint64_t kRareSpace = 0;

    const uint32_t *stopFreq = &freq[lenFreq] - kFreqSpace;
    const uint32_t *stopRare = &rare[lenRare] - kRareSpace;

    VEC_T Rare;

    VEC_T F0, F1, F2, F3;

    if (COMPILER_RARELY( (rare >= stopRare) || (freq >= stopFreq) )) goto FINISH_SCALAR;
    uint64_t valRare;
    valRare = rare[0];
    VEC_SET_ALL_TO_INT(Rare, valRare);

    uint64_t maxFreq;
    maxFreq = freq[4 * 4 - 1];
    VEC_LOAD_OFFSET(F0, freq, 0 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F1, freq, 1 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F2, freq, 2 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F3, freq, 3 * sizeof(VEC_T));

    if (COMPILER_RARELY(maxFreq < valRare)) goto ADVANCE_FREQ;

ADVANCE_RARE:
    do {
        *matchOut = static_cast<uint32_t>(valRare);
        valRare = rare[1]; // for next iteration
        ASM_LEA_ADD_BYTES(rare, sizeof(*rare)); // rare += 1;

        if (COMPILER_RARELY(rare >= stopRare)) {
            rare -= 1;
            goto FINISH_SCALAR;
        }

        VEC_CMP_EQUAL(F0, Rare) ;
        VEC_CMP_EQUAL(F1, Rare) ;
        VEC_CMP_EQUAL(F2, Rare) ;
        VEC_CMP_EQUAL(F3, Rare);

        VEC_SET_ALL_TO_INT(Rare, valRare);

        VEC_OR(F0, F1);
        VEC_OR(F2, F3);
        VEC_OR(F0, F2);;

        VEC_ADD_PTEST(matchOut, 1, F0);

        VEC_LOAD_OFFSET(F0, freq, 0 * sizeof(VEC_T)) ;
        VEC_LOAD_OFFSET(F1, freq, 1 * sizeof(VEC_T)) ;
        VEC_LOAD_OFFSET(F2, freq, 2 * sizeof(VEC_T)) ;
        VEC_LOAD_OFFSET(F3, freq, 3 * sizeof(VEC_T));

    } while (maxFreq >= valRare);

    VEC_T P0, P1, P2;
    uint64_t maxProbe;

ADVANCE_FREQ:
    do {
        const uint64_t kProbe = (3 + 1) * 4 * 4;
        const uint32_t *probeFreq = freq + kProbe;
        maxProbe = freq[(3 + 2) * 4 * 4 - 1];

        VEC_LOAD_OFFSET(P0, freq, (0 + 2) * 4 * sizeof(VEC_T)) ;
        VEC_LOAD_OFFSET(P1, freq, (1 + 2) * 4 * sizeof(VEC_T)) ;
        VEC_LOAD_OFFSET(P2, freq, (2 + 2) * 4 * sizeof(VEC_T));

        if (COMPILER_RARELY(probeFreq >= stopFreq)) {
            goto FINISH_SCALAR;
        }

        freq = probeFreq;

    } while (maxProbe < valRare);

    VEC_CMP_GREATER(P0, Rare) ;
    VEC_CMP_GREATER(P1, Rare) ;
    VEC_CMP_GREATER(P2, Rare);


    int32_t back;
    back = 0; // NOTE: back must be signed
    {
        int32_t _probe;
        VEC_COPY_LOW(_probe, P0);
        back += _probe;
    } ;
    {
        int32_t _probe;
        VEC_COPY_LOW(_probe, P1);
        back += _probe;
    } ;
    {
        int32_t _probe;
        VEC_COPY_LOW(_probe, P2);
        back += _probe;
    };

    back *= 4 * 4;
    freq += back;
    maxFreq = freq[4 * 4 - 1];

    VEC_LOAD_OFFSET(F0, freq, 0 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F1, freq, 1 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F2, freq, 2 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F3, freq, 3 * sizeof(VEC_T));

    goto ADVANCE_RARE;

    size_t count;
FINISH_SCALAR:
    count = matchOut - matchOrig;

    lenFreq = stopFreq + kFreqSpace - freq;
    lenRare = stopRare + kRareSpace - rare;

    size_t tail = match_scalar(freq, lenFreq, rare, lenRare, matchOut);
    return count + tail;
}

// unsafe, assumes signed ints
size_t match_v4_f8_p3
(const uint32_t *rare, size_t lenRare,
 const uint32_t *freq, size_t lenFreq,
 uint32_t *matchOut) {

    const uint32_t *matchOrig = matchOut;
    if (lenFreq == 0 || lenRare == 0) return 0;

    const uint64_t kFreqSpace = 8 * 4 * (3 + 1) - 1;
    const uint64_t kRareSpace = 0;

    const uint32_t *stopFreq = &freq[lenFreq] - kFreqSpace;
    const uint32_t *stopRare = &rare[lenRare] - kRareSpace;

    VEC_T Rare;

    VEC_T F0, F1, F2, F3, F4, F5, F6, F7;

    if (COMPILER_RARELY( (rare >= stopRare) || (freq >= stopFreq) )) goto FINISH_SCALAR;
    uint64_t valRare;
    valRare = rare[0];
    VEC_SET_ALL_TO_INT(Rare, valRare);

    uint64_t maxFreq;
    maxFreq = freq[8 * 4 - 1];
    VEC_LOAD_OFFSET(F0, freq, 0 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F1, freq, 1 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F2, freq, 2 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F3, freq, 3 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F4, freq, 4 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F5, freq, 5 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F6, freq, 6 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F7, freq, 7 * sizeof(VEC_T));

    if (COMPILER_RARELY(maxFreq < valRare)) goto ADVANCE_FREQ;


ADVANCE_RARE:
    do {
        *matchOut = static_cast<uint32_t>(valRare);
        valRare = rare[1]; // for next iteration
        ASM_LEA_ADD_BYTES(rare, sizeof(*rare)); // rare += 1;

        if (COMPILER_RARELY(rare >= stopRare)) {
            rare -= 1;
            goto FINISH_SCALAR;
        }

        VEC_CMP_EQUAL(F0, Rare) ;
        VEC_CMP_EQUAL(F1, Rare) ;
        VEC_CMP_EQUAL(F2, Rare) ;
        VEC_CMP_EQUAL(F3, Rare) ;
        VEC_CMP_EQUAL(F4, Rare) ;
        VEC_CMP_EQUAL(F5, Rare) ;
        VEC_CMP_EQUAL(F6, Rare) ;
        VEC_CMP_EQUAL(F7, Rare);


        VEC_SET_ALL_TO_INT(Rare, valRare);

        VEC_OR(F0, F1);
        VEC_OR(F2, F3);
        VEC_OR(F4, F5);
        VEC_OR(F6, F7);
        VEC_OR(F0, F2);
        VEC_OR(F4, F6);
        VEC_OR(F0, F4);

        VEC_ADD_PTEST(matchOut, 1, F0);

        VEC_LOAD_OFFSET(F0, freq, 0 * sizeof(VEC_T)) ;
        VEC_LOAD_OFFSET(F1, freq, 1 * sizeof(VEC_T)) ;
        VEC_LOAD_OFFSET(F2, freq, 2 * sizeof(VEC_T)) ;
        VEC_LOAD_OFFSET(F3, freq, 3 * sizeof(VEC_T)) ;
        VEC_LOAD_OFFSET(F4, freq, 4 * sizeof(VEC_T)) ;
        VEC_LOAD_OFFSET(F5, freq, 5 * sizeof(VEC_T)) ;
        VEC_LOAD_OFFSET(F6, freq, 6 * sizeof(VEC_T)) ;
        VEC_LOAD_OFFSET(F7, freq, 7 * sizeof(VEC_T));

    } while (maxFreq >= valRare);


    VEC_T P0, P1, P2;
    uint64_t maxProbe;

ADVANCE_FREQ:
    do {
        const uint64_t kProbe = (3 + 1) * 8 * 4;
        const uint32_t *probeFreq = freq + kProbe;
        maxProbe = freq[(3 + 2) * 8 * 4 - 1];

        VEC_LOAD_OFFSET(P0, freq, (0 + 2) * 8 * sizeof(VEC_T)) ;
        VEC_LOAD_OFFSET(P1, freq, (1 + 2) * 8 * sizeof(VEC_T)) ;
        VEC_LOAD_OFFSET(P2, freq, (2 + 2) * 8 * sizeof(VEC_T));

        if (COMPILER_RARELY(probeFreq >= stopFreq)) {
            goto FINISH_SCALAR;
        }

        freq = probeFreq;

    } while (maxProbe < valRare);

    VEC_CMP_GREATER(P0, Rare) ;
    VEC_CMP_GREATER(P1, Rare) ;
    VEC_CMP_GREATER(P2, Rare);

    int32_t back;
    back = 0; // NOTE: back must be signed
    {
        int32_t _probe;
        VEC_COPY_LOW(_probe, P0);
        back += _probe;
    } ;
    {
        int32_t _probe;
        VEC_COPY_LOW(_probe, P1);
        back += _probe;
    } ;
    {
        int32_t _probe;
        VEC_COPY_LOW(_probe, P2);
        back += _probe;
    };

    back *= 8 * 4;
    freq += back;
    maxFreq = freq[8 * 4 - 1];

    VEC_LOAD_OFFSET(F0, freq, 0 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F1, freq, 1 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F2, freq, 2 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F3, freq, 3 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F4, freq, 4 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F5, freq, 5 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F6, freq, 6 * sizeof(VEC_T)) ;
    VEC_LOAD_OFFSET(F7, freq, 7 * sizeof(VEC_T));

    goto ADVANCE_RARE;

    size_t count;
FINISH_SCALAR:
    count = matchOut - matchOrig;

    lenFreq = stopFreq + kFreqSpace - freq;
    lenRare = stopRare + kRareSpace - rare;

    size_t tail = match_scalar(freq, lenFreq, rare, lenRare, matchOut);
    return count + tail;
}
