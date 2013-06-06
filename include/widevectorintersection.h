
#ifndef WIDEVECTORINTERSECTION_H_
#define WIDEVECTORINTERSECTION_H_

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
size_t widevector_cardinality_intersect(const uint32_t *A, const size_t s_a,
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
            if ((((A[i_a] - 1) ^ B[i_b + BlockSize - 1]) >> 16 == 0)
                    and (((B[i_b] - 1) ^ A[i_a + BlockSize - 1]) >> 16 == 0)) {


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
 * Variation by Leonid Boytsov. Very clever way of computing
 * the cardinality, but it is unclear how you would actually
 * compute the intersection fast.
 */
size_t leowidevector_cardinality_intersect(const uint32_t *A, const size_t s_a,
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

            if (A[i_a + BlockSize - 1] < B[i_b] + 65536 and
                B[i_b + BlockSize - 1] < A[i_a] + 65536 ) {
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
                //}
            } else {
                count += __simd4by4(v_a, v_b);
                count += __simd4by4(v_aa, v_b);
                count += __simd4by4(v_a, v_bb);
                count += __simd4by4(v_aa, v_bb);
            }

            const uint32_t a_max = A[i_a + BlockSize - 1];
            if (a_max <= B[i_b + BlockSize - 1]) {
                i_a += BlockSize;
                if (i_a >= st_a)
                    goto end;
                v_a = _mm_load_si128((__m128i *) &A[i_a]);
                v_aa = _mm_load_si128((__m128i *) &A[i_a + HalfBlockSize]);
            }
            if (a_max >= B[i_b + BlockSize - 1]) {
                i_b += BlockSize;
                if (i_b >= st_b)
                    goto end;
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


#endif /* WIDEVECTORINTERSECTION_H_ */
