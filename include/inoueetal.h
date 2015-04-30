
#ifndef INOUETAL_H_
#define INOUETAL_H_
	const static int popcnt_u32_4bit[16] = { 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4 };

	const static __m128i shuffle_mask[16] = {
		_mm_set_epi8(15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0),
		_mm_set_epi8(15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0),
		_mm_set_epi8(15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 7, 6, 5, 4),
		_mm_set_epi8(15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0),
		_mm_set_epi8(15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 11, 10, 9, 8),
		_mm_set_epi8(15, 14, 13, 12, 11, 10, 9, 8, 11, 10, 9, 8, 3, 2, 1, 0),
		_mm_set_epi8(15, 14, 13, 12, 11, 10, 9, 8, 11, 10, 9, 8, 7, 6, 5, 4),
		_mm_set_epi8(15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0),
		_mm_set_epi8(15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 15, 14, 13, 12),
		_mm_set_epi8(15, 14, 13, 12, 11, 10, 9, 8, 15, 14, 13, 12, 3, 2, 1, 0),
		_mm_set_epi8(15, 14, 13, 12, 11, 10, 9, 8, 15, 14, 13, 12, 7, 6, 5, 4),
		_mm_set_epi8(15, 14, 13, 12, 15, 14, 13, 12, 7, 6, 5, 4, 3, 2, 1, 0),
		_mm_set_epi8(15, 14, 13, 12, 11, 10, 9, 8, 15, 14, 13, 12, 11, 10, 9, 8),
		_mm_set_epi8(15, 14, 13, 12, 15, 14, 13, 12, 11, 10, 9, 8, 3, 2, 1, 0),
		_mm_set_epi8(15, 14, 13, 12, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4),
		_mm_set_epi8(15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0),
	};




	/*
	* Inspired by "Faster Set Intersection with SIMD instructions
	* by Reducing Branch Mispredictions".
	*/
	 size_t SIMDIntersectWithPrefilter(const uint32_t *A, const size_t s_a,
		const uint32_t *B, const size_t s_b, uint32_t * out) {

		assert(out != A);
		assert(out != B);
		size_t i_a = 0, i_b = 0, i_out = 0;

		// trim lengths to be a multiple of 4
		size_t st_a = (s_a / 4) * 4;
		size_t st_b = (s_b / 4) * 4;

		if (i_a < st_a && i_b < st_b) {
			const __m128i a_mask = _mm_set_epi8(12, 12, 12, 12, 8, 8, 8, 8, 4, 4, 4, 4, 0, 0, 0, 0);
			const __m128i b_mask = _mm_set_epi8(12, 8, 4, 0, 12, 8, 4, 0, 12, 8, 4, 0, 12, 8, 4, 0);

			// load initial data in registers.
			__m128i v_a = _mm_loadu_si128((__m128i *) &A[i_a]);
			__m128i v_b = _mm_loadu_si128((__m128i *) &B[i_b]);
			__m128i v_a_filter = _mm_shuffle_epi8(v_a, a_mask);
			__m128i v_b_filter = _mm_shuffle_epi8(v_b, b_mask);
			uint32_t a_max = A[3];
			uint32_t b_max = B[3];

			for(;;) {

				// check for potential intersection of least significant byte.
				__m128i v_c = _mm_cmpeq_epi8(v_a_filter, v_b_filter);

				if (!_mm_movemask_epi8(v_c)) {
					// No hit so load the next 4 lowest bytes from smallest
				advance:
					const uint32_t a_max_local = a_max;
					const uint32_t b_max_local = b_max;
					if (a_max_local <= b_max_local) {
						i_a += 4;
						if (i_a < st_a) {
							v_a = _mm_loadu_si128((__m128i *) &A[i_a]);
							v_a_filter = _mm_shuffle_epi8(v_a, a_mask);
							a_max = _mm_extract_epi32(v_a, 3);
						}
						else {
							break;
						}
					}
					if (a_max_local >= b_max_local) {
						i_b += 4;
						if (i_b < st_b) {
							v_b = _mm_loadu_si128((__m128i *) &B[i_b]);
							v_b_filter = _mm_shuffle_epi8(v_b, b_mask);
							b_max = _mm_extract_epi32(v_b, 3);
						}
						else {
							break;
						}
					}
				} else {

					// TODO: Any way to figure how to do this without having to copy registers?
					// If we can, that would free up more registers when we implement unrolling.
					__m128i v_as = v_a;
					__m128i v_bs = v_b;

					//[ compute mask of common elements
					const uint32_t cyclic_shift = _MM_SHUFFLE(0, 3, 2, 1);
					__m128i cmp_mask1 = _mm_cmpeq_epi32(v_as, v_bs); // pairwise comparison
					v_bs = _mm_shuffle_epi32(v_bs, cyclic_shift); // shuffling
					__m128i cmp_mask2 = _mm_cmpeq_epi32(v_as, v_bs); // again...
					v_bs = _mm_shuffle_epi32(v_bs, cyclic_shift);
					__m128i cmp_mask3 = _mm_cmpeq_epi32(v_as, v_bs); // and again...
					v_bs = _mm_shuffle_epi32(v_bs, cyclic_shift);
					__m128i cmp_mask4 = _mm_cmpeq_epi32(v_as, v_bs); // and again.
					__m128i cmp_mask = _mm_or_si128(_mm_or_si128(cmp_mask1, cmp_mask2),
						_mm_or_si128(cmp_mask3, cmp_mask4)); // OR-ing of comparison masks
					// convert the 128-bit mask to the 4-bit mask
					const int mask = _mm_movemask_ps(_mm_castsi128_ps(cmp_mask));
					//]

					//[ copy out common elements
					const __m128i p = _mm_shuffle_epi8(v_as, shuffle_mask[mask]);
					_mm_storeu_si128((__m128i*)(out + i_out), p);
					i_out += popcnt_u32_4bit[mask]; // a number of elements is a weight of the mask
					//]

					goto advance;
				}
			}
		}


		// intersect the tail using scalar intersection

		while (i_a < s_a && i_b < s_b) {
			const uint32_t a = A[i_a];
			const uint32_t b = B[i_b];
			if (a != b) {
				if (a <= b) {
					i_a++;
				}
				if (a >= b) {
					i_b++;
				}
			} else {
				out[i_out++] = a;
				i_a++;
				i_b++;
			}
		}

		return i_out;
	}

	 size_t lemireSIMDIntersectWithPrefilter(const uint32_t *A, const size_t s_a,
		const uint32_t *B, const size_t s_b, uint32_t * out) {

		assert(out != A);
		assert(out != B);
		size_t i_a = 0, i_b = 0, i_out = 0;

		// trim lengths to be a multiple of 4
		size_t st_a = (s_a / 4) * 4;
		size_t st_b = (s_b / 4) * 4;

		if (i_a < st_a && i_b < st_b) {
			const __m128i a_mask = _mm_set_epi8(12, 12, 12, 12, 8, 8, 8, 8, 4, 4, 4, 4, 0, 0, 0, 0);
			const __m128i b_mask = _mm_set_epi8(12, 8, 4, 0, 12, 8, 4, 0, 12, 8, 4, 0, 12, 8, 4, 0);

			// load initial data in registers.
			__m128i v_a = _mm_loadu_si128((__m128i *) &A[i_a]);
			__m128i v_b = _mm_loadu_si128((__m128i *) &B[i_b]);
			__m128i v_a_filter = _mm_shuffle_epi8(v_a, a_mask);
			__m128i v_b_filter = _mm_shuffle_epi8(v_b, b_mask);
			uint32_t a_max = A[3];
			uint32_t b_max = B[3];

			for(;;) {

				// check for potential intersection of least significant byte.
				__m128i v_c = _mm_cmpeq_epi8(v_a_filter, v_b_filter);

				if (!_mm_movemask_epi8(v_c)) {
					// No hit so load the next 4 lowest bytes from smallest
				advance:
					const uint32_t a_max_local = a_max;
					const uint32_t b_max_local = b_max;
					if (a_max_local <= b_max_local) {
						i_a += 4;
						if (i_a < st_a) {
							v_a = _mm_loadu_si128((__m128i *) &A[i_a]);
							v_a_filter = _mm_shuffle_epi8(v_a, a_mask);
							a_max = _mm_extract_epi32(v_a, 3);
						}
						else {
							break;
						}
					}
					if (a_max_local >= b_max_local) {
						i_b += 4;
						if (i_b < st_b) {
							v_b = _mm_loadu_si128((__m128i *) &B[i_b]);
							v_b_filter = _mm_shuffle_epi8(v_b, b_mask);
							b_max = _mm_extract_epi32(v_b, 3);
						}
						else {
							break;
						}
					}
				} else {
				    const static uint32_t cyclic_shift1 = _MM_SHUFFLE(0, 3, 2, 1);
				    const static uint32_t cyclic_shift2 = _MM_SHUFFLE(1, 0, 3, 2);
				    const static uint32_t cyclic_shift3 = _MM_SHUFFLE(2, 1, 0, 3);
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
					const int mask = _mm_movemask_ps(_mm_castsi128_ps(cmp_mask));
					//]

					//[ copy out common elements
					const __m128i p = _mm_shuffle_epi8(v_a, shuffle_mask[mask]);
					_mm_storeu_si128((__m128i*)(out + i_out), p);
					i_out += _mm_popcnt_u32(mask); // a number of elements is a weight of the mask
					//]

					goto advance;
				}
			}
		}


		// intersect the tail using scalar intersection

		while (i_a < s_a && i_b < s_b) {
			const uint32_t a = A[i_a];
			const uint32_t b = B[i_b];
			if (a != b) {
				if (a <= b) {
					i_a++;
				}
				if (a >= b) {
					i_b++;
				}
			} else {
				out[i_out++] = a;
				i_a++;
				i_b++;
			}
		}

		return i_out;
	}


#endif /* INOUETAL_H_ */
