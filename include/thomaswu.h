

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


typedef long (*flaggedintersectionfindfunction)(int *foundp, UINT4 goal, const UINT4 *target, long ntargets);

long
Intersection_find_v3_cmpeq (int *foundp, UINT4 goal, const UINT4 *target, long ntargets);

long
Intersection_truefind_v3_cmpeq_scalar (int *foundp, UINT4 goal, const UINT4 *target, long ntargets);


long
Intersection_truefind_v3_cmpeq_simd32 (int *foundp, UINT4 goal, const UINT4 *target, long ntargets);


long
Intersection_truefind_v3_cmpeq_simd8 (int *foundp, UINT4 goal, const UINT4 *target, long ntargets) ;

long
Intersection_truefind_v3_cmpeq_binary (int *foundp, UINT4 goal, const UINT4 *target, long ntargets) ;

template <flaggedintersectionfindfunction FINDFUNCTION>
size_t
compute_intersection_flagged (const uint32_t * rare,
        const size_t nrare, const uint32_t * freq, const size_t nfreq, uint32_t * out) {
  UINT4 *init_out;
  size_t i;

  init_out = out;

  size_t lenFreq = nfreq;
  long pos;
  int foundp;
  for (i = 0; i < nrare; i++) {
      pos = FINDFUNCTION(&foundp,rare[i],freq,lenFreq);
      if (foundp == 1) {
        *out++ = rare[i];
      }
      freq += pos;
      lenFreq -= pos;
    }
  return (out - init_out);
}


#endif /* THOMASWU_H_ */
