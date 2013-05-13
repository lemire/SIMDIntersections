/**
 * This code is released under the
 * Apache License Version 2.0 http://www.apache.org/licenses/.
 *
 * (c) Daniel Lemire, http://lemire.me/en/
 */

#ifndef INTERSECTION_H_
#define INTERSECTION_H_

#include "common.h"

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
size_t intersectioncardinality(const uint32_t * set1, const size_t length1,
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

#endif /* INTERSECTION_H_ */
