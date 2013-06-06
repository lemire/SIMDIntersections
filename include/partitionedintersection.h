/**
 * Schemes inspired or lifted from
 * From Schlegel et al., Fast Sorted-Set Intersection using SIMD Instructions
 */

#ifndef PARTITIONEDINTERSECTION_H_
#define PARTITIONEDINTERSECTION_H_

namespace partitioned {

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
 * From Schlegel et al., Fast Sorted-Set Intersection using SIMD Instructions
 *
 * Optimized by D. Lemire on May 3rd 2013
 */
static size_t faster2_cardinality_intersect_vector16(const uint16_t *A,
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
#endif /* PARTITIONEDINTERSECTION_H_ */
