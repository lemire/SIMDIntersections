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
        const uint32_t * set2, const size_t length2);

/**
 * Compute the *cardinality* of the intersection between two *sorted*
 * arrays.
 *
 * Algorithm design by D. Lemire. It uses several while loops on
 * purpose.
 *
 */
size_t danscalarintersection(const uint32_t * set1, const size_t length1,
        const uint32_t * set2, const size_t length2, uint32_t * out) ;
/**
 * This is the classical approach
 */
size_t classicalintersectioncardinality(const uint32_t * set1,
        const size_t length1, const uint32_t * set2, const size_t length2);
/**
 * This is the classical approach
 */
size_t classicalintersection(const uint32_t * set1,
        const size_t length1, const uint32_t * set2, const size_t length2, uint32_t * out) ;


#endif /* INTERSECTION_H_ */
