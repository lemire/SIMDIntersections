

#ifndef THOMASWU_H_
#define THOMASWU_H_

#include "common.h"
#include "intersection.h"

typedef uint32_t UINT4;

typedef uint64_t UINT8;

typedef long (*intersectionfindfunction)(UINT4 goal, const UINT4 *target, long ntargets);

template <intersectionfindfunction FINDFUNCTION>
size_t
compute_intersection (const uint32_t * rare,
        const size_t nrare, const uint32_t * freq, const size_t nfreq, uint32_t * out) {
  UINT4 goal;
  const UINT4 *stop_rare;
  UINT4 *init_out;
  long j;
  long nfreqleft = static_cast<int>(nfreq);// possibly unsafe if nfreq exceeds the range of longs

  init_out = out;
  stop_rare = &(rare[nrare]);
  while (rare < stop_rare) {
    goal = *rare++;

    /* Need some heuristic (? based on nfreq/nrare) for deciding
       between find_v1, find_v3, and find_simdgallop */
    j = FINDFUNCTION(goal,freq,nfreqleft);

    if (j >= nfreqleft) {
      return (out - init_out);
    } else if (freq[j] == goal) {
      *out++ = goal;
    }
    freq += j;
    nfreqleft -= j;
  }
  return (out - init_out);
}


long
Intersection_find_scalar (UINT4 goal, const UINT4 *target, long ntargets);
long
Intersection_find_gallop (UINT4 goal, const UINT4 *target, long ntargets);
long
Intersection_find_v1 (UINT4 goal, const UINT4 *target, long ntargets);
long
Intersection_find_v1_aligned (UINT4 goal, const UINT4 *target, long ntargets);
long
Intersection_find_v1_plow (UINT4 goal, const UINT4 *target, long ntargets);
long
Intersection_find_v2 (UINT4 goal, const UINT4 *target, long ntargets);
long
Intersection_find_v2_aligned (UINT4 goal, const UINT4 *target, long ntargets);
long
Intersection_find_v3 (UINT4 goal, const UINT4 *target, long ntargets);
long
Intersection_find_v3_aligned (UINT4 goal, const UINT4 *target, long ntargets);
long
Intersection_find_simdgallop_v0 (UINT4 goal, const UINT4 *target, long ntargets);
long
Intersection_find_simdgallop_v1 (UINT4 goal, const UINT4 *target, long ntargets);
long
Intersection_find_simdgallop_v2 (UINT4 goal, const UINT4 *target, long ntargets);
long
Intersection_find_simdgallop_v3 (UINT4 goal, const UINT4 *target, long ntargets);




#endif /* THOMASWU_H_ */
