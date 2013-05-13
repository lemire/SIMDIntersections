
#ifndef HYBRIDINTERSECTION_H_
#define HYBRIDINTERSECTION_H_

#include "intersection.h"
#include "gallopingintersection.h"
#include "binarysearchintersection.h"
#include "mediumintersection.h"
#include "widevectorintersection.h"

size_t danielshybridintersectioncardinality(const uint32_t * set1,
		const size_t length1, const uint32_t * set2, const size_t length2) {
	if ((10 * length1 <= length2) or (10 * length2 <= length1)) {
		if ((200 * length1 < length2) or (200 * length2 < length1)) {
			if (length1 < length2)
				return onesidedgallopingintersectioncardinality(set1, length1,
						set2, length2);
			else
				return onesidedgallopingintersectioncardinality(set2, length2,
						set1, length1);
		} else {
			if (length1 < length2)
				return nate3_count_medium(set1, length1, set2, length2);
			else
				return nate3_count_medium(set2, length2, set1, length1);
		}
	}
	return widevector2_cardinality_intersect(set1, length1, set2, length2);
}

#endif /* HYBRIDINTERSECTION_H_ */
