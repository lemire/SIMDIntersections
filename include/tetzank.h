// imported from https://github.com/tetzank/SIMDSetOperations/
#ifndef INCLUDE_TETZANK_H_
#define INCLUDE_TETZANK_H_

#if defined(_MSC_VER)
#define ALIGNED(x) __declspec(align(x))
#else
#if defined(__GNUC__)
#define ALIGNED(x) __attribute__ ((aligned(x)))
#endif
#endif
static uint32_t shuffle_mask_avx[] ALIGNED(0x1000) = {
		7, 6, 5, 4, 3, 2, 1, 0,
		0, 7, 6, 5, 4, 3, 2, 1,
		1, 7, 6, 5, 4, 3, 2, 0,
		0, 1, 7, 6, 5, 4, 3, 2,
		2, 7, 6, 5, 4, 3, 1, 0,
		0, 2, 7, 6, 5, 4, 3, 1,
		1, 2, 7, 6, 5, 4, 3, 0,
		0, 1, 2, 7, 6, 5, 4, 3,
		3, 7, 6, 5, 4, 2, 1, 0,
		0, 3, 7, 6, 5, 4, 2, 1,
		1, 3, 7, 6, 5, 4, 2, 0,
		0, 1, 3, 7, 6, 5, 4, 2,
		2, 3, 7, 6, 5, 4, 1, 0,
		0, 2, 3, 7, 6, 5, 4, 1,
		1, 2, 3, 7, 6, 5, 4, 0,
		0, 1, 2, 3, 7, 6, 5, 4,
		4, 7, 6, 5, 3, 2, 1, 0,
		0, 4, 7, 6, 5, 3, 2, 1,
		1, 4, 7, 6, 5, 3, 2, 0,
		0, 1, 4, 7, 6, 5, 3, 2,
		2, 4, 7, 6, 5, 3, 1, 0,
		0, 2, 4, 7, 6, 5, 3, 1,
		1, 2, 4, 7, 6, 5, 3, 0,
		0, 1, 2, 4, 7, 6, 5, 3,
		3, 4, 7, 6, 5, 2, 1, 0,
		0, 3, 4, 7, 6, 5, 2, 1,
		1, 3, 4, 7, 6, 5, 2, 0,
		0, 1, 3, 4, 7, 6, 5, 2,
		2, 3, 4, 7, 6, 5, 1, 0,
		0, 2, 3, 4, 7, 6, 5, 1,
		1, 2, 3, 4, 7, 6, 5, 0,
		0, 1, 2, 3, 4, 7, 6, 5,
		5, 7, 6, 4, 3, 2, 1, 0,
		0, 5, 7, 6, 4, 3, 2, 1,
		1, 5, 7, 6, 4, 3, 2, 0,
		0, 1, 5, 7, 6, 4, 3, 2,
		2, 5, 7, 6, 4, 3, 1, 0,
		0, 2, 5, 7, 6, 4, 3, 1,
		1, 2, 5, 7, 6, 4, 3, 0,
		0, 1, 2, 5, 7, 6, 4, 3,
		3, 5, 7, 6, 4, 2, 1, 0,
		0, 3, 5, 7, 6, 4, 2, 1,
		1, 3, 5, 7, 6, 4, 2, 0,
		0, 1, 3, 5, 7, 6, 4, 2,
		2, 3, 5, 7, 6, 4, 1, 0,
		0, 2, 3, 5, 7, 6, 4, 1,
		1, 2, 3, 5, 7, 6, 4, 0,
		0, 1, 2, 3, 5, 7, 6, 4,
		4, 5, 7, 6, 3, 2, 1, 0,
		0, 4, 5, 7, 6, 3, 2, 1,
		1, 4, 5, 7, 6, 3, 2, 0,
		0, 1, 4, 5, 7, 6, 3, 2,
		2, 4, 5, 7, 6, 3, 1, 0,
		0, 2, 4, 5, 7, 6, 3, 1,
		1, 2, 4, 5, 7, 6, 3, 0,
		0, 1, 2, 4, 5, 7, 6, 3,
		3, 4, 5, 7, 6, 2, 1, 0,
		0, 3, 4, 5, 7, 6, 2, 1,
		1, 3, 4, 5, 7, 6, 2, 0,
		0, 1, 3, 4, 5, 7, 6, 2,
		2, 3, 4, 5, 7, 6, 1, 0,
		0, 2, 3, 4, 5, 7, 6, 1,
		1, 2, 3, 4, 5, 7, 6, 0,
		0, 1, 2, 3, 4, 5, 7, 6,
		6, 7, 5, 4, 3, 2, 1, 0,
		0, 6, 7, 5, 4, 3, 2, 1,
		1, 6, 7, 5, 4, 3, 2, 0,
		0, 1, 6, 7, 5, 4, 3, 2,
		2, 6, 7, 5, 4, 3, 1, 0,
		0, 2, 6, 7, 5, 4, 3, 1,
		1, 2, 6, 7, 5, 4, 3, 0,
		0, 1, 2, 6, 7, 5, 4, 3,
		3, 6, 7, 5, 4, 2, 1, 0,
		0, 3, 6, 7, 5, 4, 2, 1,
		1, 3, 6, 7, 5, 4, 2, 0,
		0, 1, 3, 6, 7, 5, 4, 2,
		2, 3, 6, 7, 5, 4, 1, 0,
		0, 2, 3, 6, 7, 5, 4, 1,
		1, 2, 3, 6, 7, 5, 4, 0,
		0, 1, 2, 3, 6, 7, 5, 4,
		4, 6, 7, 5, 3, 2, 1, 0,
		0, 4, 6, 7, 5, 3, 2, 1,
		1, 4, 6, 7, 5, 3, 2, 0,
		0, 1, 4, 6, 7, 5, 3, 2,
		2, 4, 6, 7, 5, 3, 1, 0,
		0, 2, 4, 6, 7, 5, 3, 1,
		1, 2, 4, 6, 7, 5, 3, 0,
		0, 1, 2, 4, 6, 7, 5, 3,
		3, 4, 6, 7, 5, 2, 1, 0,
		0, 3, 4, 6, 7, 5, 2, 1,
		1, 3, 4, 6, 7, 5, 2, 0,
		0, 1, 3, 4, 6, 7, 5, 2,
		2, 3, 4, 6, 7, 5, 1, 0,
		0, 2, 3, 4, 6, 7, 5, 1,
		1, 2, 3, 4, 6, 7, 5, 0,
		0, 1, 2, 3, 4, 6, 7, 5,
		5, 6, 7, 4, 3, 2, 1, 0,
		0, 5, 6, 7, 4, 3, 2, 1,
		1, 5, 6, 7, 4, 3, 2, 0,
		0, 1, 5, 6, 7, 4, 3, 2,
		2, 5, 6, 7, 4, 3, 1, 0,
		0, 2, 5, 6, 7, 4, 3, 1,
		1, 2, 5, 6, 7, 4, 3, 0,
		0, 1, 2, 5, 6, 7, 4, 3,
		3, 5, 6, 7, 4, 2, 1, 0,
		0, 3, 5, 6, 7, 4, 2, 1,
		1, 3, 5, 6, 7, 4, 2, 0,
		0, 1, 3, 5, 6, 7, 4, 2,
		2, 3, 5, 6, 7, 4, 1, 0,
		0, 2, 3, 5, 6, 7, 4, 1,
		1, 2, 3, 5, 6, 7, 4, 0,
		0, 1, 2, 3, 5, 6, 7, 4,
		4, 5, 6, 7, 3, 2, 1, 0,
		0, 4, 5, 6, 7, 3, 2, 1,
		1, 4, 5, 6, 7, 3, 2, 0,
		0, 1, 4, 5, 6, 7, 3, 2,
		2, 4, 5, 6, 7, 3, 1, 0,
		0, 2, 4, 5, 6, 7, 3, 1,
		1, 2, 4, 5, 6, 7, 3, 0,
		0, 1, 2, 4, 5, 6, 7, 3,
		3, 4, 5, 6, 7, 2, 1, 0,
		0, 3, 4, 5, 6, 7, 2, 1,
		1, 3, 4, 5, 6, 7, 2, 0,
		0, 1, 3, 4, 5, 6, 7, 2,
		2, 3, 4, 5, 6, 7, 1, 0,
		0, 2, 3, 4, 5, 6, 7, 1,
		1, 2, 3, 4, 5, 6, 7, 0,
		0, 1, 2, 3, 4, 5, 6, 7,
		7, 6, 5, 4, 3, 2, 1, 0,
		0, 7, 6, 5, 4, 3, 2, 1,
		1, 7, 6, 5, 4, 3, 2, 0,
		0, 1, 7, 6, 5, 4, 3, 2,
		2, 7, 6, 5, 4, 3, 1, 0,
		0, 2, 7, 6, 5, 4, 3, 1,
		1, 2, 7, 6, 5, 4, 3, 0,
		0, 1, 2, 7, 6, 5, 4, 3,
		3, 7, 6, 5, 4, 2, 1, 0,
		0, 3, 7, 6, 5, 4, 2, 1,
		1, 3, 7, 6, 5, 4, 2, 0,
		0, 1, 3, 7, 6, 5, 4, 2,
		2, 3, 7, 6, 5, 4, 1, 0,
		0, 2, 3, 7, 6, 5, 4, 1,
		1, 2, 3, 7, 6, 5, 4, 0,
		0, 1, 2, 3, 7, 6, 5, 4,
		4, 7, 6, 5, 3, 2, 1, 0,
		0, 4, 7, 6, 5, 3, 2, 1,
		1, 4, 7, 6, 5, 3, 2, 0,
		0, 1, 4, 7, 6, 5, 3, 2,
		2, 4, 7, 6, 5, 3, 1, 0,
		0, 2, 4, 7, 6, 5, 3, 1,
		1, 2, 4, 7, 6, 5, 3, 0,
		0, 1, 2, 4, 7, 6, 5, 3,
		3, 4, 7, 6, 5, 2, 1, 0,
		0, 3, 4, 7, 6, 5, 2, 1,
		1, 3, 4, 7, 6, 5, 2, 0,
		0, 1, 3, 4, 7, 6, 5, 2,
		2, 3, 4, 7, 6, 5, 1, 0,
		0, 2, 3, 4, 7, 6, 5, 1,
		1, 2, 3, 4, 7, 6, 5, 0,
		0, 1, 2, 3, 4, 7, 6, 5,
		5, 7, 6, 4, 3, 2, 1, 0,
		0, 5, 7, 6, 4, 3, 2, 1,
		1, 5, 7, 6, 4, 3, 2, 0,
		0, 1, 5, 7, 6, 4, 3, 2,
		2, 5, 7, 6, 4, 3, 1, 0,
		0, 2, 5, 7, 6, 4, 3, 1,
		1, 2, 5, 7, 6, 4, 3, 0,
		0, 1, 2, 5, 7, 6, 4, 3,
		3, 5, 7, 6, 4, 2, 1, 0,
		0, 3, 5, 7, 6, 4, 2, 1,
		1, 3, 5, 7, 6, 4, 2, 0,
		0, 1, 3, 5, 7, 6, 4, 2,
		2, 3, 5, 7, 6, 4, 1, 0,
		0, 2, 3, 5, 7, 6, 4, 1,
		1, 2, 3, 5, 7, 6, 4, 0,
		0, 1, 2, 3, 5, 7, 6, 4,
		4, 5, 7, 6, 3, 2, 1, 0,
		0, 4, 5, 7, 6, 3, 2, 1,
		1, 4, 5, 7, 6, 3, 2, 0,
		0, 1, 4, 5, 7, 6, 3, 2,
		2, 4, 5, 7, 6, 3, 1, 0,
		0, 2, 4, 5, 7, 6, 3, 1,
		1, 2, 4, 5, 7, 6, 3, 0,
		0, 1, 2, 4, 5, 7, 6, 3,
		3, 4, 5, 7, 6, 2, 1, 0,
		0, 3, 4, 5, 7, 6, 2, 1,
		1, 3, 4, 5, 7, 6, 2, 0,
		0, 1, 3, 4, 5, 7, 6, 2,
		2, 3, 4, 5, 7, 6, 1, 0,
		0, 2, 3, 4, 5, 7, 6, 1,
		1, 2, 3, 4, 5, 7, 6, 0,
		0, 1, 2, 3, 4, 5, 7, 6,
		6, 7, 5, 4, 3, 2, 1, 0,
		0, 6, 7, 5, 4, 3, 2, 1,
		1, 6, 7, 5, 4, 3, 2, 0,
		0, 1, 6, 7, 5, 4, 3, 2,
		2, 6, 7, 5, 4, 3, 1, 0,
		0, 2, 6, 7, 5, 4, 3, 1,
		1, 2, 6, 7, 5, 4, 3, 0,
		0, 1, 2, 6, 7, 5, 4, 3,
		3, 6, 7, 5, 4, 2, 1, 0,
		0, 3, 6, 7, 5, 4, 2, 1,
		1, 3, 6, 7, 5, 4, 2, 0,
		0, 1, 3, 6, 7, 5, 4, 2,
		2, 3, 6, 7, 5, 4, 1, 0,
		0, 2, 3, 6, 7, 5, 4, 1,
		1, 2, 3, 6, 7, 5, 4, 0,
		0, 1, 2, 3, 6, 7, 5, 4,
		4, 6, 7, 5, 3, 2, 1, 0,
		0, 4, 6, 7, 5, 3, 2, 1,
		1, 4, 6, 7, 5, 3, 2, 0,
		0, 1, 4, 6, 7, 5, 3, 2,
		2, 4, 6, 7, 5, 3, 1, 0,
		0, 2, 4, 6, 7, 5, 3, 1,
		1, 2, 4, 6, 7, 5, 3, 0,
		0, 1, 2, 4, 6, 7, 5, 3,
		3, 4, 6, 7, 5, 2, 1, 0,
		0, 3, 4, 6, 7, 5, 2, 1,
		1, 3, 4, 6, 7, 5, 2, 0,
		0, 1, 3, 4, 6, 7, 5, 2,
		2, 3, 4, 6, 7, 5, 1, 0,
		0, 2, 3, 4, 6, 7, 5, 1,
		1, 2, 3, 4, 6, 7, 5, 0,
		0, 1, 2, 3, 4, 6, 7, 5,
		5, 6, 7, 4, 3, 2, 1, 0,
		0, 5, 6, 7, 4, 3, 2, 1,
		1, 5, 6, 7, 4, 3, 2, 0,
		0, 1, 5, 6, 7, 4, 3, 2,
		2, 5, 6, 7, 4, 3, 1, 0,
		0, 2, 5, 6, 7, 4, 3, 1,
		1, 2, 5, 6, 7, 4, 3, 0,
		0, 1, 2, 5, 6, 7, 4, 3,
		3, 5, 6, 7, 4, 2, 1, 0,
		0, 3, 5, 6, 7, 4, 2, 1,
		1, 3, 5, 6, 7, 4, 2, 0,
		0, 1, 3, 5, 6, 7, 4, 2,
		2, 3, 5, 6, 7, 4, 1, 0,
		0, 2, 3, 5, 6, 7, 4, 1,
		1, 2, 3, 5, 6, 7, 4, 0,
		0, 1, 2, 3, 5, 6, 7, 4,
		4, 5, 6, 7, 3, 2, 1, 0,
		0, 4, 5, 6, 7, 3, 2, 1,
		1, 4, 5, 6, 7, 3, 2, 0,
		0, 1, 4, 5, 6, 7, 3, 2,
		2, 4, 5, 6, 7, 3, 1, 0,
		0, 2, 4, 5, 6, 7, 3, 1,
		1, 2, 4, 5, 6, 7, 3, 0,
		0, 1, 2, 4, 5, 6, 7, 3,
		3, 4, 5, 6, 7, 2, 1, 0,
		0, 3, 4, 5, 6, 7, 2, 1,
		1, 3, 4, 5, 6, 7, 2, 0,
		0, 1, 3, 4, 5, 6, 7, 2,
		2, 3, 4, 5, 6, 7, 1, 0,
		0, 2, 3, 4, 5, 6, 7, 1,
		1, 2, 3, 4, 5, 6, 7, 0,
		0, 1, 2, 3, 4, 5, 6, 7
};

