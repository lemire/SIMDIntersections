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
 * Failed attempt at vectorization by D. Lemire. Keeping it only as a reference.
 */
size_t vectorintersectioncardinality(const uint32_t * set1, const size_t l1,
		const uint32_t * set2, const size_t l2) {
	if ((0 == l1) or (0 == l2))
		return 0;
	const uint32_t cyclic_shift = _MM_SHUFFLE(0, 3, 2, 1);
	size_t answer = 0;
	size_t k1 = 0, k2 = 0;
	size_t length1 = (l1 / 4) * 4;
	size_t length2 = (l2 / 4) * 4;
	__m128i v_a, v_b;
	if ((length1 > 0) and (length2 > 0)) {
		v_a = _mm_load_si128((__m128i *) set1);
		v_b = _mm_load_si128((__m128i *) set2);
	}
	while (true) {
		if (set1[k1 + 3] < set2[k2]) {
			do {
				k1 += 4;
				if (k1 == length1)
					goto end;
			} while (set1[k1 + 3] < set2[k2]);
			v_a = _mm_load_si128((__m128i *) (set1 + k1));
		}
		if (set2[k2 + 3] < set1[k1]) {

			do {
				k2 += 4;
				if (k2 == length2)
					goto end;
			} while (set2[k2 + 3] < set1[k1]);
			v_b = _mm_load_si128((__m128i *) (set2 + k2));

		} else {
			/**
			 * we have that set1[k1+3] >= set2[k2]
			 * and set2[k1+3] >= set1[k2]
			 *
			 */
			__m128i cmp_mask = _mm_cmpeq_epi32(v_a, v_b);
			__m128i cycledv_b = _mm_shuffle_epi32(v_b, cyclic_shift); // shuffling
			const __m128i cmp_mask2 = _mm_cmpeq_epi32(v_a, cycledv_b); // again...
			cmp_mask = _mm_or_si128(cmp_mask2, cmp_mask);
			cycledv_b = _mm_shuffle_epi32(cycledv_b, cyclic_shift); // shuffling
			const __m128i cmp_mask3 = _mm_cmpeq_epi32(v_a, cycledv_b); // and again...
			cmp_mask = _mm_or_si128(cmp_mask3, cmp_mask);
			cycledv_b = _mm_shuffle_epi32(cycledv_b, cyclic_shift); // shuffling
			const __m128i cmp_mask4 = _mm_cmpeq_epi32(v_a, cycledv_b); // and again...
			cmp_mask = _mm_or_si128(cmp_mask4, cmp_mask);
			const int mask = _mm_movemask_ps((__m128 ) cmp_mask);
			//]

			//[ copy out common elements
			//__m128i p = _mm_shuffle_epi8(v_a, shuffle_mask[mask]);
			//_mm_storeu_si128((__m128i*)&C[count], p);
			answer += _mm_popcnt_u32(mask); // a number of elements is a weight of the mask

			const uint32_t a_max = set1[k1 + 3];
			const uint32_t b_max = set2[k2 + 3];
			if (a_max <= b_max) {
				k1 += 4;
				if (k1 == length1)
					goto end;
				v_a = _mm_load_si128((__m128i *) (set1 + k1));
			}
			if (a_max >= b_max) {
				k2 += 4;
				if (k2 == length2)
					goto end;
				v_b = _mm_load_si128((__m128i *) (set2 + k2));

			}

		}
	}
	end: if ((k1 < l1) and (k2 < l2))
		while (true) {
			if (set1[k1] < set2[k2]) {
				do {
					++k1;
					if (k1 == l1)
						return answer;
				} while (set1[k1] < set2[k2]);
			}
			if (set2[k2] < set1[k1]) {

				do {
					++k2;
					if (k2 == l2)
						return answer;
				} while (set2[k2] < set1[k1]);
			} else {
				// assert(set2[k2] == set1[k1]);
				++answer;
				++k1;
				if (k1 == l1)
					break;
				++k2;
				if (k2 == l2)
					break;
			}
		}

	return answer;

}

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
size_t onesidedgallopingintersectioncardinality(const uint32_t * smallset, const size_t smalllength,
		const uint32_t * largeset, const size_t largelength) {
	if (0 == smalllength)
		return 0;
	size_t answer = 0;
	size_t k1 = 0, k2 = 0;
	while (true) {
		if (largeset[k1] <  smallset[k2]) {
			k1 = __frogadvanceUntil(largeset, k1, largelength, smallset[k2]);
			if (k1 == largelength)
				return answer;
		}
		midpoint:
		if (smallset[k2] < largeset[k1]) {
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
 * Vectorized version by D. Lemire.
 */
size_t widevector_cardinality_intersect(const uint32_t *A, const size_t s_a,
		const uint32_t *B, const size_t s_b) {
	const size_t BlockSize = 8;
	const size_t HalfBlockSize = 4;

	size_t count = 0;
	size_t i_a = 0, i_b = 0;
	const static uint32_t cyclic_shift = _MM_SHUFFLE(0, 3, 2, 1);
	// trim lengths to be a multiple of 8
	size_t st_a = (s_a / BlockSize) * BlockSize;
	size_t st_b = (s_b / BlockSize) * BlockSize;

	if (i_a < st_a && i_b < st_b) {
		while (A[i_a + BlockSize - 1] < B[i_b]) {
			i_a += BlockSize;
			if (i_a >= st_a)
				goto end;
		}
		while (B[i_b + BlockSize - 1] < A[i_a]) {
			i_b += BlockSize;
			if (i_b >= st_b)
				goto end;
		};
		__m128i v_a, v_b, v_aa, v_bb;
		v_a = _mm_load_si128((__m128i *) &A[i_a]);
		v_aa = _mm_load_si128((__m128i *) &A[i_a + HalfBlockSize]);
		v_b = _mm_load_si128((__m128i *) &B[i_b]);
		v_bb = _mm_load_si128((__m128i *) &B[i_b + HalfBlockSize]);

		while (true) {

			if ((((A[i_a] - 1) ^ A[i_a + BlockSize - 1]) >> 16 == 0)
					and (((B[i_b] - 1) ^ B[i_b + BlockSize - 1]) >> 16 == 0)) {
				if ((A[i_a] ^ B[i_b]) >> 16 == 0) {

					// higher 16 bits are all equal
					//0b10101010 = 170
					const __m128i bva = _mm_blend_epi16(v_a,
							_mm_slli_si128(v_aa, 2), 170);
					const __m128i bvb = _mm_blend_epi16(v_b,
							_mm_slli_si128(v_bb, 2), 170);
					const __m128i res_v = _mm_cmpistrm(
							bva,
							bvb,
							_SIDD_UWORD_OPS | _SIDD_CMP_EQUAL_ANY
									| _SIDD_BIT_MASK);

					const int r = _mm_extract_epi32(res_v, 0);
					//__m128i p = _mm_shuffle_epi8(v_a, shuffle_mask16[r]);
					//_mm_storeu_si128((__m128i *) &C[count], p);
					count += _mm_popcnt_u32(r);
				}
			} else {

				__m128i cycledv_b;
				__m128i cmp_mask1, cmp_mask2, cmp_mask3, cmp_mask4;
				__m128i cmp_mask;
				int mask;

				cmp_mask1 = _mm_cmpeq_epi32(v_a, v_b); // pairwise comparison
				cycledv_b = _mm_shuffle_epi32(v_b, cyclic_shift); // shuffling
				cmp_mask2 = _mm_cmpeq_epi32(v_a, cycledv_b); // again...
				cmp_mask = _mm_or_si128(cmp_mask1, cmp_mask2);
				cycledv_b = _mm_shuffle_epi32(cycledv_b, cyclic_shift);
				cmp_mask3 = _mm_cmpeq_epi32(v_a, cycledv_b); // and again...
				cmp_mask = _mm_or_si128(cmp_mask, cmp_mask3);
				cycledv_b = _mm_shuffle_epi32(cycledv_b, cyclic_shift);
				cmp_mask4 = _mm_cmpeq_epi32(v_a, cycledv_b); // and again.
				cmp_mask = _mm_or_si128(cmp_mask, cmp_mask4);
				// convert the 128-bit mask to the 4-bit mask
				mask = _mm_movemask_ps((__m128 ) cmp_mask);
				count += _mm_popcnt_u32(mask); // a number of elements is a weight of the mask
				//]

				//[ copy out common elements
				//__m128i p = _mm_shuffle_epi8(v_a, shuffle_mask[mask]);
				//_mm_storeu_si128((__m128i*)&C[count], p);
				//]

				cmp_mask1 = _mm_cmpeq_epi32(v_aa, v_bb); // pairwise comparison
				cycledv_b = _mm_shuffle_epi32(v_bb, cyclic_shift); // shuffling
				cmp_mask2 = _mm_cmpeq_epi32(v_aa, cycledv_b); // again...
				cmp_mask = _mm_or_si128(cmp_mask1, cmp_mask2);
				cycledv_b = _mm_shuffle_epi32(cycledv_b, cyclic_shift);
				cmp_mask3 = _mm_cmpeq_epi32(v_aa, cycledv_b); // and again...
				cmp_mask = _mm_or_si128(cmp_mask, cmp_mask3);
				cycledv_b = _mm_shuffle_epi32(cycledv_b, cyclic_shift);
				cmp_mask4 = _mm_cmpeq_epi32(v_aa, cycledv_b); // and again.
				cmp_mask = _mm_or_si128(cmp_mask, cmp_mask4);
				// convert the 128-bit mask to the 4-bit mask
				mask = _mm_movemask_ps((__m128 ) cmp_mask);
				count += _mm_popcnt_u32(mask); // a number of elements is a weight of the mask
				//]


				cmp_mask1 = _mm_cmpeq_epi32(v_aa, v_b); // pairwise comparison
				cycledv_b = _mm_shuffle_epi32(v_b, cyclic_shift); // shuffling
				cmp_mask2 = _mm_cmpeq_epi32(v_aa, cycledv_b); // again...
				cmp_mask = _mm_or_si128(cmp_mask1, cmp_mask2);
				cycledv_b = _mm_shuffle_epi32(cycledv_b, cyclic_shift);
				cmp_mask3 = _mm_cmpeq_epi32(v_aa, cycledv_b); // and again...
				cmp_mask = _mm_or_si128(cmp_mask, cmp_mask3);
				cycledv_b = _mm_shuffle_epi32(cycledv_b, cyclic_shift);
				cmp_mask4 = _mm_cmpeq_epi32(v_aa, cycledv_b); // and again.
				cmp_mask = _mm_or_si128(cmp_mask, cmp_mask4);
				// convert the 128-bit mask to the 4-bit mask
				mask = _mm_movemask_ps((__m128 ) cmp_mask);
				count += _mm_popcnt_u32(mask); // a number of elements is a weight of the mask
				//]


				cmp_mask1 = _mm_cmpeq_epi32(v_a, v_bb); // pairwise comparison
				cycledv_b = _mm_shuffle_epi32(v_bb, cyclic_shift); // shuffling
				cmp_mask2 = _mm_cmpeq_epi32(v_a, cycledv_b); // again...
				cmp_mask = _mm_or_si128(cmp_mask1, cmp_mask2);
				cycledv_b = _mm_shuffle_epi32(cycledv_b, cyclic_shift);
				cmp_mask3 = _mm_cmpeq_epi32(v_a, cycledv_b); // and again...
				cmp_mask = _mm_or_si128(cmp_mask, cmp_mask3);
				cycledv_b = _mm_shuffle_epi32(cycledv_b, cyclic_shift);
				cmp_mask4 = _mm_cmpeq_epi32(v_a, cycledv_b); // and again.
				cmp_mask = _mm_or_si128(cmp_mask, cmp_mask4);
				// convert the 128-bit mask to the 4-bit mask
				mask = _mm_movemask_ps((__m128 ) cmp_mask);
				count += _mm_popcnt_u32(mask); // a number of elements is a weight of the mask
				//]
			}

			const uint32_t a_max = A[i_a + BlockSize - 1];
			//const uint32_t b_max = B[i_b + 3];
			if (a_max <= B[i_b + BlockSize - 1]) {
				//do {
				i_a += BlockSize;
				if (i_a >= st_a)
					goto end;
				//	} while(A[i_a + BlockSize -1] < B[i_b ]);
				v_a = _mm_load_si128((__m128i *) &A[i_a]);
				v_aa = _mm_load_si128((__m128i *) &A[i_a + HalfBlockSize]);
			}
			if (a_max >= B[i_b + BlockSize - 1]) {
				//do {
				i_b += BlockSize;
				if (i_b >= st_b)
					goto end;
				//} while(B[i_b + BlockSize - 1]<A[i_a]);
				v_b = _mm_load_si128((__m128i *) &B[i_b]);
				v_bb = _mm_load_si128((__m128i *) &B[i_b + HalfBlockSize]);

			}

		}

	}
	end:

	// intersect the tail using scalar intersection
	while (i_a < s_a && i_b < s_b) {
		if (A[i_a] < B[i_b]) {
			i_a++;
		} else if (B[i_b] < A[i_a]) {
			i_b++;
		} else {
			count++;
			i_a++;
			i_b++;
		}
	}

	return count;
}

static size_t __simd4by4(const __m128i v_a, const __m128i v_b) {
	const static uint32_t cyclic_shift = _MM_SHUFFLE(0, 3, 2, 1);

	__m128i cycledv_b;
	__m128i cmp_mask1, cmp_mask2, cmp_mask3, cmp_mask4;
	__m128i cmp_mask;
	int mask;

	cmp_mask1 = _mm_cmpeq_epi32(v_a, v_b); // pairwise comparison
	cycledv_b = _mm_shuffle_epi32(v_b, cyclic_shift); // shuffling
	cmp_mask2 = _mm_cmpeq_epi32(v_a, cycledv_b); // again...
	cmp_mask = _mm_or_si128(cmp_mask1, cmp_mask2);
	cycledv_b = _mm_shuffle_epi32(cycledv_b, cyclic_shift);
	cmp_mask3 = _mm_cmpeq_epi32(v_a, cycledv_b); // and again...
	cmp_mask = _mm_or_si128(cmp_mask, cmp_mask3);
	cycledv_b = _mm_shuffle_epi32(cycledv_b, cyclic_shift);
	cmp_mask4 = _mm_cmpeq_epi32(v_a, cycledv_b); // and again.
	cmp_mask = _mm_or_si128(cmp_mask, cmp_mask4);
	// convert the 128-bit mask to the 4-bit mask
	mask = _mm_movemask_ps((__m128 ) cmp_mask);
	return _mm_popcnt_u32(mask); // a number of elements is a weight of the mask

}

/**
 * Vectorized version by D. Lemire.
 */
size_t widevector2_cardinality_intersect(const uint32_t *A, const size_t s_a,
		const uint32_t *B, const size_t s_b) {
	const size_t BlockSize = 8;
	const size_t HalfBlockSize = 4;

	size_t count = 0;
	size_t i_a = 0, i_b = 0;
	// trim lengths to be a multiple of 8
	size_t st_a = (s_a / BlockSize) * BlockSize;
	size_t st_b = (s_b / BlockSize) * BlockSize;

	if (i_a < st_a && i_b < st_b) {
		while (A[i_a + BlockSize - 1] < B[i_b]) {
			i_a += BlockSize;
			if (i_a >= st_a)
				goto end;
		}
		while (B[i_b + BlockSize - 1] < A[i_a]) {
			i_b += BlockSize;
			if (i_b >= st_b)
				goto end;
		};
		__m128i v_a, v_b, v_aa, v_bb;
		v_a = _mm_load_si128((__m128i *) &A[i_a]);
		v_aa = _mm_load_si128((__m128i *) &A[i_a + HalfBlockSize]);
		v_b = _mm_load_si128((__m128i *) &B[i_b]);
		v_bb = _mm_load_si128((__m128i *) &B[i_b + HalfBlockSize]);

		while (true) {

			if ((((A[i_a] - 1) ^ A[i_a + BlockSize - 1]) >> 16 == 0)
					and (((B[i_b] - 1) ^ B[i_b + BlockSize - 1]) >> 16 == 0)) {
				if ((A[i_a] ^ B[i_b]) >> 16 == 0) {

					// higher 16 bits are all equal
					//0b10101010 = 170
					const __m128i bva = _mm_blend_epi16(v_a,
							_mm_slli_si128(v_aa, 2), 170);
					const __m128i bvb = _mm_blend_epi16(v_b,
							_mm_slli_si128(v_bb, 2), 170);
					const __m128i res_v = _mm_cmpistrm(
							bva,
							bvb,
							_SIDD_UWORD_OPS | _SIDD_CMP_EQUAL_ANY
									| _SIDD_BIT_MASK);

					const int r = _mm_extract_epi32(res_v, 0);
					//__m128i p = _mm_shuffle_epi8(v_a, shuffle_mask16[r]);
					//_mm_storeu_si128((__m128i *) &C[count], p);
					count += _mm_popcnt_u32(r);
				}
			} else {
				count += __simd4by4(v_a, v_b);
				count += __simd4by4(v_aa, v_b);
				count += __simd4by4(v_a, v_bb);
				count += __simd4by4(v_aa, v_bb);
			}

			const uint32_t a_max = A[i_a + BlockSize - 1];
			//const uint32_t b_max = B[i_b + 3];
			if (a_max <= B[i_b + BlockSize - 1]) {
				//do {
				i_a += BlockSize;
				if (i_a >= st_a)
					goto end;
				//	} while(A[i_a + BlockSize -1] < B[i_b ]);
				v_a = _mm_load_si128((__m128i *) &A[i_a]);
				v_aa = _mm_load_si128((__m128i *) &A[i_a + HalfBlockSize]);
			}
			if (a_max >= B[i_b + BlockSize - 1]) {
				//do {
				i_b += BlockSize;
				if (i_b >= st_b)
					goto end;
				//} while(B[i_b + BlockSize - 1]<A[i_a]);
				v_b = _mm_load_si128((__m128i *) &B[i_b]);
				v_bb = _mm_load_si128((__m128i *) &B[i_b + HalfBlockSize]);

			}

		}

	}
	end:

	// intersect the tail using scalar intersection
	while (i_a < s_a && i_b < s_b) {
		if (A[i_a] < B[i_b]) {
			i_a++;
		} else if (B[i_b] < A[i_a]) {
			i_b++;
		} else {
			count++;
			i_a++;
			i_b++;
		}
	}

	return count;
}

/**
 * Vectorized version by D. Lemire.
 */
size_t widevector3_cardinality_intersect(const uint32_t *A, const size_t s_a,
		const uint32_t *B, const size_t s_b) {
	const size_t BlockSize = 8;
	const size_t HalfBlockSize = 4;

	size_t count = 0;
	size_t i_a = 0, i_b = 0;
	// trim lengths to be a multiple of 8
	size_t st_a = (s_a / BlockSize) * BlockSize;
	size_t st_b = (s_b / BlockSize) * BlockSize;

	if (i_a < st_a && i_b < st_b) {
		while (A[i_a + BlockSize - 1] < B[i_b]) {
			i_a += BlockSize;
			if (i_a >= st_a)
				goto end;
		}
		while (B[i_b + BlockSize - 1] < A[i_a]) {
			i_b += BlockSize;
			if (i_b >= st_b)
				goto end;
		};
		__m128i v_a, v_b, v_aa, v_bb;
		v_a = _mm_load_si128((__m128i *) &A[i_a]);
		v_aa = _mm_load_si128((__m128i *) &A[i_a + HalfBlockSize]);
		v_b = _mm_load_si128((__m128i *) &B[i_b]);
		v_bb = _mm_load_si128((__m128i *) &B[i_b + HalfBlockSize]);

		while (true) {

			if ((((A[i_a] - 1) ^ A[i_a + BlockSize - 1]) >> 16 == 0)
					and (((B[i_b] - 1) ^ B[i_b + BlockSize - 1]) >> 16 == 0)) {
				if ((A[i_a] ^ B[i_b]) >> 16 == 0) {
					// higher 16 bits are all equal
					//0b10101010 = 170
					const __m128i bva = _mm_blend_epi16(v_a,
							_mm_slli_si128(v_aa, 2), 170);
					const __m128i bvb = _mm_blend_epi16(v_b,
							_mm_slli_si128(v_bb, 2), 170);
					const __m128i res_v = _mm_cmpistrm(
							bva,
							bvb,
							_SIDD_UWORD_OPS | _SIDD_CMP_EQUAL_ANY
									| _SIDD_BIT_MASK);

					const int r = _mm_extract_epi32(res_v, 0);
					//__m128i p = _mm_shuffle_epi8(v_a, shuffle_mask16[r]);
					//_mm_storeu_si128((__m128i *) &C[count], p);
					count += _mm_popcnt_u32(r);
				}
			} else {
				count += __simd4by4(v_a, v_b);
				if (A[i_a + HalfBlockSize - 1] <= B[i_b + HalfBlockSize - 1]) {
					if (A[i_a + HalfBlockSize - 1]
							>= B[i_b + HalfBlockSize - 1]) {
						count += __simd4by4(v_aa, v_bb);
					} else {
						count += __simd4by4(v_aa, v_b);
						count += __simd4by4(v_aa, v_bb);
					}

				} else {
					if (A[i_a + HalfBlockSize - 1]
							>= B[i_b + HalfBlockSize - 1]) {
						count += __simd4by4(v_a, v_bb);
						count += __simd4by4(v_aa, v_bb);
					}
				}

			}

			const uint32_t a_max = A[i_a + BlockSize - 1];
			//const uint32_t b_max = B[i_b + 3];
			if (a_max <= B[i_b + BlockSize - 1]) {
				//do {
				i_a += BlockSize;
				if (i_a >= st_a)
					goto end;
				//	} while(A[i_a + BlockSize -1] < B[i_b ]);
				v_a = _mm_load_si128((__m128i *) &A[i_a]);
				v_aa = _mm_load_si128((__m128i *) &A[i_a + HalfBlockSize]);
			}
			if (a_max >= B[i_b + BlockSize - 1]) {
				//do {
				i_b += BlockSize;
				if (i_b >= st_b)
					goto end;
				//} while(B[i_b + BlockSize - 1]<A[i_a]);
				v_b = _mm_load_si128((__m128i *) &B[i_b]);
				v_bb = _mm_load_si128((__m128i *) &B[i_b + HalfBlockSize]);

			}

		}

	}
	end:

	// intersect the tail using scalar intersection
	while (i_a < s_a && i_b < s_b) {
		if (A[i_a] < B[i_b]) {
			i_a++;
		} else if (B[i_b] < A[i_a]) {
			i_b++;
		} else {
			count++;
			i_a++;
			i_b++;
		}
	}

	return count;
}

namespace highlyscalablewordpresscom {
/**
 * Taken almost verbatim from http://highlyscalable.wordpress.com/2012/06/05/fast-intersection-sorted-lists-sse/
 * (just for comparison)
 */
size_t cardinality_intersect_scalar(const uint32_t *A, const size_t s_a,
		const uint32_t *B, const size_t s_b) {
	size_t i_a = 0, i_b = 0;
	size_t counter = 0;

	while (i_a < s_a && i_b < s_b) {
		if (A[i_a] < B[i_b]) {
			i_a++;
		} else if (B[i_b] < A[i_a]) {
			i_b++;
		} else {
			counter++;
			i_a++;
			i_b++;
		}
	}
	return counter;
}

/**
 * Taken almost verbatim from http://highlyscalable.wordpress.com/2012/06/05/fast-intersection-sorted-lists-sse/
 */
static __m128i shuffle_mask[16]; // precomputed dictionary


int getBit(int value, int position) {
	return ((value & (1 << position)) >> position);
}

// a simple implementation, we don't care about performance here
void prepare_shuffling_dictionary() {
	for (int i = 0; i < 16; i++) {
		int counter = 0;
		char permutation[16];
		memset(permutation, 0xFF, sizeof(permutation));
		for (char b = 0; b < 4; b++) {
			if (getBit(i, b)) {
				permutation[counter++] = 4 * b;
				permutation[counter++] = 4 * b + 1;
				permutation[counter++] = 4 * b + 2;
				permutation[counter++] = 4 * b + 3;
			}
		}
		__m128i mask = _mm_loadu_si128((const __m128i *) permutation);
		shuffle_mask[i] = mask;
	}
}

/**
 * Taken almost verbatim from http://highlyscalable.wordpress.com/2012/06/05/fast-intersection-sorted-lists-sse/
 */
size_t cardinality_intersect_SIMD(const uint32_t *A, const size_t s_a,
		const uint32_t *B, const size_t s_b) {
	size_t count = 0;
	size_t i_a = 0, i_b = 0;

	// trim lengths to be a multiple of 4
	size_t st_a = (s_a / 4) * 4;
	size_t st_b = (s_b / 4) * 4;

	while (i_a < st_a && i_b < st_b) {
		//[ load segments of four 32-bit elements
		__m128i v_a = _mm_load_si128((__m128i *) &A[i_a]);
		__m128i v_b = _mm_load_si128((__m128i *) &B[i_b]);
		//]

		//[ move pointers
		const uint32_t a_max = A[i_a + 3];
		const uint32_t b_max = B[i_b + 3];
		i_a += (a_max <= b_max) * 4;
		i_b += (a_max >= b_max) * 4;
		//]

		//[ compute mask of common elements
		const uint32_t cyclic_shift = _MM_SHUFFLE(0, 3, 2, 1);
		__m128i cmp_mask1 = _mm_cmpeq_epi32(v_a, v_b); // pairwise comparison
		v_b = _mm_shuffle_epi32(v_b, cyclic_shift); // shuffling
		__m128i cmp_mask2 = _mm_cmpeq_epi32(v_a, v_b); // again...
		v_b = _mm_shuffle_epi32(v_b, cyclic_shift);
		__m128i cmp_mask3 = _mm_cmpeq_epi32(v_a, v_b); // and again...
		v_b = _mm_shuffle_epi32(v_b, cyclic_shift);
		__m128i cmp_mask4 = _mm_cmpeq_epi32(v_a, v_b); // and again.
		__m128i cmp_mask = _mm_or_si128(_mm_or_si128(cmp_mask1, cmp_mask2),
				_mm_or_si128(cmp_mask3, cmp_mask4)); // OR-ing of comparison masks
		// convert the 128-bit mask to the 4-bit mask
		const int mask = _mm_movemask_ps((__m128 ) cmp_mask);
		//]

		//[ copy out common elements
		//__m128i p = _mm_shuffle_epi8(v_a, shuffle_mask[mask]);
		//_mm_storeu_si128((__m128i*)&C[count], p);
		count += _mm_popcnt_u32(mask); // a number of elements is a weight of the mask
		//]
	}

	// intersect the tail using scalar intersection
	while (i_a < s_a && i_b < s_b) {
		if (A[i_a] < B[i_b]) {
			i_a++;
		} else if (B[i_b] < A[i_a]) {
			i_b++;
		} else {
			count++;
			i_a++;
			i_b++;
		}
	}

	return count;
}

/**
 * This is a version of cardinality_intersect_SIMD partially optimized
 * by D. Lemire.
 */
size_t opti_cardinality_intersect_SIMD(const uint32_t *A, const size_t s_a,
		const uint32_t *B, const size_t s_b) {
	size_t count = 0;
	size_t i_a = 0, i_b = 0;
	const static uint32_t cyclic_shift = _MM_SHUFFLE(0, 3, 2, 1);
	// trim lengths to be a multiple of 4
	size_t st_a = (s_a / 4) * 4;
	size_t st_b = (s_b / 4) * 4;
	if (i_a < st_a && i_b < st_b) {
		__m128i v_a, v_b;
		v_a = _mm_load_si128((__m128i *) &A[i_a]);
		v_b = _mm_load_si128((__m128i *) &B[i_b]);
		while (true) {
			const __m128i cmp_mask1 = _mm_cmpeq_epi32(v_a, v_b); // pairwise comparison
			__m128i cycledv_b = _mm_shuffle_epi32(v_b, cyclic_shift); // shuffling
			const __m128i cmp_mask2 = _mm_cmpeq_epi32(v_a, cycledv_b); // again...
			__m128i cmp_mask = _mm_or_si128(cmp_mask1, cmp_mask2);
			cycledv_b = _mm_shuffle_epi32(cycledv_b, cyclic_shift);
			const __m128i cmp_mask3 = _mm_cmpeq_epi32(v_a, cycledv_b); // and again...
			cmp_mask = _mm_or_si128(cmp_mask, cmp_mask3);
			cycledv_b = _mm_shuffle_epi32(cycledv_b, cyclic_shift);
			const __m128i cmp_mask4 = _mm_cmpeq_epi32(v_a, cycledv_b); // and again.
			cmp_mask = _mm_or_si128(cmp_mask, cmp_mask4);
			// convert the 128-bit mask to the 4-bit mask
			const int mask = _mm_movemask_ps((__m128 ) cmp_mask);
			count += _mm_popcnt_u32(mask); // a number of elements is a weight of the mask
			//]

			//[ copy out common elements
			//__m128i p = _mm_shuffle_epi8(v_a, shuffle_mask[mask]);
			//_mm_storeu_si128((__m128i*)&C[count], p);
			//]
			const uint32_t a_max = A[i_a + 3];
			//const uint32_t b_max = B[i_b + 3];
			if (a_max <= B[i_b + 3]) {
				i_a += 4;
				if (i_a >= st_a)
					break;
				v_a = _mm_load_si128((__m128i *) &A[i_a]);
			}
			if (a_max >= B[i_b + 3]) {
				i_b += 4;
				if (i_b >= st_b)
					break;
				v_b = _mm_load_si128((__m128i *) &B[i_b]);
			}

		}
	}

	// intersect the tail using scalar intersection
	while (i_a < s_a && i_b < s_b) {
		if (A[i_a] < B[i_b]) {
			i_a++;
		} else if (B[i_b] < A[i_a]) {
			i_b++;
		} else {
			count++;
			i_a++;
			i_b++;
		}
	}

	return count;
}

size_t opti8_cardinality_intersect_SIMD(const uint32_t *A, const size_t s_a,
		const uint32_t *B, const size_t s_b) {
	const size_t BlockSize = 8;
	const size_t HalfBlockSize = 4;

	size_t count = 0;
	size_t i_a = 0, i_b = 0;
	const static uint32_t cyclic_shift = _MM_SHUFFLE(0, 3, 2, 1);
	// trim lengths to be a multiple of 4
	size_t st_a = (s_a / BlockSize) * BlockSize;
	size_t st_b = (s_b / BlockSize) * BlockSize;

	if (i_a < st_a && i_b < st_b) {
		/*while (A[i_a + BlockSize - 1] < B[i_b]) {
		 i_a += BlockSize;
		 if (i_a >= st_a)
		 goto end;
		 }
		 while (B[i_b + BlockSize - 1] < A[i_a]) {
		 i_b += BlockSize;
		 if (i_b >= st_b)
		 goto end;
		 };*/

		__m128i v_a, v_b, v_aa, v_bb;
		v_a = _mm_load_si128((__m128i *) &A[i_a]);
		v_aa = _mm_load_si128((__m128i *) &A[i_a + HalfBlockSize]);
		v_b = _mm_load_si128((__m128i *) &B[i_b]);
		v_bb = _mm_load_si128((__m128i *) &B[i_b + HalfBlockSize]);

		while (true) {
			__m128i cycledv_b;
			__m128i cmp_mask1, cmp_mask2, cmp_mask3, cmp_mask4;
			__m128i cmp_mask;
			int mask;

			cmp_mask1 = _mm_cmpeq_epi32(v_a, v_b); // pairwise comparison
			cycledv_b = _mm_shuffle_epi32(v_b, cyclic_shift); // shuffling
			cmp_mask2 = _mm_cmpeq_epi32(v_a, cycledv_b); // again...
			cmp_mask = _mm_or_si128(cmp_mask1, cmp_mask2);
			cycledv_b = _mm_shuffle_epi32(cycledv_b, cyclic_shift);
			cmp_mask3 = _mm_cmpeq_epi32(v_a, cycledv_b); // and again...
			cmp_mask = _mm_or_si128(cmp_mask, cmp_mask3);
			cycledv_b = _mm_shuffle_epi32(cycledv_b, cyclic_shift);
			cmp_mask4 = _mm_cmpeq_epi32(v_a, cycledv_b); // and again.
			cmp_mask = _mm_or_si128(cmp_mask, cmp_mask4);
			// convert the 128-bit mask to the 4-bit mask
			mask = _mm_movemask_ps((__m128 ) cmp_mask);
			count += _mm_popcnt_u32(mask); // a number of elements is a weight of the mask
			//]

			//[ copy out common elements
			//__m128i p = _mm_shuffle_epi8(v_a, shuffle_mask[mask]);
			//_mm_storeu_si128((__m128i*)&C[count], p);
			//]

			cmp_mask1 = _mm_cmpeq_epi32(v_aa, v_bb); // pairwise comparison
			cycledv_b = _mm_shuffle_epi32(v_bb, cyclic_shift); // shuffling
			cmp_mask2 = _mm_cmpeq_epi32(v_aa, cycledv_b); // again...
			cmp_mask = _mm_or_si128(cmp_mask1, cmp_mask2);
			cycledv_b = _mm_shuffle_epi32(cycledv_b, cyclic_shift);
			cmp_mask3 = _mm_cmpeq_epi32(v_aa, cycledv_b); // and again...
			cmp_mask = _mm_or_si128(cmp_mask, cmp_mask3);
			cycledv_b = _mm_shuffle_epi32(cycledv_b, cyclic_shift);
			cmp_mask4 = _mm_cmpeq_epi32(v_aa, cycledv_b); // and again.
			cmp_mask = _mm_or_si128(cmp_mask, cmp_mask4);
			// convert the 128-bit mask to the 4-bit mask
			mask = _mm_movemask_ps((__m128 ) cmp_mask);
			count += _mm_popcnt_u32(mask); // a number of elements is a weight of the mask
			//]


			cmp_mask1 = _mm_cmpeq_epi32(v_aa, v_b); // pairwise comparison
			cycledv_b = _mm_shuffle_epi32(v_b, cyclic_shift); // shuffling
			cmp_mask2 = _mm_cmpeq_epi32(v_aa, cycledv_b); // again...
			cmp_mask = _mm_or_si128(cmp_mask1, cmp_mask2);
			cycledv_b = _mm_shuffle_epi32(cycledv_b, cyclic_shift);
			cmp_mask3 = _mm_cmpeq_epi32(v_aa, cycledv_b); // and again...
			cmp_mask = _mm_or_si128(cmp_mask, cmp_mask3);
			cycledv_b = _mm_shuffle_epi32(cycledv_b, cyclic_shift);
			cmp_mask4 = _mm_cmpeq_epi32(v_aa, cycledv_b); // and again.
			cmp_mask = _mm_or_si128(cmp_mask, cmp_mask4);
			// convert the 128-bit mask to the 4-bit mask
			mask = _mm_movemask_ps((__m128 ) cmp_mask);
			count += _mm_popcnt_u32(mask); // a number of elements is a weight of the mask
			//]


			cmp_mask1 = _mm_cmpeq_epi32(v_a, v_bb); // pairwise comparison
			cycledv_b = _mm_shuffle_epi32(v_bb, cyclic_shift); // shuffling
			cmp_mask2 = _mm_cmpeq_epi32(v_a, cycledv_b); // again...
			cmp_mask = _mm_or_si128(cmp_mask1, cmp_mask2);
			cycledv_b = _mm_shuffle_epi32(cycledv_b, cyclic_shift);
			cmp_mask3 = _mm_cmpeq_epi32(v_a, cycledv_b); // and again...
			cmp_mask = _mm_or_si128(cmp_mask, cmp_mask3);
			cycledv_b = _mm_shuffle_epi32(cycledv_b, cyclic_shift);
			cmp_mask4 = _mm_cmpeq_epi32(v_a, cycledv_b); // and again.
			cmp_mask = _mm_or_si128(cmp_mask, cmp_mask4);
			// convert the 128-bit mask to the 4-bit mask
			mask = _mm_movemask_ps((__m128 ) cmp_mask);
			count += _mm_popcnt_u32(mask); // a number of elements is a weight of the mask
			//]


			const uint32_t a_max = A[i_a + BlockSize - 1];
			//const uint32_t b_max = B[i_b + 3];
			if (a_max <= B[i_b + BlockSize - 1]) {
				//do {
				i_a += BlockSize;
				if (i_a >= st_a)
					goto end;
				//} while(A[i_a + BlockSize -1] < B[i_b ]);
				v_a = _mm_load_si128((__m128i *) &A[i_a]);
				v_aa = _mm_load_si128((__m128i *) &A[i_a + HalfBlockSize]);
			}
			if (a_max >= B[i_b + BlockSize - 1]) {
				//do {
				i_b += BlockSize;
				if (i_b >= st_b)
					goto end;
				//} while(B[i_b + BlockSize - 1]<A[i_a]);
				v_b = _mm_load_si128((__m128i *) &B[i_b]);
				v_bb = _mm_load_si128((__m128i *) &B[i_b + HalfBlockSize]);

			}

		}

	}
	end:

	// intersect the tail using scalar intersection
	while (i_a < s_a && i_b < s_b) {
		if (A[i_a] < B[i_b]) {
			i_a++;
		} else if (B[i_b] < A[i_a]) {
			i_b++;
		} else {
			count++;
			i_a++;
			i_b++;
		}
	}

	return count;
}

size_t opti2_cardinality_intersect_SIMD(const uint32_t *A, const size_t s_a,
		const uint32_t *B, const size_t s_b) {
	size_t count = 0;
	size_t i_a = 0, i_b = 0;
	const static uint32_t cyclic_shift1 = _MM_SHUFFLE(0, 3, 2, 1);
	const static uint32_t cyclic_shift2 = _MM_SHUFFLE(1, 0, 3, 2);
	const static uint32_t cyclic_shift3 = _MM_SHUFFLE(2, 1, 0, 3);

	// trim lengths to be a multiple of 4
	size_t st_a = (s_a / 4) * 4;
	size_t st_b = (s_b / 4) * 4;
	if (i_a < st_a && i_b < st_b) {
		__m128i v_a, v_b;
		v_a = _mm_load_si128((__m128i *) &A[i_a]);
		v_b = _mm_load_si128((__m128i *) &B[i_b]);
		while (true) {
			const __m128i cmp_mask1 = _mm_cmpeq_epi32(v_a, v_b); // pairwise comparison
			const __m128i cmp_mask2 = _mm_cmpeq_epi32(v_a,
					_mm_shuffle_epi32(v_b, cyclic_shift1)); // again...
			__m128i cmp_mask = _mm_or_si128(cmp_mask1, cmp_mask2);
			const __m128i cmp_mask3 = _mm_cmpeq_epi32(v_a,
					_mm_shuffle_epi32(v_b, cyclic_shift2)); // and again...
			cmp_mask = _mm_or_si128(cmp_mask, cmp_mask3);
			const __m128i cmp_mask4 = _mm_cmpeq_epi32(v_a,
					_mm_shuffle_epi32(v_b, cyclic_shift3)); // and again.
			cmp_mask = _mm_or_si128(cmp_mask, cmp_mask4);
			// convert the 128-bit mask to the 4-bit mask
			const int mask = _mm_movemask_ps((__m128 ) cmp_mask);
			count += _mm_popcnt_u32(mask); // a number of elements is a weight of the mask
			//]

			//[ copy out common elements
			//__m128i p = _mm_shuffle_epi8(v_a, shuffle_mask[mask]);
			//_mm_storeu_si128((__m128i*)&C[count], p);
			//]
			const uint32_t a_max = A[i_a + 3];
			//const uint32_t b_max = B[i_b + 3];
			if (a_max <= B[i_b + 3]) {
				i_a += 4;
				if (i_a >= st_a)
					break;
				v_a = _mm_load_si128((__m128i *) &A[i_a]);
			}
			if (a_max >= B[i_b + 3]) {
				i_b += 4;
				if (i_b >= st_b)
					break;
				v_b = _mm_load_si128((__m128i *) &B[i_b]);
			}

		}
	}

	// intersect the tail using scalar intersection
	while (i_a < s_a && i_b < s_b) {
		if (A[i_a] < B[i_b]) {
			i_a++;
		} else if (B[i_b] < A[i_a]) {
			i_b++;
		} else {
			count++;
			i_a++;
			i_b++;
		}
	}

	return count;
}

/**
 * Silly function.
 */
uint16_t _high16(uint32_t x) {
	return x >> 16;
}
/**
 * Another function.
 */
uint16_t _low16(uint32_t x) {
	return x;
}

/**
 * From Schlegel et al., Fast Sorted-Set Intersection using SIMD Instructions
 */
// A - sorted array
// s_a - size of A
// R - partitioned sorted array
size_t partition(const uint32_t *A, const size_t s_a, uint16_t *R, const size_t /*Rlength*/) {
	uint16_t high = 0;
	size_t partition_length = 0;
	size_t partition_size_position = 1;
	size_t counter = 0;
	size_t p = 0;
	if (p < s_a) {
		uint16_t chigh = _high16(A[p]); // upper dword
		uint16_t clow = _low16(A[p]); // lower dword
		if (chigh == 0) {
			R[counter++] = chigh; // partition prefix
			R[counter++] = 0; // reserve place for partition size
			R[counter++] = clow; // write the first element
			partition_length = 1; // reset counters
			//R[partition_size_position] = partition_length - 1;  // store "-1"
			//partition_size_position = counter - 2;
			high = chigh;
			++p;
		}

	}
	for (; p < s_a; p++) {
		uint16_t chigh = _high16(A[p]); // upper dword
		uint16_t clow = _low16(A[p]); // lower dword
		if (chigh == high && p != 0) { // add element to the current partition
			R[counter++] = clow;
			partition_length++;
		} else { // start new partition
			R[counter++] = chigh; // partition prefix
			R[counter++] = 0; // reserve place for partition size
			R[counter++] = clow; // write the first element
			R[partition_size_position] = partition_length - 1; // store "-1"
			partition_length = 1; // reset counters
			partition_size_position = counter - 2;
			high = chigh;
		}
	}
	R[partition_size_position] = partition_length - 1;

	return counter;
}

/**
 * Useful for debugging purposes.
 */
size_t inverse_partition(uint32_t *A, const size_t /*s_a*/, const uint16_t *R,
		const size_t Rlength) {
	size_t i = 0;
	size_t p = 0;
	while (i < Rlength) {
		uint16_t chigh = R[i++];
		size_t sizepart = static_cast<size_t> (R[i++]) + 1;
		while (sizepart > 0) {
			uint16_t clow = R[i++];
			A[p++] = (static_cast<uint32_t> (chigh) << 16) | clow;
			--sizepart;
		}
	}
	return p;
}

/**
 * From Schlegel et al., Fast Sorted-Set Intersection using SIMD Instructions
 *
 * Optimized  by D. Lemire on April 30th 2013
 */
static size_t cardinality_intersect_vector16(const uint16_t *A,
		const uint16_t *B, const size_t s_a, const size_t s_b/*, uint16_t *C*/) {
	size_t count = 0;
	size_t i_a = 0, i_b = 0;

	const size_t st_a = (s_a / 8) * 8;
	const size_t st_b = (s_b / 8) * 8;
	__m128i v_a, v_b;
	if ((i_a < st_a) and (i_b < st_b)) {
		v_a = _mm_loadu_si128((__m128i *) &A[i_a]);
		v_b = _mm_loadu_si128((__m128i *) &B[i_b]);

		while (true) {
			const __m128i res_v = _mm_cmpestrm(v_b, 8, v_a, 8,
					_SIDD_UWORD_OPS | _SIDD_CMP_EQUAL_ANY | _SIDD_BIT_MASK);
			const int r = _mm_extract_epi32(res_v, 0);
			//__m128i p = _mm_shuffle_epi8(v_a, shuffle_mask16[r]);
			//_mm_storeu_si128((__m128i *) &C[count], p);
			count += _mm_popcnt_u32(r);
			const uint16_t a_max = A[i_a + 7];
			const uint16_t b_max = B[i_b + 7];
			if (a_max <= b_max) {
				i_a += 8;
				if (i_a == st_a)
					break;
				v_a = _mm_loadu_si128((__m128i *) &A[i_a]);

			}
			if (b_max <= a_max) {
				i_b += 8;
				if (i_b == st_b)
					break;
				v_b = _mm_loadu_si128((__m128i *) &B[i_b]);

			}
		}
	}
	// intersect the tail using scalar intersection
	while (i_a < s_a && i_b < s_b) {
		if (A[i_a] < B[i_b]) {
			i_a++;
		} else if (B[i_b] < A[i_a]) {
			i_b++;
		} else {
			count++;
			i_a++;
			i_b++;
		}
	}

	return count;
}

/**
 * From Schlegel et al., Fast Sorted-Set Intersection using SIMD Instructions
 *
 * Optimized by D. Lemire on May 3rd 2013
 */
static size_t faster_cardinality_intersect_vector16(const uint16_t *A,
		const uint16_t *B, const size_t s_a, const size_t s_b/*, uint16_t *C*/) {
	size_t count = 0;
	size_t i_a = 0, i_b = 0;

	const size_t st_a = (s_a / 8) * 8;
	const size_t st_b = (s_b / 8) * 8;
	__m128i v_a, v_b;
	if ((i_a < st_a) and (i_b < st_b)) {
		v_a = _mm_loadu_si128((__m128i *) &A[i_a]);
		v_b = _mm_loadu_si128((__m128i *) &B[i_b]);
		while ((A[i_a] == 0) or (B[i_b] == 0)) {
			const __m128i res_v = _mm_cmpestrm(v_b, 8, v_a, 8,
					_SIDD_UWORD_OPS | _SIDD_CMP_EQUAL_ANY | _SIDD_BIT_MASK);
			const int r = _mm_extract_epi32(res_v, 0);
			//__m128i p = _mm_shuffle_epi8(v_a, shuffle_mask16[r]);
			//_mm_storeu_si128((__m128i *) &C[count], p);
			count += _mm_popcnt_u32(r);
			const uint16_t a_max = A[i_a + 7];
			const uint16_t b_max = B[i_b + 7];
			if (a_max <= b_max) {
				i_a += 8;
				if (i_a == st_a)
					break;
				v_a = _mm_loadu_si128((__m128i *) &A[i_a]);

			}
			if (b_max <= a_max) {
				i_b += 8;
				if (i_b == st_b)
					break;
				v_b = _mm_loadu_si128((__m128i *) &B[i_b]);

			}

		}
		if ((i_a < st_a) and (i_b < st_b))
			while (true) {
				const __m128i res_v = _mm_cmpistrm(v_b, v_a,
						_SIDD_UWORD_OPS | _SIDD_CMP_EQUAL_ANY | _SIDD_BIT_MASK);
				const int r = _mm_extract_epi32(res_v, 0);
				//__m128i p = _mm_shuffle_epi8(v_a, shuffle_mask16[r]);
				//_mm_storeu_si128((__m128i *) &C[count], p);
				count += _mm_popcnt_u32(r);
				const uint16_t a_max = A[i_a + 7];
				const uint16_t b_max = B[i_b + 7];
				if (a_max <= b_max) {
					i_a += 8;
					if (i_a == st_a)
						break;
					v_a = _mm_loadu_si128((__m128i *) &A[i_a]);

				}
				if (b_max <= a_max) {
					i_b += 8;
					if (i_b == st_b)
						break;
					v_b = _mm_loadu_si128((__m128i *) &B[i_b]);

				}
			}
	}
	// intersect the tail using scalar intersection
	while (i_a < s_a && i_b < s_b) {
		if (A[i_a] < B[i_b]) {
			i_a++;
		} else if (B[i_b] < A[i_a]) {
			i_b++;
		} else {
			count++;
			i_a++;
			i_b++;
		}
	}

	return count;
}

/**
 * Strictly for testing/debugging purposes.
 */
size_t scalar_cardinality_intersect_vector16(const uint16_t *A,
		const uint16_t *B, const size_t s_a, const size_t s_b/*, uint16_t *C*/) {
	size_t count = 0;
	size_t i_a = 0, i_b = 0;
	// intersect the tail using scalar intersection
	while (i_a < s_a && i_b < s_b) {
		if (A[i_a] < B[i_b]) {
			i_a++;
		} else if (B[i_b] < A[i_a]) {
			i_b++;
		} else {
			count++;
			i_a++;
			i_b++;
		}
	}

	return count;
}

/**
 * From Schlegel et al., Fast Sorted-Set Intersection using SIMD Instructions
 */
// A, B - partitioned operands
size_t cardinality_intersect_partitioned(const uint16_t *A, const uint16_t *B,
		const size_t s_a, const size_t s_b) {
	size_t i_a = 0, i_b = 0;
	size_t counter = 0;
	while (i_a < s_a && i_b < s_b) {
		if (A[i_a] < B[i_b]) {
			i_a += static_cast<size_t> (A[i_a + 1]) + 2 + 1;
		} else if (B[i_b] < A[i_a]) {
			i_b += static_cast<size_t> (B[i_b + 1]) + 2 + 1;
		} else {
			//C[counter++] = A[i_a]; // write partition prefix
			size_t partition_size = cardinality_intersect_vector16(&A[i_a + 2],
					&B[i_b + 2], static_cast<size_t> (A[i_a + 1]) + 1,
					static_cast<size_t> (B[i_b + 1]) + 1);//, &C[counter + 1]);
			//C[counter++] = partition_size; // write partition size
			counter += partition_size;
			i_a += static_cast<size_t> (A[i_a + 1]) + 2 + 1;
			i_b += static_cast<size_t> (B[i_b + 1]) + 2 + 1;
		}
	}
	//std::cout<<"partcounter = "<< partcounter<<std::endl;
	return counter;
}

/**
 * Version optimized by D. Lemire of
 * From Schlegel et al., Fast Sorted-Set Intersection using SIMD Instructions
 */
size_t faster_cardinality_intersect_partitioned(const uint16_t *A,
		const uint16_t *B, const size_t s_a, const size_t s_b) {
	size_t i_a = 0, i_b = 0;
	size_t counter = 0;
	while (i_a < s_a && i_b < s_b) {
		if (A[i_a] < B[i_b]) {
			i_a += static_cast<size_t> (A[i_a + 1]) + 2 + 1;
		} else if (B[i_b] < A[i_a]) {
			i_b += static_cast<size_t> (B[i_b + 1]) + 2 + 1;
		} else {
			size_t partition_size = faster_cardinality_intersect_vector16(
					&A[i_a + 2], &B[i_b + 2],
					static_cast<size_t> (A[i_a + 1]) + 1,
					static_cast<size_t> (B[i_b + 1]) + 1);//, &C[counter + 1]);
			//C[counter++] = partition_size; // write partition size
			counter += partition_size;
			i_a += static_cast<size_t> (A[i_a + 1]) + 2 + 1;
			i_b += static_cast<size_t> (B[i_b + 1]) + 2 + 1;
		}
	}
	return counter;
}

size_t faster2_cardinality_intersect_partitioned(const uint16_t *A,
		const uint16_t *B, const size_t s_a, const size_t s_b) {
	size_t i_a = 0, i_b = 0;
	size_t counter = 0;
	while (i_a < s_a && i_b < s_b) {
		if (A[i_a] < B[i_b]) {
			do {
				i_a += static_cast<size_t> (A[i_a + 1]) + 2 + 1;
				if (i_a >= s_a)
					goto end;
			} while (A[i_a] < B[i_b]);
		}
		if (B[i_b] < A[i_a]) {
			do {
				i_b += static_cast<size_t> (B[i_b + 1]) + 2 + 1;
				if (i_b >= s_b)
					goto end;
			} while (B[i_b] < A[i_a]);
		} else {
			size_t partition_size = faster_cardinality_intersect_vector16(
					&A[i_a + 2], &B[i_b + 2],
					static_cast<size_t> (A[i_a + 1]) + 1,
					static_cast<size_t> (B[i_b + 1]) + 1);//, &C[counter + 1]);
			//C[counter++] = partition_size; // write partition size
			counter += partition_size;
			i_a += static_cast<size_t> (A[i_a + 1]) + 2 + 1;
			i_b += static_cast<size_t> (B[i_b + 1]) + 2 + 1;
		}
	}
	end: return counter;
}

}

#define VEC __m128i
#define VECLEN (sizeof(VEC)/sizeof(uint32_t))
#define VECMAX (VECLEN - 1)

size_t nate_count_scalar(const uint32_t *A, const size_t lenA,
		const uint32_t *B, const size_t lenB) {

	size_t count = 0;
	if (lenA == 0 || lenB == 0)
		return count;

	const uint32_t *endA = A + lenA;
	const uint32_t *endB = B + lenB;

	while (1) {
		while (*A < *B) {
			SKIP_FIRST_COMPARE: if (++A == endA)
				return count;
		}
		while (*A > *B) {
			if (++B == endB)
				return count;
		}
		if (*A == *B) {
			count++;
			if (++A == endA || ++B == endB)
				return count;
		} else {
			goto SKIP_FIRST_COMPARE;
		}
	}

	return count; // NOTREACHED
}

size_t nate_count_medium(const uint32_t *rare, const size_t lenRare,
		const uint32_t *freq, const size_t lenFreq) {

	// FUTURE: could swap freq and rare if inverted

	size_t count = 0;
	if (lenFreq == 0 || lenRare == 0)
		return count;

#define FREQSPACE (8 * VECLEN)
#define RARESPACE 1
	const uint32_t *stopFreq = freq + lenFreq - FREQSPACE;
	const uint32_t *stopRare = rare + lenRare - RARESPACE;

	if (freq > stopFreq) {
		return nate_count_scalar(freq, lenFreq, rare, lenRare);
	}

	uint32_t nextRare = *rare;
	uint32_t maxFreq = freq[7 * VECLEN + VECMAX];
	VEC M0, M1, M2, M3, M4, M5, M6, M7;
	VEC A0, A1, A2, A3;
	A0 = _mm_load_si128((VEC *) freq + 0);
	A1 = _mm_load_si128((VEC *) freq + 1);
	A2 = _mm_load_si128((VEC *) freq + 2);
	A3 = _mm_load_si128((VEC *) freq + 3);

	while (maxFreq < nextRare) { // advance freq to a possible match
		freq += VECLEN * 8; // NOTE: loop below requires this
		if (freq > stopFreq)
			goto FINISH_SCALAR;
		maxFreq = freq[VECLEN * 7 + VECMAX];
		A0 = _mm_load_si128((VEC *) freq + 0);
		A1 = _mm_load_si128((VEC *) freq + 1);
		A2 = _mm_load_si128((VEC *) freq + 2);
		A3 = _mm_load_si128((VEC *) freq + 3);
	}

	while (rare < stopRare) {
		uint32_t matchRare = nextRare;
		VEC Match = _mm_set1_epi32(matchRare);
		if (maxFreq < matchRare) { // if no match possible
			freq += VECLEN * 8; // advance 8 vectors
			if (freq > stopFreq)
				goto FINISH_SCALAR;
			maxFreq = freq[VECLEN * 7 + VECMAX];
			while (maxFreq < matchRare) { // if still no match possible
				freq += VECLEN * 8; // advance another 8 vectors
				if (freq > stopFreq)
					goto FINISH_SCALAR;
				maxFreq = freq[VECLEN * 7 + VECMAX];
				// printf("FREQ_ADVANCE2: %d %d\n", matchRare, maxFreq);
				M0 = _mm_load_si128((VEC *) freq + 0);
				M1 = _mm_load_si128((VEC *) freq + 1);
				M2 = _mm_load_si128((VEC *) freq + 2);
				M3 = _mm_load_si128((VEC *) freq + 3);
			}
			A0 = M0; // ratchet original vectors forward
			A1 = M1;
			A2 = M2;
			A3 = M3;
		} else { // if a match is still possible
			M0 = A0; // revert to original vectors
			M1 = A1;
			M2 = A2;
			M3 = A3;
		}

		rare += 1;
		nextRare = *rare;
		// NOTE: At this point M0, M1, M2, M3 must match freq + 0,1,2,3
		VEC Q0, Q1, Q2, Q3; // quarters
		VEC S0, S1; // semis
		VEC F0; // final

		M4 = _mm_load_si128((VEC *) freq + 4);
		M5 = _mm_load_si128((VEC *) freq + 5);
		M6 = _mm_load_si128((VEC *) freq + 6);
		M7 = _mm_load_si128((VEC *) freq + 7);

		M0 = _mm_cmpeq_epi32(M0, Match);
		M1 = _mm_cmpeq_epi32(M1, Match);
		Q0 = _mm_or_si128(M0, M1);
		M0 = _mm_load_si128((VEC *) freq + 8);
		M1 = _mm_load_si128((VEC *) freq + 9);

		M2 = _mm_cmpeq_epi32(M2, Match);
		M3 = _mm_cmpeq_epi32(M3, Match);
		Q1 = _mm_or_si128(M2, M3);
		M2 = _mm_load_si128((VEC *) freq + 10);
		M3 = _mm_load_si128((VEC *) freq + 11);

		M4 = _mm_cmpeq_epi32(M4, Match);
		M5 = _mm_cmpeq_epi32(M5, Match);
		Q2 = _mm_or_si128(M4, M5);

		M6 = _mm_cmpeq_epi32(M6, Match);
		M7 = _mm_cmpeq_epi32(M7, Match);
		Q3 = _mm_or_si128(M6, M7);

		S0 = _mm_or_si128(Q0, Q1);
		S1 = _mm_or_si128(Q2, Q3);
		F0 = _mm_or_si128(S0, S1);

		// PROFILE: check that this compiles to a CMOV
		if (_mm_testz_si128(F0, F0)) {
			// printf("NOT_FOUND: %d %d\n", matchRare, maxFreq);
		} else {
			// printf("FOUND: %d %d\n", matchRare, maxFreq);
			count += 1;
		}
	}

	FINISH_SCALAR: return count + nate_count_scalar(freq,
			stopFreq + FREQSPACE - freq, rare, stopRare + RARESPACE - rare);
}

/**
 * Version hacked by D. Lemire, original by Nathan Kurz
 */
size_t nate2_count_medium(const uint32_t *rare, const size_t lenRare,
		const uint32_t *freq, const size_t lenFreq) {
	// FUTURE: could swap freq and rare if inverted
	size_t count = 0;
	if (lenFreq == 0 || lenRare == 0)
		return count;

	typedef __m128i vec;
	const uint32_t veclen = sizeof(VEC) / sizeof(uint32_t);
	const size_t vecmax = veclen - 1;
	const size_t freqspace = 8 * veclen;
	const size_t rarespace = 1;

	const uint32_t *stopFreq = freq + lenFreq - freqspace;
	const uint32_t *stopRare = rare + lenRare - rarespace;
	if (freq > stopFreq) {
		return nate_count_scalar(freq, lenFreq, rare, lenRare);
	}
	uint32_t maxFreq = freq[7 * veclen + vecmax];
	vec M0, M1, M2, M3, M4, M5, M6, M7;

	while (maxFreq < *rare) { // advance freq to a possible match
		freq += veclen * 8; // NOTE: loop below requires this
		if (freq > stopFreq)
			goto FINISH_SCALAR;
		maxFreq = freq[veclen * 7 + vecmax];
	}
	M0 = _mm_load_si128((vec *) freq + 0);
	M1 = _mm_load_si128((vec *) freq + 1);
	M2 = _mm_load_si128((vec *) freq + 2);
	M3 = _mm_load_si128((vec *) freq + 3);
	M4 = _mm_load_si128((vec *) freq + 4);
	M5 = _mm_load_si128((vec *) freq + 5);
	M6 = _mm_load_si128((vec *) freq + 6);
	M7 = _mm_load_si128((vec *) freq + 7);

	for (; rare < stopRare; ++rare) {
		const uint32_t matchRare = *rare;//nextRare;
		const vec Match = _mm_set1_epi32(matchRare);
		if (maxFreq < matchRare) { // if no match possible
			freq += veclen * 8; // advance 8 vectors
			if (freq > stopFreq)
				goto FINISH_SCALAR;
			maxFreq = freq[veclen * 7 + vecmax];
			while (maxFreq < matchRare) { // if still no match possible
				freq += veclen * 8; // advance another 8 vectors
				if (freq > stopFreq)
					goto FINISH_SCALAR;
				maxFreq = freq[veclen * 7 + vecmax];
			}
			M0 = _mm_load_si128((vec *) freq + 0);
			M1 = _mm_load_si128((vec *) freq + 1);
			M2 = _mm_load_si128((vec *) freq + 2);
			M3 = _mm_load_si128((vec *) freq + 3);
			M4 = _mm_load_si128((vec *) freq + 4);
			M5 = _mm_load_si128((vec *) freq + 5);
			M6 = _mm_load_si128((vec *) freq + 6);
			M7 = _mm_load_si128((vec *) freq + 7);

		}
		const vec Q0 = _mm_or_si128(_mm_cmpeq_epi32(M0, Match),
				_mm_cmpeq_epi32(M1, Match));
		const vec Q1 = _mm_or_si128(_mm_cmpeq_epi32(M2, Match),
				_mm_cmpeq_epi32(M3, Match));
		const vec Q2 = _mm_or_si128(_mm_cmpeq_epi32(M4, Match),
				_mm_cmpeq_epi32(M5, Match));
		const vec Q3 = _mm_or_si128(_mm_cmpeq_epi32(M6, Match),
				_mm_cmpeq_epi32(M7, Match));
		const vec S0 = _mm_or_si128(Q0, Q1);
		const vec S1 = _mm_or_si128(Q2, Q3);
		const vec F0 = _mm_or_si128(S0, S1);
		if (_mm_testz_si128(F0, F0)) {
		} else {
			count += 1;
		}
	}

	FINISH_SCALAR: return count + nate_count_scalar(freq,
			stopFreq + FREQSPACE - freq, rare, stopRare + RARESPACE - rare);
}

/**
 * Version hacked by D. Lemire, original by Nathan Kurz
 */
size_t nate3_count_medium(const uint32_t *rare, const size_t lenRare,
		const uint32_t *freq, const size_t lenFreq) {
	// FUTURE: could swap freq and rare if inverted
	size_t count = 0;
	if (lenFreq == 0 || lenRare == 0)
		return count;

	typedef __m128i vec;
	const uint32_t veclen = sizeof(VEC) / sizeof(uint32_t);
	const size_t vecmax = veclen - 1;
	const size_t freqspace = 8 * veclen;
	const size_t rarespace = 1;

	const uint32_t *stopFreq = freq + lenFreq - freqspace;
	const uint32_t *stopRare = rare + lenRare - rarespace;
	if (freq > stopFreq) {
		return nate_count_scalar(freq, lenFreq, rare, lenRare);
	}
	uint32_t maxFreq = freq[7 * veclen + vecmax];
	vec M0, M1, M2, M3;

	while (maxFreq < *rare) { // advance freq to a possible match
		freq += veclen * 8; // NOTE: loop below requires this
		if (freq > stopFreq)
			goto FINISH_SCALAR;
		maxFreq = freq[veclen * 7 + vecmax];
	}
	M0 = _mm_load_si128((vec *) freq + 0);
	M1 = _mm_load_si128((vec *) freq + 1);
	M2 = _mm_load_si128((vec *) freq + 2);
	M3 = _mm_load_si128((vec *) freq + 3);
	for (; rare < stopRare; ++rare) {
		const uint32_t matchRare = *rare;//nextRare;
		const vec Match = _mm_set1_epi32(matchRare);
		if (maxFreq < matchRare) { // if no match possible
			freq += veclen * 8; // advance 8 vectors
			if (freq > stopFreq)
				goto FINISH_SCALAR;
			maxFreq = freq[veclen * 7 + vecmax];
			while (maxFreq < matchRare) { // if still no match possible
				freq += veclen * 8; // advance another 8 vectors
				if (freq > stopFreq)
					goto FINISH_SCALAR;
				maxFreq = freq[veclen * 7 + vecmax];
			}
			M0 = _mm_load_si128((vec *) freq + 0);
			M1 = _mm_load_si128((vec *) freq + 1);
			M2 = _mm_load_si128((vec *) freq + 2);
			M3 = _mm_load_si128((vec *) freq + 3);
		}
		const vec Q0 = _mm_or_si128(_mm_cmpeq_epi32(M0, Match),
				_mm_cmpeq_epi32(M1, Match));
		const vec Q1 = _mm_or_si128(_mm_cmpeq_epi32(M2, Match),
				_mm_cmpeq_epi32(M3, Match));
		const vec Q2 = _mm_or_si128(
				_mm_cmpeq_epi32(_mm_load_si128((vec *) freq + 4), Match),
				_mm_cmpeq_epi32(_mm_load_si128((vec *) freq + 5), Match));
		const vec Q3 = _mm_or_si128(
				_mm_cmpeq_epi32(_mm_load_si128((vec *) freq + 6), Match),
				_mm_cmpeq_epi32(_mm_load_si128((vec *) freq + 7), Match));
		const vec S0 = _mm_or_si128(Q0, Q1);
		const vec S1 = _mm_or_si128(Q2, Q3);
		const vec F0 = _mm_or_si128(S0, S1);
		if (_mm_testz_si128(F0, F0)) {
		} else {
			count += 1;
		}
	}

	FINISH_SCALAR: return count + nate_count_scalar(freq,
			stopFreq + FREQSPACE - freq, rare, stopRare + RARESPACE - rare);
}

size_t nate4_count_medium(const uint32_t *rare, const size_t lenRare,
		const uint32_t *freq, const size_t lenFreq) {
	// FUTURE: could swap freq and rare if inverted
	size_t count = 0;
	if (lenFreq == 0 || lenRare == 0)
		return count;

	typedef __m128i vec;
	const uint32_t veclen = sizeof(VEC) / sizeof(uint32_t);
	const size_t vecmax = veclen - 1;
	const size_t freqspace = 4 * veclen;
	const size_t rarespace = 1;

	const uint32_t *stopFreq = freq + lenFreq - freqspace;
	const uint32_t *stopRare = rare + lenRare - rarespace;

	if (freq > stopFreq) {
		return nate_count_scalar(freq, lenFreq, rare, lenRare);
	}
	uint32_t maxFreq = freq[3 * veclen + vecmax];
	vec M0, M1, M2, M3;

	while (maxFreq < *rare) { // advance freq to a possible match
		freq += veclen * 4; // NOTE: loop below requires this
		if (freq > stopFreq)
			goto FINISH_SCALAR;
		maxFreq = freq[veclen * 3 + vecmax];
	}
	M0 = _mm_load_si128((vec *) freq + 0);
	M1 = _mm_load_si128((vec *) freq + 1);
	M2 = _mm_load_si128((vec *) freq + 2);
	M3 = _mm_load_si128((vec *) freq + 3);

	for (; rare < stopRare; ++rare) {
		const uint32_t matchRare = *rare;//nextRare;
		const vec Match = _mm_set1_epi32(matchRare);
		if (maxFreq < matchRare) { // if no match possible
			freq += veclen * 4; // advance 8 vectors
			if (freq > stopFreq)
				goto FINISH_SCALAR;
			maxFreq = freq[veclen * 3 + vecmax];
			while (maxFreq < matchRare) { // if still no match possible
				freq += veclen * 4; // advance another 8 vectors
				if (freq > stopFreq)
					goto FINISH_SCALAR;
				maxFreq = freq[veclen * 3 + vecmax];
			}
			M0 = _mm_load_si128((vec *) freq + 0);
			M1 = _mm_load_si128((vec *) freq + 1);
			M2 = _mm_load_si128((vec *) freq + 2);
			M3 = _mm_load_si128((vec *) freq + 3);
		}
		const vec Q0 = _mm_or_si128(_mm_cmpeq_epi32(M0, Match),
				_mm_cmpeq_epi32(M1, Match));
		const vec Q1 = _mm_or_si128(_mm_cmpeq_epi32(M2, Match),
				_mm_cmpeq_epi32(M3, Match));
		const vec F0 = _mm_or_si128(Q0, Q1);
		if (_mm_testz_si128(F0, F0)) {
		} else {
			count += 1;
		}
	}

	FINISH_SCALAR: return count + nate_count_scalar(freq,
			stopFreq + FREQSPACE - freq, rare, stopRare + RARESPACE - rare);
}

size_t nate5_count_medium(const uint32_t *rare, const size_t lenRare,
		const uint32_t *freq, const size_t lenFreq) {
	// FUTURE: could swap freq and rare if inverted
	size_t count = 0;
	if (lenFreq == 0 || lenRare == 0)
		return count;

	typedef __m128i vec;
	const uint32_t veclen = sizeof(VEC) / sizeof(uint32_t);
	const size_t vecmax = veclen - 1;
	const size_t freqspace = 2 * veclen;
	const size_t rarespace = 1;

	const uint32_t *stopFreq = freq + lenFreq - freqspace;
	const uint32_t *stopRare = rare + lenRare - rarespace;

	if (freq > stopFreq) {
		return nate_count_scalar(freq, lenFreq, rare, lenRare);
	}
	uint32_t maxFreq = freq[ veclen + vecmax];
	vec M0, M1;

	while (maxFreq < *rare) { // advance freq to a possible match
		freq += veclen * 2; // NOTE: loop below requires this
		if (freq > stopFreq)
			goto FINISH_SCALAR;
		maxFreq = freq[veclen  + vecmax];
	}
	M0 = _mm_load_si128((vec *) freq + 0);
	M1 = _mm_load_si128((vec *) freq + 1);

	for (; rare < stopRare; ++rare) {
		const uint32_t matchRare = *rare;//nextRare;
		const vec Match = _mm_set1_epi32(matchRare);
		if (maxFreq < matchRare) { // if no match possible
			freq += veclen * 2; // advance 8 vectors
			if (freq > stopFreq)
				goto FINISH_SCALAR;
			maxFreq = freq[veclen  + vecmax];
			while (maxFreq < matchRare) { // if still no match possible
				freq += veclen * 2; // advance another 8 vectors
				if (freq > stopFreq)
					goto FINISH_SCALAR;
				maxFreq = freq[veclen  + vecmax];
			}
			M0 = _mm_load_si128((vec *) freq + 0);
			M1 = _mm_load_si128((vec *) freq + 1);
		}
		const vec F0 = _mm_or_si128(_mm_cmpeq_epi32(M0, Match),
				_mm_cmpeq_epi32(M1, Match));
		if (_mm_testz_si128(F0, F0)) {
		} else {
			count += 1;
		}
	}

	FINISH_SCALAR: return count + nate_count_scalar(freq,
			stopFreq + FREQSPACE - freq, rare, stopRare + RARESPACE - rare);
}


size_t danielshybridintersectioncardinality(const uint32_t * set1,
		const size_t length1, const uint32_t * set2, const size_t length2) {
	if ((10 * length1 <= length2) or (10 * length2 <= length1)) {
		if ((200 * length1 < length2) or (200 * length2 < length1)) {
			if(length1 < length2)
				return onesidedgallopingintersectioncardinality(set1, length1, set2, length2);
			else
				return onesidedgallopingintersectioncardinality(set2, length2, set1, length1);
		} else {
			if (length1 < length2)
				return nate3_count_medium(set1, length1, set2, length2);
			else
				return nate3_count_medium(set2, length2, set1, length1);
		}
	}
	return widevector2_cardinality_intersect(set1, length1, set2, length2);
}

#endif /* INTERSECTION_H_ */
