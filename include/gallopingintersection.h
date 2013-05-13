/*
 * gallopingintersection.h
 *
 *  Created on: May 13, 2013
 *      Author: ?
 */

#ifndef GALLOPINGINTERSECTION_H_
#define GALLOPINGINTERSECTION_H_

/**
 * This is often called galloping or exponential search.
 *
 * Used by frogintersectioncardinality below
 *
 * Based on binary search...
 * Find the smallest integer larger than pos such
 * that array[pos]>= min.
 * If none can be found, return array.length.
 * From code by O. Kaser.
 */
static size_t __frogadvanceUntil(const uint32_t * array, const size_t pos,
        const size_t length, const size_t min) {
    size_t lower = pos + 1;

    // special handling for a possibly common sequential case
    if ((lower >= length) or (array[lower] >= min)) {
        return lower;
    }

    size_t spansize = 1; // could set larger
    // bootstrap an upper limit

    while ((lower + spansize < length) and (array[lower + spansize] < min))
        spansize *= 2;
    size_t upper = (lower + spansize < length) ? lower + spansize : length - 1;

    // maybe we are lucky (could be common case when the seek ahead expected to be small and sequential will otherwise make us look bad)
    if (array[upper] == min) {
        return upper;
    }

    if (array[upper] < min) {// means array has no item >= min
        return length;
    }

    // we know that the next-smallest span was too small
    lower += (spansize / 2);

    // else begin binary search
    size_t mid = 0;
    while (lower + 1 != upper) {
        mid = (lower + upper) / 2;
        if (array[mid] == min) {
            return mid;
        } else if (array[mid] < min)
            lower = mid;
        else
            upper = mid;
    }
    return upper;

}

/**
 * based on galloping
 */
size_t frogintersectioncardinality(const uint32_t * set1, const size_t length1,
        const uint32_t * set2, const size_t length2) {
    if ((0 == length1) or (0 == length2))
        return 0;
    size_t answer = 0;
    size_t k1 = 0, k2 = 0;
    while (true) {
        if (set1[k1] < set2[k2]) {
            k1 = __frogadvanceUntil(set1, k1, length1, set2[k2]);
            if (k1 == length1)
                return answer;
        }
        if (set2[k2] < set1[k1]) {
            k2 = __frogadvanceUntil(set2, k2, length2, set1[k1]);
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

/**
 * assumes that smalllength < largelength
 */
size_t onesidedgallopingintersectioncardinality(const uint32_t * smallset,
        const size_t smalllength, const uint32_t * largeset,
        const size_t largelength) {
    if (0 == smalllength)
        return 0;
    size_t answer = 0;
    size_t k1 = 0, k2 = 0;
    while (true) {
        if (largeset[k1] < smallset[k2]) {
            k1 = __frogadvanceUntil(largeset, k1, largelength, smallset[k2]);
            if (k1 == largelength)
                return answer;
        }
        midpoint: if (smallset[k2] < largeset[k1]) {
            ++k2;
            if (k2 == smalllength)
                return answer;
        } else {
            ++answer;
            ++k2;
            if (k2 == smalllength)
                return answer;
            k1 = __frogadvanceUntil(largeset, k1, largelength, smallset[k2]);
            if (k1 == largelength)
                return answer;
            goto midpoint;
        }
    }
    return answer;

}

#endif /* GALLOPINGINTERSECTION_H_ */