size_t tetzank_intersect_scalar(const uint32_t *list1, size_t size1, const uint32_t *list2, size_t size2, uint32_t *result){
	size_t counter=0;
	const uint32_t *end1 = list1+size1, *end2 = list2+size2;
	while(list1 != end1 && list2 != end2){
		if(*list1 < *list2){
			list1++;
		}else if(*list1 > *list2){
			list2++;
		}else{
			result[counter++] = *list1;
			list1++; list2++;
		}
	}
	return counter;
}

size_t tetzank_intersect_scalar_count(const uint32_t *list1, size_t size1, const uint32_t *list2, size_t size2){
	size_t counter=0;
	const uint32_t *end1 = list1+size1, *end2 = list2+size2;
	while(list1 != end1 && list2 != end2){
		if(*list1 < *list2){
			list1++;
		}else if(*list1 > *list2){
			list2++;
		}else{
			counter++;
			list1++; list2++;
		}
	}
	return counter;
}


#ifdef __AVX2__


#include <immintrin.h>


size_t tetzank_intersect_vector_avx2(const uint32_t *list1, size_t size1, const uint32_t *list2, size_t size2, uint32_t *result){
	size_t count=0, i_a=0, i_b=0;
	size_t st_a = (size1 / 8) * 8;
	size_t st_b = (size2 / 8) * 8;
	while(i_a < st_a && i_b < st_b){
		__m256i v_a = _mm256_loadu_si256((const __m256i*)&list1[i_a]);
		__m256i v_b = _mm256_loadu_si256((const __m256i*)&list2[i_b]);

		int32_t a_max = list1[i_a+7];
		int32_t b_max = list2[i_b+7];
		i_a += (a_max <= b_max) * 8;
		i_b += (a_max >= b_max) * 8;

		constexpr int32_t cyclic_shift = _MM_SHUFFLE(0,3,2,1); //rotating right
		constexpr int32_t cyclic_shift2= _MM_SHUFFLE(2,1,0,3); //rotating left
		constexpr int32_t cyclic_shift3= _MM_SHUFFLE(1,0,3,2); //between
		__m256i cmp_mask1 = _mm256_cmpeq_epi32(v_a, v_b);
		__m256 rot1 = _mm256_permute_ps((__m256)v_b, cyclic_shift);
		__m256i cmp_mask2 = _mm256_cmpeq_epi32(v_a, (__m256i)rot1);
		__m256 rot2 = _mm256_permute_ps((__m256)v_b, cyclic_shift3);
		__m256i cmp_mask3 = _mm256_cmpeq_epi32(v_a, (__m256i)rot2);
		__m256 rot3 = _mm256_permute_ps((__m256)v_b, cyclic_shift2);
		__m256i cmp_mask4 = _mm256_cmpeq_epi32(v_a, (__m256i)rot3);

		__m256 rot4 = _mm256_permute2f128_ps((__m256)v_b, (__m256)v_b, 1);

		__m256i cmp_mask5 = _mm256_cmpeq_epi32(v_a, (__m256i)rot4);
		__m256 rot5 = _mm256_permute_ps(rot4, cyclic_shift);
		__m256i cmp_mask6 = _mm256_cmpeq_epi32(v_a, (__m256i)rot5);
		__m256 rot6 = _mm256_permute_ps(rot4, cyclic_shift3);
		__m256i cmp_mask7 = _mm256_cmpeq_epi32(v_a, (__m256i)rot6);
		__m256 rot7 = _mm256_permute_ps(rot4, cyclic_shift2);
		__m256i cmp_mask8 = _mm256_cmpeq_epi32(v_a, (__m256i)rot7);

		// AVX2: _mm256_or_si256
		__m256i cmp_mask = _mm256_or_si256(
			_mm256_or_si256(
				_mm256_or_si256(cmp_mask1, cmp_mask2),
				_mm256_or_si256(cmp_mask3, cmp_mask4)
			),
			_mm256_or_si256(
				_mm256_or_si256(cmp_mask5, cmp_mask6),
				_mm256_or_si256(cmp_mask7, cmp_mask8)
			)
		);
		int32_t mask = _mm256_movemask_ps((__m256)cmp_mask);

		__m256i idx = _mm256_load_si256((const __m256i*)&shuffle_mask_avx[mask*8]);
		__m256i p = _mm256_permutevar8x32_epi32(v_a, idx);
		_mm256_storeu_si256((__m256i*)&result[count], p);

		count += _mm_popcnt_u32(mask);
	}
	// intersect the tail using scalar intersection
	count += tetzank_intersect_scalar(list1+i_a, size1-i_a, list2+i_b, size2-i_b, result+count);
	return count;
}
size_t tetzank_intersect_vector_avx2_count(const uint32_t *list1, size_t size1, const uint32_t *list2, size_t size2){
	size_t count=0, i_a=0, i_b=0;
	size_t st_a = (size1 / 8) * 8;
	size_t st_b = (size2 / 8) * 8;
	while(i_a < st_a && i_b < st_b){
		__m256i v_a = _mm256_loadu_si256((const __m256i*)&list1[i_a]);
		__m256i v_b = _mm256_loadu_si256((const __m256i*)&list2[i_b]);
		int32_t a_max = list1[i_a+7];
		int32_t b_max = list2[i_b+7];
		i_a += (a_max <= b_max) * 8;
		i_b += (a_max >= b_max) * 8;

		constexpr int32_t cyclic_shift = _MM_SHUFFLE(0,3,2,1); //rotating right
		constexpr int32_t cyclic_shift2= _MM_SHUFFLE(2,1,0,3); //rotating left
		constexpr int32_t cyclic_shift3= _MM_SHUFFLE(1,0,3,2); //between
		// AVX2: _mm256_cmpeq_epi32
		__m256i cmp_mask1 = _mm256_cmpeq_epi32(v_a, v_b);
		__m256 rot1 = _mm256_permute_ps((__m256)v_b, cyclic_shift);
		__m256i cmp_mask2 = _mm256_cmpeq_epi32(v_a, (__m256i)rot1);
		__m256 rot2 = _mm256_permute_ps((__m256)v_b, cyclic_shift3);
		__m256i cmp_mask3 = _mm256_cmpeq_epi32(v_a, (__m256i)rot2);
		__m256 rot3 = _mm256_permute_ps((__m256)v_b, cyclic_shift2);
		__m256i cmp_mask4 = _mm256_cmpeq_epi32(v_a, (__m256i)rot3);

		__m256 rot4 = _mm256_permute2f128_ps((__m256)v_b, (__m256)v_b, 1);

		__m256i cmp_mask5 = _mm256_cmpeq_epi32(v_a, (__m256i)rot4);
		__m256 rot5 = _mm256_permute_ps(rot4, cyclic_shift);
		__m256i cmp_mask6 = _mm256_cmpeq_epi32(v_a, (__m256i)rot5);
		__m256 rot6 = _mm256_permute_ps(rot4, cyclic_shift3);
		__m256i cmp_mask7 = _mm256_cmpeq_epi32(v_a, (__m256i)rot6);
		__m256 rot7 = _mm256_permute_ps(rot4, cyclic_shift2);
		__m256i cmp_mask8 = _mm256_cmpeq_epi32(v_a, (__m256i)rot7);

		// AVX2: _mm256_or_si256
		__m256i cmp_mask = _mm256_or_si256(
			_mm256_or_si256(
				_mm256_or_si256(cmp_mask1, cmp_mask2),
				_mm256_or_si256(cmp_mask3, cmp_mask4)
			),
			_mm256_or_si256(
				_mm256_or_si256(cmp_mask5, cmp_mask6),
				_mm256_or_si256(cmp_mask7, cmp_mask8)
			)
		);
		int32_t mask = _mm256_movemask_ps((__m256)cmp_mask);
		count += _mm_popcnt_u32(mask);
	}
	// intersect the tail using scalar intersection
	count += tetzank_intersect_scalar_count(list1+i_a, size1-i_a, list2+i_b, size2-i_b);

	return count;
}
#endif




#endif /* INCLUDE_TETZANK_H_ */
