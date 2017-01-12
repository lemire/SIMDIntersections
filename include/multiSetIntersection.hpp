/*
 * multiSetIntersection.hpp
 *
 *  Created on: 2016/12/20
 *      Author: SparkleXS
 */

#ifndef INCLUDE_MULTISETINTERSECTION_HPP_
#define INCLUDE_MULTISETINTERSECTION_HPP_

#include "intersectionfactory.h"
#include "timer.h"
#include "synthetic.h"
#include "util.h"

namespace msis/*MultiSet InterSection*/{
// here adapts the range [start,end], different from __BSadvanceUntil
// whose range is [start+1,end-1]
static _ALWAYSINLINE size_t binarySearch_wider(const uint32_t * array,
		const size_t start, const size_t end, const size_t min) {
	size_t lower = start;
	size_t upper = end;
	if (lower == end || array[lower] >= min) {
		return lower;
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

// here adapts the range [start,end]
static _ALWAYSINLINE size_t gallopping(const uint32_t * array,
		const size_t start, const size_t end, const size_t min) {
	size_t lower = start;

	// special handling for a possibly common sequential case
	if ((lower >= end) or (array[lower] >= min)) {
		return lower;
	}

	size_t spansize = 1; // could set larger
	// bootstrap an upper limit

	// sxs: here spansize is enlarged to the maximum where its corresponding
	// element is geq min
	while ((lower + spansize <= end) and (array[lower + spansize] < min))
		spansize *= 2;
	size_t upper = (lower + spansize <= end) ? lower + spansize : end;

	// maybe we are lucky (could be common case when the seek ahead expected to be small and sequential will otherwise make us look bad)
	//if (array[upper] == min) {
	//    return upper;
	//}

	if (array[upper] < min) {    // means array has no item >= min
		return end;
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

void small_vs_small(const mySet &sets, std::vector<uint32_t> &out);

void BYintersect_sorted(const uint32_t *D, const size_t &D_end,
		const uint32_t *Q, const size_t &Q_end, uint32_t **out,
		uint32_t &count);

// without swap
void set_vs_set(const mySet &sets, std::vector<uint32_t> &out);

void swapping_set_vs_set(const mySet &sets, std::vector<uint32_t> &out);

void adaptive(const mySet &sets, std::vector<uint32_t> &out);

void sequential(const mySet &sets, std::vector<uint32_t> &out);

void small_adaptive(const mySet &sets, std::vector<uint32_t> &out);

//without resorting
void max(const mySet &sets, std::vector<uint32_t> &out);

void BaezaYates(const mySet &sets, std::vector<uint32_t> &out);

}

#endif /* INCLUDE_MULTISETINTERSECTION_HPP_ */
