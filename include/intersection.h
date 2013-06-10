/**
 * This code is released under the
 * Apache License Version 2.0 http://www.apache.org/licenses/.
 *
 * (c) Daniel Lemire, http://lemire.me/en/
 */

#ifndef INTERSECTION_H_
#define INTERSECTION_H_

#include "common.h"
typedef size_t (*intersectionfunction)(const uint32_t * set1,
        const size_t length1, const uint32_t * set2, const size_t length2, uint32_t * out);


typedef size_t (*cardinalityintersectionfunction)(const uint32_t * set1,
        const size_t length1, const uint32_t * set2, const size_t length2);

typedef size_t (*cardinalityintersectionfunctionpart)(const uint16_t *A,
        const uint16_t *B, const size_t s_a, const size_t s_b);

/**
 * Compute the *cardinality* of the intersection between two *sorted*
 * arrays.
 *
 * Algorithm design by D. Lemire. It uses several while loops on
 * purpose.
 *
 */
size_t danscalarintersectioncardinality(const uint32_t * set1, const size_t length1,
        const uint32_t * set2, const size_t length2) {
    if ((0 == length1) or (0 == length2))
        return 0;
    size_t answer = 0;
    size_t k1 = 0, k2 = 0;
    while (true) {
        if (set1[k1] < set2[k2]) {
            do {
                ++k1;
                if (k1 == length1)
                    return answer;
            } while (set1[k1] < set2[k2]);
        }
        if (set2[k2] < set1[k1]) {

            do {
                ++k2;
                if (k2 == length2)
                    return answer;
            } while (set2[k2] < set1[k1]);
        } else {
            // assert(set2[k2] == set1[k1]);
            ++answer;
            ++k1;
            if (k1 == length1)
                break;
            ++k2;
            if (k2 == length2)
                break;
        }
    }
    return answer;
}

/**
 * Compute the *cardinality* of the intersection between two *sorted*
 * arrays.
 *
 * Algorithm design by D. Lemire. It uses several while loops on
 * purpose.
 *
 */
size_t danscalarintersection(const uint32_t * set1, const size_t length1,
        const uint32_t * set2, const size_t length2, uint32_t * out) {
    if ((0 == length1) or (0 == length2))
        return 0;
    size_t answer = 0;
    size_t k1 = 0, k2 = 0;
    while (true) {
        if (set1[k1] < set2[k2]) {
            do {
                ++k1;
                if (k1 == length1)
                    return answer;
            } while (set1[k1] < set2[k2]);
        }
        if (set2[k2] < set1[k1]) {

            do {
                ++k2;
                if (k2 == length2)
                    return answer;
            } while (set2[k2] < set1[k1]);
        } else {

            // assert(set2[k2] == set1[k1]);
            out[answer++] = set1[k1];
            ++k1;
            if (k1 == length1)
                break;
            ++k2;
            if (k2 == length2)
                break;
        }
    }
    return answer;
}

/**
 * This is the classical approach
 */
size_t classicalintersectioncardinality(const uint32_t * set1,
        const size_t length1, const uint32_t * set2, const size_t length2) {
    if ((0 == length1) or (0 == length2))
        return 0;
    size_t answer = 0;
    size_t k1 = 0, k2 = 0;
    while (true) {
        if (set1[k1] < set2[k2]) {
            ++k1;
            if (k1 == length1)
                return answer;
        } else if (set2[k2] < set1[k1]) {
            ++k2;
            if (k2 == length2)
                return answer;
        } else {
            // (set2[k2] == set1[k1])
            ++answer;
            ++k1;
            if (k1 == length1)
                break;
            ++k2;
            if (k2 == length2)
                break;

        }
    }
    return answer;

}
/**
 * This is the classical approach
 */
size_t classicalintersection(const uint32_t * set1,
        const size_t length1, const uint32_t * set2, const size_t length2, uint32_t * out) {
    if ((0 == length1) or (0 == length2))
        return 0;
    size_t answer = 0;
    size_t k1 = 0, k2 = 0;
    while (true) {
        if (set1[k1] < set2[k2]) {
            ++k1;
            if (k1 == length1)
                return answer;
        } else if (set2[k2] < set1[k1]) {
            ++k2;
            if (k2 == length2)
                return answer;
        } else {
            // (set2[k2] == set1[k1])
            out[answer++] = set1[k1];
            ++k1;
            if (k1 == length1)
                break;
            ++k2;
            if (k2 == length2)
                break;

        }
    }
    return answer;

}


/**
 * Failed attempt at reproducing the good results of the branchless scheme
 * from Fast Sorted-Set Intersection using SIMD Instructions
 */
size_t branchlessintersection(const uint32_t * set1, const size_t length1,
        const uint32_t * set2, const size_t length2, uint32_t * out) {
    if ((0 == length1) or (0 == length2))
        return 0;
    const uint32_t * const initout(out);
    const uint32_t * const finalset1(set1 + length1);
    const uint32_t * const finalset2(set2 + length2);
    // preliminary loop
    if (*set1 == *set2) {
        do {
            *out++ = *set1;
            ++set1;
            ++set2;
            if (set1 == finalset1 or set2 == finalset2)
                return (out - initout);
        } while (*set1 == *set2);
    }

    const unsigned int N = 4;

    // main loop
    while ((set1 + N < finalset1) && (set2 + N < finalset2)) {
        for (unsigned int k = 0; k < N; ++k) {
            // this is branchless...
            set1 = (*set1 <= *set2) ? set1 + 1 : set1;
            set2 = (*set2 < *set1) ? set2 + 1 : set2;
            *out = *set1;
            out = (*set1 == *set2) ? out + 1 : out;
        }

    }
    // final processing
    if (*set1 == *set2) {
        ++set1;
        ++set2;
    }

    while (set1 < finalset1 && set2 < finalset2) {
        if (*set1 < *set2) {
            ++set1;
        } else if (*set1 > *set2) {
            ++set2;
        } else {
            *out++ = *set1;
            ++set1;
            ++set2;
        }
    }

    return (out - initout);
}
#endif /* INTERSECTION_H_ */
