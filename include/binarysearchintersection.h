/*
 * binarysearch.h
 *
 *  Created on: May 13, 2013
 *      Author: ?
 */

#ifndef BINARYSEARCH_H_
#define BINARYSEARCH_H_

#include "common.h"


/**
 * This is pure binary search
 * Used by BSintersectioncardinality below
 * @param array
 * @param pos
 * @param min
 * @return
 */
static size_t __BSadvanceUntil(const uint32_t * array, const size_t pos,
        const size_t length, const size_t min) {
    size_t lower = pos + 1;
    if (lower == length || array[lower] >= min) {
        return lower;
    }
    // can safely assume that length>0
    size_t upper = length - 1;
    if (array[upper] < min) {
        return length;
    }
    size_t mid;
    while (lower < upper) {
        mid = (lower + upper) / 2;
        if (array[mid] == min) {
            return mid;
        }

        if (array[mid] < min) {
            lower = mid + 1;
        } else {
            upper = mid;
        }
    }
    return upper;
}

/**
 * Based on binary search.
 */
size_t BSintersectioncardinality(const uint32_t * set1, const size_t length1,
        const uint32_t * set2, const size_t length2) {
    if ((0 == length1) or (0 == length2))
        return 0;
    size_t answer = 0;
    size_t k1 = 0, k2 = 0;
    while (true) {
        if (set1[k1] < set2[k2]) {
            k1 = __BSadvanceUntil(set1, k1, length1, set2[k2]);
            if (k1 == length1)
                return answer;
        }
        if (set2[k2] < set1[k1]) {
            k2 = __BSadvanceUntil(set2, k2, length2, set1[k1]);
            if (k2 == length2)
                return answer;
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

static size_t __FixedBSadvanceUntil(const uint32_t * array,
        const size_t length, const size_t min) {
    size_t lower = 0;
    size_t upper = length - 1;
    if (array[upper] < min) {
        return length;
    }
    size_t mid;
    while (lower < upper) {
        mid = (lower + upper) / 2;
        if (array[mid] == min) {
            return mid;
        }

        if (array[mid] < min) {
            lower = mid + 1;
        } else {
            upper = mid;
        }
    }
    return upper;
}

/**
 * Based on binary search.
 */
size_t FixedBSintersectioncardinality(const uint32_t * set1,
        const size_t length1, const uint32_t * set2, const size_t length2) {
    if ((0 == length1) or (0 == length2))
        return 0;
    size_t answer = 0;
    size_t k1 = 0, k2 = 0;
    while (true) {
        if (set1[k1] < set2[k2]) {
            k1 = __FixedBSadvanceUntil(set1, length1, set2[k2]);
            if (k1 == length1)
                return answer;
        }

        if (set2[k2] < set1[k1]) {
            k2 = __FixedBSadvanceUntil(set2, length2, set1[k1]);
            if (k2 == length2)
                return answer;
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

#endif /* BINARYSEARCH_H_ */
