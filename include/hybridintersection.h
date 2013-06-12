#ifndef HYBRIDINTERSECTION_H_
#define HYBRIDINTERSECTION_H_

#include "intersection.h"
#include "gallopingintersection.h"
#include "binarysearchintersection.h"
#include "mediumintersection.h"
#include "widevectorintersection.h"
#include "hscalableintersection.h"

size_t danielshybridintersectioncardinality(const uint32_t *set1,
        const size_t length1, const uint32_t *set2, const size_t length2) {
    if ((10 * length1 <= length2) or (10 * length2 <= length1)) {
        if ((200 * length1 < length2) or (200 * length2 < length1)) {
            if (length1 < length2)
                return danfar_count_medium(set1, length1,
                                           set2, length2);
            else
                return danfar_count_medium(set2, length2,
                                           set1, length1);
        } else {
            if (length1 < length2)
                return natedanalt_count_medium(set1, length1, set2, length2);
            else
                return natedanalt_count_medium(set2, length2, set1, length1);
        }
    }
    return highlyscalablewordpresscom::dan_cardinality_intersect_SIMD(set1, length1,
            set2, length2);
}

size_t danielshybridintersection(const uint32_t *set1,
                                 const size_t length1, const uint32_t *set2, const size_t length2,
                                 uint32_t *out) {
    if ((10 * length1 <= length2) or (10 * length2 <= length1)) {
        if ((200 * length1 < length2) or (200 * length2 < length1)) {
            if (length1 < length2)
                return danfar_medium(set1, length1,
                                     set2, length2, out);
            else
                return danfar_medium(set2, length2,
                                     set1, length1, out);
        } else {
            if (length1 < length2)
                return natedanalt_medium(set1, length1, set2, length2, out);
            else
                return natedanalt_medium(set2, length2, set1, length1, out);
        }
    }
    return highlyscalablewordpresscom::dan_intersect_SIMD(set1, length1, set2,
            length2, out);
}


#endif /* HYBRIDINTERSECTION_H_ */
