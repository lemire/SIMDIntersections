#ifndef HYBRIDINTERSECTION_H_
#define HYBRIDINTERSECTION_H_

#include "intersection.h"
#include "gallopingintersection.h"
#include "binarysearchintersection.h"
#include "mediumintersection.h"
#include "widevectorintersection.h"
#include "hscalableintersection.h"
#include "match.h"
size_t danielshybridintersectioncardinality(const uint32_t * set1,
        const size_t length1, const uint32_t * set2, const size_t length2) {
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
    return highlyscalablewordpresscom::dan_cardinality_intersect_SIMD(set1, length1, set2, length2);
}

size_t olddanielshybridintersection(const uint32_t * set1,
        const size_t length1, const uint32_t * set2, const size_t length2, uint32_t *out) {
    if ((10 * length1 <= length2) or (10 * length2 <= length1)) {
        if ((200 * length1 < length2) or (200 * length2 < length1)) {
            if (length1 < length2)
                return danfar_medium(set1, length1,
                        set2, length2,out);
            else
                return danfar_medium(set2, length2,
                        set1, length1,out);
        } else {
            if (length1 < length2)
                return natedanalt_medium(set1, length1, set2, length2,out);
            else
                return natedanalt_medium(set2, length2, set1, length1,out);
        }
    }
    return highlyscalablewordpresscom::dan_intersect_SIMD(set1, length1, set2, length2,out);
}

size_t danielshybridintersection(const uint32_t * set1,
        const size_t length1, const uint32_t * set2, const size_t length2, uint32_t *out) {
    if ((length1==0) or (length2 == 0)) return 0;
    const double scale1 = set1[length1 - 1];
    const double scale2 = set2[length2 - 1];
    if ((10 * static_cast<double>(length1) * scale2 <= static_cast<double>(length2) * scale1) or (10 * static_cast<double>(length2) * scale1 <= static_cast<double>(length1) * scale2)) {
	if ((200 * static_cast<double>(length1) * scale2 < static_cast<double>(length2) * scale1) or (200 * static_cast<double>(length2) * scale1 < static_cast<double>(length1) * scale2)) {
            if (static_cast<double>(length1) * scale2 < static_cast<double>(length2) * scale1)
                return danfar_medium(set1, length1,
                        set2, length2,out);
            else
                return danfar_medium(set2, length2,
                        set1, length1,out);
        }
        if (static_cast<double>(length1) * scale2 < static_cast<double>(length2) * scale1)
                return natedanalt_medium(set1, length1, set2, length2,out);
        else
                return natedanalt_medium(set2, length2, set1, length1,out);
    } else if ((2 * static_cast<double>(length1) * scale2 <= static_cast<double>(length2) * scale1) or (2 * static_cast<double>(length2) * scale1 <= static_cast<double>(length1) * scale2)) {
            if (static_cast<double>(length1) * scale2 < static_cast<double>(length2) * scale1)
                return match_v4_f2_p0(set1, length1,
                        set2, length2,out);
            else
                return match_v4_f2_p0(set2, length2,
                        set1, length1,out);
    }
    return highlyscalablewordpresscom::dan_intersect_SIMD(set1, length1, set2, length2,out);
}


#endif /* HYBRIDINTERSECTION_H_ */
