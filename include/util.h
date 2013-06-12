/**
 * This code is released under the
 * Apache License Version 2.0 http://www.apache.org/licenses/.
 *
 */

#ifndef UTIL_H_
#define UTIL_H_

#include "common.h"

using namespace std;

/**
 * unsatured packing.
 */
__attribute__((always_inline))
inline __m128i __pack_epu32(__m128i hi, __m128i lo) {
    const static __m128i mask =
        _mm_set_epi8(0, 0, -1, -1, 0, 0, -1, -1, 0, 0, -1, -1, 0, 0, -1, -1);
    hi = _mm_and_si128(hi, mask);
    lo = _mm_and_si128(lo, mask);
    return _mm_packus_epi32(hi, lo);
}

/**
 * Not recommended.
 */
__attribute__((always_inline))
inline __m128i __altpack_epu32(__m128i hi, __m128i lo) {
    //0b10101010 = 170
    __m128i bva =
        _mm_blend_epi16(hi,
                        _mm_slli_si128(lo, 2), 170);
    const static __m128i shufflekey =
        _mm_set_epi8(15, 14, 11, 10, 7, 6, 3, 2,  13, 12, 9, 8, 5, 4, 1, 0);
    return _mm_shuffle_epi8(bva, shufflekey);
}


vector<string> split(const string &str, const string &del) {
    vector <string> tokens;
    size_t lastPos = str.find_first_not_of(del, 0);
    size_t pos = str.find_first_of(del, lastPos);
    while (string::npos != pos || string::npos != lastPos) {
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        lastPos = str.find_first_not_of(del, pos);
        pos = str.find_first_of(del, lastPos);
    }
    return tokens;
}


//Fisher-Yates shuffle
template<class iter>
iter shuffleFY(iter begin, iter end, size_t N) {
    size_t M = distance(begin, end);
    while (N--) {
        iter r = begin;
        advance(r, rand() % M);
        swap(*begin, *r);
        begin++;
        M--;
    }
    return begin;
}

#endif /* UTIL_H_ */
