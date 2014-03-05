#include "thomaswu.h"

#define HAVE_SSE4_1 1
//#define HAVE_AVX2 1
#define HAVE_64_BIT 1

#define SIMDWIDTH 4     /* sizeof(__m128i)/sizeof(UINT4) */
#define V0_BLOCKSIZE (1*SIMDWIDTH)
#define V1_BLOCKSIZE (2*SIMDWIDTH)
#define V1_ALIGNED_BLOCKSIZE (2*SIMDWIDTH+4)
#define V2_BLOCKSIZE (8*SIMDWIDTH)
#define V2_ALIGNED_BLOCKSIZE (8*SIMDWIDTH+4)
#define V16_BLOCKSIZE (16*SIMDWIDTH)
#define V3_BLOCKSIZE (32*SIMDWIDTH)
#define V3_ALIGNED_BLOCKSIZE (32*SIMDWIDTH+4)


#ifdef DEBUG
#define debug(x) x
#else
#define debug(x)
#endif

#ifdef DEBUG
/* For debugging */
static void
print_vector_hex (__m128i x) {
  UINT4 *s = (UINT4 *) &x;

  printf("%08X %08X %08X %08X\n",s[0],s[1],s[2],s[3]);
  return;
}

static void
print_vector (__m128i x) {
  UINT4 *s = (UINT4 *) &x;

  printf("%u %u %u %u\n",s[0],s[1],s[2],s[3]);
  return;
}
#endif


long
Intersection_find_scalar (UINT4 goal, const UINT4 *target, long ntargets) {
  const UINT4 *end_target, *init_target;

  debug(printf("Calling find_scalar with goal %u against ntargets %d\n",goal,ntargets));
  init_target = target;
  end_target = &(target[ntargets]);
  while (target < end_target && *target < goal) {
    target++;
  }
  return (target - init_target);
}



long
Intersection_find_gallop (UINT4 goal, const UINT4 *target, long ntargets) {
  const UINT4 *end_target;
  long lowi, midi, highi;

  debug(printf("Calling find_gallop with goal %u against ntargets %d\n",goal,ntargets));
  end_target = &(target[ntargets]);

  /* Galloping search */
  highi = 1;
  while (target + highi < end_target && target[highi] < goal) {
    highi <<= 1;            /* gallop by 2 */
  }

  lowi = highi/2;
  if (highi > ntargets) {
    highi = ntargets;
  }

  /* Binary search */
  while (lowi < highi) {
    midi = (lowi + highi)/2;
    if (target[midi] < goal) {
      lowi = midi + 1;
    } else {
      highi = midi;
    }
  }

  return highi;
}



/* The find_v1, find_v3, and find_simdgallop procedures return an
   index j such that targets[j] == goal or is the next higher value.
   The calling procedure therefore needs to check if the goal was
   found by evaluating (j < ntargets && target[j] == goal) */

long
Intersection_find_v1 (UINT4 goal, const UINT4 *target, long ntargets) {
  const UINT4 *end_target, *stop_target, *init_target;
  __m128i Match;
  __m128i F0;
  UINT4 *hits;
  int pos;
  const __m128i conversion =  _mm_set1_epi32(2147483648U); /* 2^31 */

  init_target = target;
  stop_target = &(target[ntargets - V1_BLOCKSIZE]);
  end_target = &(target[ntargets]);

  if (target >= stop_target) {
    return Intersection_find_scalar(goal,target,ntargets);
  } else {
    while (target[V1_BLOCKSIZE] < goal) {
      target += V1_BLOCKSIZE;
      if (target >= stop_target) {
    return (target - init_target) + Intersection_find_scalar(goal,target,/*ntargets*/(end_target - target));
      }
    }

    Match = _mm_set1_epi32(goal - 2147483648U);
    F0 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 0), conversion),Match),32-1),
              _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 1), conversion),Match),28-1));

    debug(printf("F0: "));
    debug(print_vector_hex(F0));

    if (
#ifdef HAVE_SSE4_1
    _mm_testz_si128(F0,F0)
#else
    _mm_movemask_epi8(_mm_cmpeq_epi8(F0,_mm_setzero_si128())) == 0xFFFF
#endif
    ) {
      debug(printf("Not found\n"));
      return (target - init_target);
    } else {
      hits = (UINT4 *) &F0;
#ifdef HAVE_AVX2
      F0 = _mm_sllv_epi32(F0,_mm_set_epi32(0,1,2,3));
      pos = __builtin_clz(hits[0] | hits[1] | hits[2] | hits[3]);
#else
      pos = __builtin_clz(hits[0] | (hits[1] << 1) | (hits[2] << 2) | (hits[3] << 3));
#endif
      debug(printf("pos = %d => %d\n",pos,32 - pos));
      return (target - init_target) + (32 - pos);
    }
  }
}


long
Intersection_find_v1_aligned (UINT4 goal, const UINT4 *target, long ntargets) {
  const UINT4 *end_target, *stop_target, *init_target;
  __m128i Match;
  __m128i F0;
  UINT4 *hits;
  int pos;
  int n_prealign;
  const __m128i conversion =  _mm_set1_epi32(2147483648U); /* 2^31 */


  init_target = target;
  stop_target = &(target[ntargets - V1_ALIGNED_BLOCKSIZE]);
  end_target = &(target[ntargets]);

  if (target >= stop_target) {
    return Intersection_find_scalar(goal,target,ntargets);
  } else {
    while (target[V1_ALIGNED_BLOCKSIZE] < goal) {
      target += V1_ALIGNED_BLOCKSIZE;
      if (target >= stop_target) {
    return (target - init_target) + Intersection_find_scalar(goal,target,/*ntargets*/(end_target - target));
      }
    }

    //conversion = _mm_set1_epi32(2147483648U); /* 2^31 */
    Match = _mm_set1_epi32(goal - 2147483648U);

    /* Mini-scalar before we get to aligned data */
#ifdef HAVE_64_BIT
    n_prealign = (16 - static_cast<int>((UINT8) target & 0xF)) / 4; /* 1, 2, 3, or 4 */
#else
    n_prealign = (16 - static_cast<int>((UINT4) target & 0xF)) / 4;
#endif
    F0 = _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_load_si128((__m128i *) (target) + 0), conversion),Match),32-1);
    if (_mm_testz_si128(F0,F0)) {
      debug(printf("No elements are less than goal, so stop\n"));
      return (target - init_target);
    } else if (_mm_movemask_epi8(_mm_cmpeq_epi8(F0,_mm_set1_epi32(0x1))) != 0xFFFF) {
      debug(printf("Some elements are less than goal, so return\n"));
      hits = (UINT4 *) &F0;
#ifdef HAVE_AVX2
      F0 = _mm_sllv_epi32(F0,_mm_set_epi32(0,1,2,3));
      pos = __builtin_clz(hits[0] | hits[1] | hits[2] | hits[3]);
#else
      pos = __builtin_clz(hits[0] | (hits[1] << 1) | (hits[2] << 2) | (hits[3] << 3));
#endif
      debug(printf("pos = %d => %d\n",pos,32 - pos));
      return (target - init_target) + (32 - pos);
    } else {
      debug(printf("All elements are less than goal, so iterate\n"));
      target += n_prealign;

      F0 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_load_si128((__m128i *) (target) + 0), conversion),Match),32-1),
            _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_load_si128((__m128i *) (target) + 1), conversion),Match),28-1));

      debug(printf("F0: "));
      debug(print_vector_hex(F0));

      if (
#ifdef HAVE_SSE4_1
      _mm_testz_si128(F0,F0)
#else
      _mm_movemask_epi8(_mm_cmpeq_epi8(F0,_mm_setzero_si128())) == 0xFFFF
#endif
      ) {
    debug(printf("Not found\n"));
    return (target - init_target);

      } else {
    hits = (UINT4 *) &F0;
#ifdef HAVE_AVX2
    F0 = _mm_sllv_epi32(F0,_mm_set_epi32(0,1,2,3));
    pos = __builtin_clz(hits[0] | hits[1] | hits[2] | hits[3]);
#else
    pos = __builtin_clz(hits[0] | (hits[1] << 1) | (hits[2] << 2) | (hits[3] << 3));
#endif
    debug(printf("pos = %d => %d\n",pos,32 - pos));
    return (target - init_target) + (32 - pos);
      }
    }
  }
}

long
Intersection_find_v1_plow (UINT4 goal, const UINT4 *target, long ntargets) {
  const UINT4 *end_target, *stop_target, *init_target;
  __m128i Match;
  __m128i F0;
  UINT4 *hits;
  int pos;
  const __m128i conversion =  _mm_set1_epi32(2147483648U); /* 2^31 */

  debug(printf("Entering Intersection_find_v1_plow with goal %u against %d targets\n",goal,ntargets));

  init_target = target;
  stop_target = &(target[ntargets - V1_BLOCKSIZE]);
  end_target = &(target[ntargets]);

  if (target >= stop_target) {
    /* Need to call scalar without subtracting V1_BLOCKSIZE from target */
    return Intersection_find_scalar(goal,target,ntargets);
  } else {
    //conversion = _mm_set1_epi32(2147483648U); /* 2^31 */
    Match = _mm_set1_epi32(goal - 2147483648U);

    while (target < stop_target) {
      F0 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 0), conversion),Match),32-1),
            _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 1), conversion),Match),28-1));


      debug(printf("F0: "));
      debug(print_vector_hex(F0));

      if (
#ifdef HAVE_SSE4_1
      _mm_testz_si128(F0,F0)
#else
      _mm_movemask_epi8(_mm_cmpeq_epi8(F0,_mm_setzero_si128())) == 0xFFFF
#endif
      ) {
    debug(printf("No elements are less than goal, so stop\n"));
    return (target - init_target);

      } else if (_mm_movemask_epi8(_mm_cmpeq_epi8(F0,_mm_set1_epi32(0x1F))) == 0xFFFF) {
    debug(printf("All elements are less than goal, so iterate\n"));
    target += V1_BLOCKSIZE;

      } else {
    /* Some elements in block < goal, so return */
    hits = (UINT4 *) &F0;
#ifdef HAVE_AVX2
    F0 = _mm_sllv_epi32(F0,_mm_set_epi32(0,1,2,3));
    pos = __builtin_clz(hits[0] | hits[1] | hits[2] | hits[3]);
#else
    pos = __builtin_clz(hits[0] | (hits[1] << 1) | (hits[2] << 2) | (hits[3] << 3));
#endif
    debug(printf("pos = %d => %d\n",pos,32 - pos));
    return (target - init_target) + (32 - pos);
      }
    }

    target -= V1_BLOCKSIZE;
    return (target - init_target) + Intersection_find_scalar(goal,target,/*ntargets*/(end_target - target));
  }
}


long
Intersection_find_v2 (UINT4 goal, const UINT4 *target, long ntargets) {
  const UINT4 *end_target, *stop_target, *init_target;
  __m128i Match;
  __m128i F0, Q0, Q1, Q2, Q3;
  UINT4 *hits;
  int pos;
  const __m128i conversion =  _mm_set1_epi32(2147483648U); /* 2^31 */

  init_target = target;
  stop_target = &(target[ntargets - V2_BLOCKSIZE]);
  end_target = &(target[ntargets]);

  if (target >= stop_target) {
    return Intersection_find_scalar(goal,target,ntargets);
  } else {
    while (target[V2_BLOCKSIZE] < goal) {
      target += V2_BLOCKSIZE;
      if (target >= stop_target) {
    return (target - init_target) + Intersection_find_scalar(goal,target,/*ntargets*/(end_target - target));
      }
    }

    //conversion = _mm_set1_epi32(2147483648U); /* 2^31 */
    Match = _mm_set1_epi32(goal - 2147483648U);

    Q0 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 0), conversion),Match),32-1),
              _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 1), conversion),Match),28-1));
    Q1 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 2), conversion),Match),24-1),
              _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 3), conversion),Match),20-1));
    Q2 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 4), conversion),Match),16-1),
              _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 5), conversion),Match),12-1));
    Q3 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 6), conversion),Match),8-1),
              _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 7), conversion),Match),4-1));

#ifdef DEBUG
    printf("Q0: ");
    print_vector_hex(Q0);
    printf("Q1: ");
    print_vector_hex(Q1);
    printf("Q2: ");
    print_vector_hex(Q2);
    printf("Q3: ");
    print_vector_hex(Q3);
#endif

    F0 = _mm_or_si128(_mm_or_si128(Q0, Q1), _mm_or_si128(Q2, Q3));
    debug(printf("F0: "));
    debug(print_vector_hex(F0));

    if (
#ifdef HAVE_SSE4_1
    _mm_testz_si128(F0,F0)
#else
    _mm_movemask_epi8(_mm_cmpeq_epi8(F0,_mm_setzero_si128())) == 0xFFFF
#endif
    ) {
      debug(printf("Not found\n"));
      return (target - init_target);
    } else {
      hits = (UINT4 *) &F0;
#ifdef HAVE_AVX2
      F0 = _mm_sllv_epi32(F0,_mm_set_epi32(0,1,2,3));
      pos = __builtin_clz(hits[0] | hits[1] | hits[2] | hits[3]);
#else
      pos = __builtin_clz(hits[0] | (hits[1] << 1) | (hits[2] << 2) | (hits[3] << 3));
#endif
      debug(printf("pos = %d => %d\n",pos,32 - pos));
      return (target - init_target) + (32 - pos);
    }
  }
}



long
Intersection_find_v2_plow (UINT4 goal, const UINT4 *target, long ntargets) {
  const UINT4 *end_target, *stop_target, *init_target;
  __m128i Match;
  __m128i F0, Q0, Q1, Q2, Q3;
  UINT4 *hits;
  int pos;
  const __m128i conversion =  _mm_set1_epi32(2147483648U); /* 2^31 */

  debug(printf("Entering Intersection_find_v1_plow with goal %u against %d targets\n",goal,ntargets));

  init_target = target;
  stop_target = &(target[ntargets - V2_BLOCKSIZE]);
  end_target = &(target[ntargets]);

  if (target >= stop_target) {
    /* Need to call scalar without subtracting V1_BLOCKSIZE from target */
    return Intersection_find_scalar(goal,target,ntargets);
  } else {
    //conversion = _mm_set1_epi32(2147483648U); /* 2^31 */
    Match = _mm_set1_epi32(goal - 2147483648U);

    while (target < stop_target) {
      Q0 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 0), conversion),Match),32-1),
            _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 1), conversion),Match),28-1));
      Q1 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 2), conversion),Match),24-1),
            _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 3), conversion),Match),20-1));
      Q2 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 4), conversion),Match),16-1),
            _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 5), conversion),Match),12-1));
      Q3 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 6), conversion),Match),8-1),
            _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 7), conversion),Match),4-1));

#ifdef DEBUG
      printf("Q0: ");
      print_vector_hex(Q0);
      printf("Q1: ");
      print_vector_hex(Q1);
      printf("Q2: ");
      print_vector_hex(Q2);
      printf("Q3: ");
      print_vector_hex(Q3);
#endif

      F0 = _mm_or_si128(_mm_or_si128(Q0, Q1), _mm_or_si128(Q2, Q3));
      debug(printf("F0: "));
      debug(print_vector_hex(F0));

      if (
#ifdef HAVE_SSE4_1
      _mm_testz_si128(F0,F0)
#else
      _mm_movemask_epi8(_mm_cmpeq_epi8(F0,_mm_setzero_si128())) == 0xFFFF
#endif
      ) {
    debug(printf("No elements are less than goal, so stop\n"));
    return (target - init_target);

      } else if (_mm_movemask_epi8(_mm_cmpeq_epi8(F0,_mm_set1_epi32(0x1FFFFFFF))) == 0xFFFF) {
    debug(printf("All elements are less than goal, so iterate\n"));
    target += V2_BLOCKSIZE;

      } else {
    /* Some elements in block < goal, so return */
    hits = (UINT4 *) &F0;
#ifdef HAVE_AVX2
    F0 = _mm_sllv_epi32(F0,_mm_set_epi32(0,1,2,3));
    pos = __builtin_clz(hits[0] | hits[1] | hits[2] | hits[3]);
#else
    pos = __builtin_clz(hits[0] | (hits[1] << 1) | (hits[2] << 2) | (hits[3] << 3));
#endif
    debug(printf("pos = %d => %d\n",pos,32 - pos));
    return (target - init_target) + (32 - pos);
      }
    }

    target -= V2_BLOCKSIZE;
    return (target - init_target) + Intersection_find_scalar(goal,target,/*ntargets*/(end_target - target));
  }
}


long
Intersection_find_v2_aligned (UINT4 goal, const UINT4 *target, long ntargets) {
  const UINT4 *end_target, *stop_target, *init_target;
  __m128i Match;
  __m128i F0, Q0, Q1, Q2, Q3;
  UINT4 *hits;
  int pos;
  int n_prealign;
  const __m128i conversion =  _mm_set1_epi32(2147483648U); /* 2^31 */


  init_target = target;
  stop_target = &(target[ntargets - V2_ALIGNED_BLOCKSIZE]);
  end_target = &(target[ntargets]);

  if (target >= stop_target) {
    return Intersection_find_scalar(goal,target,ntargets);
  } else {
    while (target[V2_BLOCKSIZE] < goal) {
      target += V2_BLOCKSIZE;
      if (target >= stop_target) {
    return (target - init_target) + Intersection_find_scalar(goal,target,/*ntargets*/(end_target - target));
      }
    }

    /* Mini-scalar before we get to aligned data */
#ifdef HAVE_64_BIT
    n_prealign = ((16 - ((UINT8) target & 0xF)) / 4) & 0x3;
#else
    n_prealign = ((16 - ((UINT4) target & 0xF)) / 4) & 0x3;
#endif

    debug(printf("target is at location %p.  Need %d to get to 128-bit boundary\n",target,n_prealign));
    while (--n_prealign >= 0 && *target < goal) {
      target++;
    }
    if (*target >= goal) {
      return (target - init_target);
    }

    //conversion = _mm_set1_epi32(2147483648U); /* 2^31 */
    Match = _mm_set1_epi32(goal - 2147483648U);

    Q0 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_load_si128((__m128i *) (target) + 0), conversion),Match),32-1),
              _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_load_si128((__m128i *) (target) + 1), conversion),Match),28-1));
    Q1 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_load_si128((__m128i *) (target) + 2), conversion),Match),24-1),
              _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_load_si128((__m128i *) (target) + 3), conversion),Match),20-1));
    Q2 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_load_si128((__m128i *) (target) + 4), conversion),Match),16-1),
              _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_load_si128((__m128i *) (target) + 5), conversion),Match),12-1));
    Q3 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_load_si128((__m128i *) (target) + 6), conversion),Match),8-1),
              _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_load_si128((__m128i *) (target) + 7), conversion),Match),4-1));

#ifdef DEBUG
    printf("Q0: ");
    print_vector_hex(Q0);
    printf("Q1: ");
    print_vector_hex(Q1);
    printf("Q2: ");
    print_vector_hex(Q2);
    printf("Q3: ");
    print_vector_hex(Q3);
#endif

    F0 = _mm_or_si128(_mm_or_si128(Q0, Q1), _mm_or_si128(Q2, Q3));
    debug(printf("F0: "));
    debug(print_vector_hex(F0));

    if (
#ifdef HAVE_SSE4_1
    _mm_testz_si128(F0,F0)
#else
    _mm_movemask_epi8(_mm_cmpeq_epi8(F0,_mm_setzero_si128())) == 0xFFFF
#endif
    ) {
      debug(printf("Not found\n"));
      return (target - init_target);
    } else {
      hits = (UINT4 *) &F0;
#ifdef HAVE_AVX2
      F0 = _mm_sllv_epi32(F0,_mm_set_epi32(0,1,2,3));
      pos = __builtin_clz(hits[0] | hits[1] | hits[2] | hits[3]);
#else
      pos = __builtin_clz(hits[0] | (hits[1] << 1) | (hits[2] << 2) | (hits[3] << 3));
#endif
      debug(printf("pos = %d => %d\n",pos,32 - pos));
      return (target - init_target) + (32 - pos);
    }
  }
}


long
Intersection_find_v16 (UINT4 goal, const UINT4 *target, long ntargets) {
  const UINT4 *end_target, *stop_target, *init_target;
  __m128i Match;
  __m128i F0, Q0, Q1, Q2, Q3;
  UINT4 *hits;
  int pos;
  int base;
  const __m128i conversion =  _mm_set1_epi32(2147483648U); /* 2^31 */

  init_target = target;
  stop_target = &(target[ntargets - V16_BLOCKSIZE]);
  end_target = &(target[ntargets]);

  if (target >= stop_target) {
    return Intersection_find_scalar(goal,target,ntargets);
  } else {
    while (target[V16_BLOCKSIZE] < goal) {
      target += V16_BLOCKSIZE;
      if (target >= stop_target) {
    return (target - init_target) + Intersection_find_scalar(goal,target,/*ntargets*/(end_target - target));
      }
    }

    //conversion = _mm_set1_epi32(2147483648U); /* 2^31 */
    Match = _mm_set1_epi32(goal - 2147483648U);

    if (target[SIMDWIDTH * 8] >= goal) {
      base = 0;
      Q0 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 0), conversion),Match),32-1),
            _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 1), conversion),Match),28-1));
      Q1 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 2), conversion),Match),24-1),
            _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 3), conversion),Match),20-1));
      Q2 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 4), conversion),Match),16-1),
            _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 5), conversion),Match),12-1));
      Q3 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 6), conversion),Match),8-1),
            _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 7), conversion),Match),4-1));
    } else {
      base = 32;
      Q0 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 8), conversion),Match),32-1),
            _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 9), conversion),Match),28-1));
      Q1 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 10), conversion),Match),24-1),
            _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 11), conversion),Match),20-1));
      Q2 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 12), conversion),Match),16-1),
            _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 13), conversion),Match),12-1));
      Q3 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 14), conversion),Match),8-1),
            _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 15), conversion),Match),4-1));
    }

#ifdef DEBUG
    printf("Q0: ");
    print_vector_hex(Q0);
    printf("Q1: ");
    print_vector_hex(Q1);
    printf("Q2: ");
    print_vector_hex(Q2);
    printf("Q3: ");
    print_vector_hex(Q3);
#endif

    F0 = _mm_or_si128(_mm_or_si128(Q0, Q1), _mm_or_si128(Q2, Q3));
    debug(printf("F0: "));
    debug(print_vector_hex(F0));

    if (
#ifdef HAVE_SSE4_1
    _mm_testz_si128(F0,F0)
#else
    _mm_movemask_epi8(_mm_cmpeq_epi8(F0,_mm_setzero_si128())) == 0xFFFF
#endif
    ) {
      debug(printf("Not found\n"));
      return (target - init_target) + base;
    } else {
      hits = (UINT4 *) &F0;
#ifdef HAVE_AVX2
      F0 = _mm_sllv_epi32(F0,_mm_set_epi32(0,1,2,3));
      pos = __builtin_clz(hits[0] | hits[1] | hits[2] | hits[3]);
#else
      pos = __builtin_clz(hits[0] | (hits[1] << 1) | (hits[2] << 2) | (hits[3] << 3));
#endif
      debug(printf("base = %d, pos = %d => %d\n",base,pos,32 - pos));
      return (target - init_target) + base + (32 - pos);
    }
  }
}



long
Intersection_find_v3 (UINT4 goal, const UINT4 *target, long ntargets) {
  const UINT4 *end_target, *stop_target, *init_target;
  __m128i Match;
  __m128i F0, Q0, Q1, Q2, Q3;
  UINT4 *hits;
  int pos, base;
  const __m128i conversion =  _mm_set1_epi32(2147483648U); /* 2^31 */

  init_target = target;
  stop_target = &(target[ntargets - V3_BLOCKSIZE]);
  end_target = &(target[ntargets]);

  if (target >= stop_target) {
    return Intersection_find_scalar(goal,target,ntargets);
  } else {
    while (target[V3_BLOCKSIZE] < goal) {
      debug(printf("target[V3_BLOCKSIZE] %u < goal %u, so advancing by V3_BLOCKSIZE\n",target[V3_BLOCKSIZE],goal));
      target += V3_BLOCKSIZE;
      if (target >= stop_target) {
    return (target - init_target) + Intersection_find_scalar(goal,target,/*ntargets*/(end_target - target));
      }
    }

    //conversion = _mm_set1_epi32(2147483648U); /* 2^31 */
    Match = _mm_set1_epi32(goal - 2147483648U);

    if (target[SIMDWIDTH * 16] >= goal) {
      if (target[SIMDWIDTH * 8] >= goal) {
    base = 0;
    Q2 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 4), conversion),Match),16-1),
              _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 5), conversion),Match),12-1));
    Q3 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 6), conversion),Match),8-1),
              _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 7), conversion),Match),4-1));
    Q0 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 0), conversion),Match),32-1),
              _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 1), conversion),Match),28-1));
    Q1 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 2), conversion),Match),24-1),
              _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 3), conversion),Match),20-1));
      } else {
    base = 32;
    Q0 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 8), conversion),Match),32-1),
              _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 9), conversion),Match),28-1));
    Q1 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 10), conversion),Match),24-1),
              _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 11), conversion),Match),20-1));
    Q2 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 12), conversion),Match),16-1),
              _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 13), conversion),Match),12-1));
    Q3 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 14), conversion),Match),8-1),
              _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 15), conversion),Match),4-1));
      }
    } else {
      if (target[SIMDWIDTH * 24] >= goal) {
    base = 64;
    Q2 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 4 + 16), conversion),Match),16-1),
              _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 5 + 16), conversion),Match),12-1));
    Q3 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 6 + 16), conversion),Match),8-1),
              _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 7 + 16), conversion),Match),4-1));
    Q0 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 0 + 16), conversion),Match),32-1),
              _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 1 + 16), conversion),Match),28-1));
    Q1 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 2 + 16), conversion),Match),24-1),
              _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 3 + 16), conversion),Match),20-1));
      } else {
    base = 96;
    Q0 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 8 + 16), conversion),Match),32-1),
              _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 9 + 16), conversion),Match),28-1));
    Q1 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 10 + 16), conversion),Match),24-1),
              _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 11 + 16), conversion),Match),20-1));
    Q2 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 12 + 16), conversion),Match),16-1),
              _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 13 + 16), conversion),Match),12-1));
    Q3 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 14 + 16), conversion),Match),8-1),
              _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 15 + 16), conversion),Match),4-1));
      }
    }

#ifdef DEBUG
    printf("Q0: ");
    print_vector_hex(Q0);
    printf("Q1: ");
    print_vector_hex(Q1);
    printf("Q2: ");
    print_vector_hex(Q2);
    printf("Q3: ");
    print_vector_hex(Q3);
#endif

    F0 = _mm_or_si128(_mm_or_si128(Q0, Q1), _mm_or_si128(Q2, Q3));
    debug(printf("F0: "));
    debug(print_vector_hex(F0));

    if (
#ifdef HAVE_SSE4_1
    _mm_testz_si128(F0,F0)
#else
    _mm_movemask_epi8(_mm_cmpeq_epi8(F0,_mm_setzero_si128())) == 0xFFFF
#endif
    ) {
      debug(printf("Not found\n"));
      return (target - init_target) + base;
    } else {
      hits = (UINT4 *) &F0;
#ifdef HAVE_AVX2
      F0 = _mm_sllv_epi32(F0,_mm_set_epi32(0,1,2,3));
      pos = __builtin_clz(hits[0] | hits[1] | hits[2] | hits[3]);
#else
      pos = __builtin_clz(hits[0] | (hits[1] << 1) | (hits[2] << 2) | (hits[3] << 3));
#endif
      debug(printf("base = %d, pos = %d => %d\n",base,pos,32 - pos));
      return (target - init_target) + base + (32 - pos);
    }
  }
}


long
Intersection_find_v3_linear (UINT4 goal, const UINT4 *target, long ntargets) {
  const UINT4 *end_target, *stop_target, *init_target;
  __m128i Match;
  __m128i F0, Q0, Q1, Q2, Q3;
  UINT4 *hits;
  int pos, base;
  const __m128i conversion =  _mm_set1_epi32(2147483648U); /* 2^31 */

  init_target = target;
  stop_target = &(target[ntargets - V3_BLOCKSIZE]);
  end_target = &(target[ntargets]);

  if (target >= stop_target) {
    return Intersection_find_scalar(goal,target,ntargets);
  } else {
    while (target[V3_BLOCKSIZE] < goal) {
      debug(printf("target[V3_BLOCKSIZE] %u < goal %u, so advancing by V3_BLOCKSIZE\n",target[V3_BLOCKSIZE],goal));
      target += V3_BLOCKSIZE;
      if (target >= stop_target) {
    return (target - init_target) + Intersection_find_scalar(goal,target,/*ntargets*/(end_target - target));
      }
    }

    //conversion = _mm_set1_epi32(2147483648U); /* 2^31 */
    Match = _mm_set1_epi32(goal - 2147483648U);

    if (target[SIMDWIDTH * 8] >= goal) {
      base = 0;
      Q0 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 0), conversion),Match),32-1),
            _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 1), conversion),Match),28-1));
      Q1 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 2), conversion),Match),24-1),
            _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 3), conversion),Match),20-1));
      Q2 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 4), conversion),Match),16-1),
            _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 5), conversion),Match),12-1));
      Q3 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 6), conversion),Match),8-1),
            _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 7), conversion),Match),4-1));
    } else if (target[SIMDWIDTH * 16] >= goal) {
      base = 32;
      Q0 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 8), conversion),Match),32-1),
            _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 9), conversion),Match),28-1));
      Q1 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 10), conversion),Match),24-1),
            _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 11), conversion),Match),20-1));
      Q2 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 12), conversion),Match),16-1),
            _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 13), conversion),Match),12-1));
      Q3 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 14), conversion),Match),8-1),
            _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 15), conversion),Match),4-1));
    } else if (target[SIMDWIDTH * 24] >= goal) {
      base = 64;
      Q0 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 0 + 16), conversion),Match),32-1),
            _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 1 + 16), conversion),Match),28-1));
      Q1 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 2 + 16), conversion),Match),24-1),
            _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 3 + 16), conversion),Match),20-1));
      Q2 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 4 + 16), conversion),Match),16-1),
            _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 5 + 16), conversion),Match),12-1));
      Q3 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 6 + 16), conversion),Match),8-1),
            _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 7 + 16), conversion),Match),4-1));
    } else {
      base = 96;
      Q0 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 8 + 16), conversion),Match),32-1),
            _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 9 + 16), conversion),Match),28-1));
      Q1 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 10 + 16), conversion),Match),24-1),
            _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 11 + 16), conversion),Match),20-1));
      Q2 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 12 + 16), conversion),Match),16-1),
            _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 13 + 16), conversion),Match),12-1));
      Q3 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 14 + 16), conversion),Match),8-1),
            _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 15 + 16), conversion),Match),4-1));
    }

#ifdef DEBUG
    printf("Q0: ");
    print_vector_hex(Q0);
    printf("Q1: ");
    print_vector_hex(Q1);
    printf("Q2: ");
    print_vector_hex(Q2);
    printf("Q3: ");
    print_vector_hex(Q3);
#endif

    F0 = _mm_or_si128(_mm_or_si128(Q0, Q1), _mm_or_si128(Q2, Q3));
    debug(printf("F0: "));
    debug(print_vector_hex(F0));

    if (
#ifdef HAVE_SSE4_1
    _mm_testz_si128(F0,F0)
#else
    _mm_movemask_epi8(_mm_cmpeq_epi8(F0,_mm_setzero_si128())) == 0xFFFF
#endif
    ) {
      debug(printf("Not found\n"));
      return (target - init_target) + base;
    } else {
      hits = (UINT4 *) &F0;
#ifdef HAVE_AVX2
      F0 = _mm_sllv_epi32(F0,_mm_set_epi32(0,1,2,3));
      pos = __builtin_clz(hits[0] | hits[1] | hits[2] | hits[3]);
#else
      pos = __builtin_clz(hits[0] | (hits[1] << 1) | (hits[2] << 2) | (hits[3] << 3));
#endif
      debug(printf("base = %d, pos = %d => %d\n",base,pos,32 - pos));
      return (target - init_target) + base + (32 - pos);
    }
  }
}


long
Intersection_find_v3_aligned (UINT4 goal, const UINT4 *target, long ntargets) {
  const UINT4 *end_target, *stop_target, *init_target;
  __m128i Match;
  __m128i F0, Q0, Q1, Q2, Q3;
  UINT4 *hits;
  int pos, base;
  int n_prealign;
  const __m128i conversion =  _mm_set1_epi32(2147483648U); /* 2^31 */

  init_target = target;
  stop_target = &(target[ntargets - V3_BLOCKSIZE - 3]);
  end_target = &(target[ntargets]);

  if (target >= stop_target) {
    return Intersection_find_scalar(goal,target,ntargets);
  } else {
    while (target[V3_BLOCKSIZE] < goal) {
      debug(printf("target[V3_BLOCKSIZE] %u < goal %u, so advancing by V3_BLOCKSIZE\n",target[V3_BLOCKSIZE],goal));
      target += V3_BLOCKSIZE;
      if (target >= stop_target) {
    return (target - init_target) + Intersection_find_scalar(goal,target,/*ntargets*/(end_target - target));
      }
    }

    /* Mini-scalar before we get to aligned data */
#ifdef HAVE_64_BIT
    n_prealign = ((16 - ((UINT8) target & 0xF)) / 4) & 0x3;
#else
    n_prealign = ((16 - ((UINT4) target & 0xF)) / 4) & 0x3;
#endif

    debug(printf("target is at location %p.  Need %d to get to 128-bit boundary\n",target,n_prealign));
    while (--n_prealign >= 0 && *target < goal) {
      target++;
    }
    if (*target >= goal) {
      return (target - init_target);
    }

    //conversion = _mm_set1_epi32(2147483648U); /* 2^31 */
    Match = _mm_set1_epi32(goal - 2147483648U);

    if (target[SIMDWIDTH * 16] >= goal) {
      if (target[SIMDWIDTH * 8] >= goal) {
    base = 0;
    Q2 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_load_si128((__m128i *) (target) + 4), conversion),Match),16-1),
              _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_load_si128((__m128i *) (target) + 5), conversion),Match),12-1));
    Q3 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_load_si128((__m128i *) (target) + 6), conversion),Match),8-1),
              _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_load_si128((__m128i *) (target) + 7), conversion),Match),4-1));
    Q0 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_load_si128((__m128i *) (target) + 0), conversion),Match),32-1),
              _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_load_si128((__m128i *) (target) + 1), conversion),Match),28-1));
    Q1 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_load_si128((__m128i *) (target) + 2), conversion),Match),24-1),
              _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_load_si128((__m128i *) (target) + 3), conversion),Match),20-1));
      } else {
    base = 32;
    Q0 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_load_si128((__m128i *) (target) + 8), conversion),Match),32-1),
              _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_load_si128((__m128i *) (target) + 9), conversion),Match),28-1));
    Q1 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_load_si128((__m128i *) (target) + 10), conversion),Match),24-1),
              _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_load_si128((__m128i *) (target) + 11), conversion),Match),20-1));
    Q2 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_load_si128((__m128i *) (target) + 12), conversion),Match),16-1),
              _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_load_si128((__m128i *) (target) + 13), conversion),Match),12-1));
    Q3 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_load_si128((__m128i *) (target) + 14), conversion),Match),8-1),
              _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_load_si128((__m128i *) (target) + 15), conversion),Match),4-1));
      }
    } else {
      if (target[SIMDWIDTH * 24] >= goal) {
    base = 64;
    Q2 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_load_si128((__m128i *) (target) + 4 + 16), conversion),Match),16-1),
              _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_load_si128((__m128i *) (target) + 5 + 16), conversion),Match),12-1));
    Q3 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_load_si128((__m128i *) (target) + 6 + 16), conversion),Match),8-1),
              _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_load_si128((__m128i *) (target) + 7 + 16), conversion),Match),4-1));
    Q0 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_load_si128((__m128i *) (target) + 0 + 16), conversion),Match),32-1),
              _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_load_si128((__m128i *) (target) + 1 + 16), conversion),Match),28-1));
    Q1 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_load_si128((__m128i *) (target) + 2 + 16), conversion),Match),24-1),
              _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_load_si128((__m128i *) (target) + 3 + 16), conversion),Match),20-1));
      } else {
    base = 96;
    Q0 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_load_si128((__m128i *) (target) + 8 + 16), conversion),Match),32-1),
              _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_load_si128((__m128i *) (target) + 9 + 16), conversion),Match),28-1));
    Q1 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_load_si128((__m128i *) (target) + 10 + 16), conversion),Match),24-1),
              _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_load_si128((__m128i *) (target) + 11 + 16), conversion),Match),20-1));
    Q2 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_load_si128((__m128i *) (target) + 12 + 16), conversion),Match),16-1),
              _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_load_si128((__m128i *) (target) + 13 + 16), conversion),Match),12-1));
    Q3 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_load_si128((__m128i *) (target) + 14 + 16), conversion),Match),8-1),
              _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_load_si128((__m128i *) (target) + 15 + 16), conversion),Match),4-1));
      }
    }

#ifdef DEBUG
    printf("Q0: ");
    print_vector_hex(Q0);
    printf("Q1: ");
    print_vector_hex(Q1);
    printf("Q2: ");
    print_vector_hex(Q2);
    printf("Q3: ");
    print_vector_hex(Q3);
#endif

    F0 = _mm_or_si128(_mm_or_si128(Q0, Q1), _mm_or_si128(Q2, Q3));
    debug(printf("F0: "));
    debug(print_vector_hex(F0));

    if (
#ifdef HAVE_SSE4_1
    _mm_testz_si128(F0,F0)
#else
    _mm_movemask_epi8(_mm_cmpeq_epi8(F0,_mm_setzero_si128())) == 0xFFFF
#endif
    ) {
      debug(printf("Not found\n"));
      return (target - init_target) + base;
    } else {
      hits = (UINT4 *) &F0;
#ifdef HAVE_AVX2
      F0 = _mm_sllv_epi32(F0,_mm_set_epi32(0,1,2,3));
      pos = __builtin_clz(hits[0] | hits[1] | hits[2] | hits[3]);
#else
      pos = __builtin_clz(hits[0] | (hits[1] << 1) | (hits[2] << 2) | (hits[3] << 3));
#endif
      debug(printf("base = %d, pos = %d => %d\n",base,pos,32 - pos));
      return (target - init_target) + base + (32 - pos);
    }
  }
}




long
Intersection_find_simdgallop_v0 (UINT4 goal, const UINT4 *target, long ntargets) {
  const UINT4 *end_target, *stop_target, *init_target;
  __m128i Match;
  __m128i F0;
  UINT4 *hits;
  int pos, base;
  const __m128i conversion =  _mm_set1_epi32(2147483648U); /* 2^31 */

  int low_offset, mid_offset, high_offset, j;


  init_target = target;
  stop_target = &(target[ntargets - V0_BLOCKSIZE]);
  end_target = &(target[ntargets]);

  if (target >= stop_target) {
    return Intersection_find_scalar(goal,target,ntargets);

  } else {
    /* Galloping search */
    debug(printf("Starting galloping search\n"));
    high_offset = 1;
    while (target + (j = V0_BLOCKSIZE * high_offset + V0_BLOCKSIZE) < stop_target &&
       target[j] < goal) {
      debug(printf("Comparing target[%d] = %u with goal %u\n",j,target[j],goal));
      high_offset <<= 1;    /* gallop by 2 */
    }
    debug(printf("Galloping finds offset to be %d\n",high_offset));

    /* Binary search to nearest offset */
    low_offset = high_offset/2;
    if (target + V0_BLOCKSIZE * high_offset + V0_BLOCKSIZE >= stop_target) {
      high_offset = (stop_target - target)/V0_BLOCKSIZE; /* Do not subtract 1 */
    }

    debug(printf("Starting binary search\n"));
    debug(printf("low offset %d, high_offset %d\n",low_offset,high_offset));
    while (low_offset < high_offset) {
      mid_offset = (low_offset + high_offset)/2;
      if (target[V0_BLOCKSIZE * mid_offset + V0_BLOCKSIZE] < goal) {
    low_offset = mid_offset + 1;
      } else {
    high_offset = mid_offset;
      }
      debug(printf("low offset %d, high_offset %d\n",low_offset,high_offset));
    }

    if (target + V0_BLOCKSIZE * high_offset + V0_BLOCKSIZE >= stop_target) {
      return Intersection_find_scalar(goal,target,/*ntargets*/(end_target - target));
    } else {
      target += V0_BLOCKSIZE * high_offset;

      /* SIMD search of block */
      //conversion = _mm_set1_epi32(2147483648U); /* 2^31 */
      Match = _mm_set1_epi32(goal - 2147483648U);
      F0 = _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 0), conversion),Match),32-1);

      debug(printf("F0: "));
      debug(print_vector_hex(F0));

      if (
#ifdef HAVE_SSE4_1
      _mm_testz_si128(F0,F0)
#else
      _mm_movemask_epi8(_mm_cmpeq_epi8(F0,_mm_setzero_si128())) == 0xFFFF
#endif
      ) {
    debug(printf("Not found\n"));
    return (target - init_target);
      } else {
    hits = (UINT4 *) &F0;
#ifdef HAVE_AVX2
    F0 = _mm_sllv_epi32(F0,_mm_set_epi32(0,1,2,3));
    pos = __builtin_clz(hits[0] | hits[1] | hits[2] | hits[3]);
#else
    pos = __builtin_clz(hits[0] | (hits[1] << 1) | (hits[2] << 2) | (hits[3] << 3));
#endif
    debug(printf("pos = %d => %d\n",pos,32 - pos));
    return (target - init_target) + (32 - pos);
      }
    }
  }
}


long
Intersection_find_simdgallop_v1 (UINT4 goal, const UINT4 *target, long ntargets) {
  const UINT4 *end_target, *stop_target, *init_target;
  __m128i Match;
  __m128i F0;
  UINT4 *hits;
  int pos, base;
  const __m128i conversion =  _mm_set1_epi32(2147483648U); /* 2^31 */

  int low_offset, mid_offset, high_offset, j;


  init_target = target;
  stop_target = &(target[ntargets - V1_BLOCKSIZE]);
  end_target = &(target[ntargets]);

  if (target >= stop_target) {
    return Intersection_find_scalar(goal,target,ntargets);

  } else {
    /* Galloping search */
    debug(printf("Starting galloping search\n"));
    high_offset = 1;
    while (target + (j = V1_BLOCKSIZE * high_offset + V1_BLOCKSIZE) < stop_target &&
       target[j] < goal) {
      debug(printf("Comparing target[%d] = %u with goal %u\n",j,target[j],goal));
      high_offset <<= 1;    /* gallop by 2 */
    }
    debug(printf("Galloping finds offset to be %d\n",high_offset));

    /* Binary search to nearest offset */
    low_offset = high_offset/2;
    if (target + V1_BLOCKSIZE * high_offset + V1_BLOCKSIZE >= stop_target) {
      high_offset = (stop_target - target)/V1_BLOCKSIZE; /* Do not subtract 1 */
    }

    debug(printf("Starting binary search\n"));
    debug(printf("low offset %d, high_offset %d\n",low_offset,high_offset));
    while (low_offset < high_offset) {
      mid_offset = (low_offset + high_offset)/2;
      if (target[V1_BLOCKSIZE * mid_offset + V1_BLOCKSIZE] < goal) {
    low_offset = mid_offset + 1;
      } else {
    high_offset = mid_offset;
      }
      debug(printf("low offset %d, high_offset %d\n",low_offset,high_offset));
    }

    if (target + V1_BLOCKSIZE * high_offset + V1_BLOCKSIZE >= stop_target) {
      return Intersection_find_scalar(goal,target,/*ntargets*/(end_target - target));
    } else {
      target += V1_BLOCKSIZE * high_offset;

      /* SIMD search of block */
      //conversion = _mm_set1_epi32(2147483648U); /* 2^31 */
      Match = _mm_set1_epi32(goal - 2147483648U);
      F0 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 0), conversion),Match),32-1),
            _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 1), conversion),Match),28-1));

      debug(printf("F0: "));
      debug(print_vector_hex(F0));

      if (
#ifdef HAVE_SSE4_1
      _mm_testz_si128(F0,F0)
#else
      _mm_movemask_epi8(_mm_cmpeq_epi8(F0,_mm_setzero_si128())) == 0xFFFF
#endif
      ) {
    debug(printf("Not found\n"));
    return (target - init_target);
      } else {
    hits = (UINT4 *) &F0;
#ifdef HAVE_AVX2
    F0 = _mm_sllv_epi32(F0,_mm_set_epi32(0,1,2,3));
    pos = __builtin_clz(hits[0] | hits[1] | hits[2] | hits[3]);
#else
    pos = __builtin_clz(hits[0] | (hits[1] << 1) | (hits[2] << 2) | (hits[3] << 3));
#endif
    debug(printf("pos = %d => %d\n",pos,32 - pos));
    return (target - init_target) + (32 - pos);
      }
    }
  }
}



long
Intersection_find_simdgallop_v2 (UINT4 goal, const UINT4 *target, long ntargets) {
  const UINT4 *end_target, *stop_target, *init_target;
  __m128i Match;
  __m128i F0, Q0, Q1, Q2, Q3;
  UINT4 *hits;
  int pos, base;
  const __m128i conversion =  _mm_set1_epi32(2147483648U); /* 2^31 */

  int low_offset, mid_offset, high_offset, j;


  init_target = target;
  stop_target = &(target[ntargets - V2_BLOCKSIZE]);
  end_target = &(target[ntargets]);

  if (target >= stop_target) {
    return Intersection_find_scalar(goal,target,ntargets);

  } else {
    /* Galloping search */
    debug(printf("Starting galloping search\n"));
    high_offset = 1;
    while (target + (j = V2_BLOCKSIZE * high_offset + V2_BLOCKSIZE) < stop_target &&
       target[j] < goal) {
      debug(printf("Comparing target[%d] = %u with goal %u\n",j,target[j],goal));
      high_offset <<= 1;    /* gallop by 2 */
    }
    debug(printf("Galloping finds offset to be %d\n",high_offset));

    /* Binary search to nearest offset */
    low_offset = high_offset/2;
    if (target + V2_BLOCKSIZE * high_offset + V2_BLOCKSIZE >= stop_target) {
      high_offset = (stop_target - target)/V2_BLOCKSIZE; /* Do not subtract 1 */
    }

    debug(printf("Starting binary search\n"));
    debug(printf("low offset %d, high_offset %d\n",low_offset,high_offset));
    while (low_offset < high_offset) {
      mid_offset = (low_offset + high_offset)/2;
      if (target[V2_BLOCKSIZE * mid_offset + V2_BLOCKSIZE] < goal) {
    low_offset = mid_offset + 1;
      } else {
    high_offset = mid_offset;
      }
      debug(printf("low offset %d, high_offset %d\n",low_offset,high_offset));
    }

    if (target + V2_BLOCKSIZE * high_offset + V2_BLOCKSIZE >= stop_target) {
      return Intersection_find_scalar(goal,target,/*ntargets*/(end_target - target));
    } else {
      target += V2_BLOCKSIZE * high_offset;

      /* SIMD search of block */
      //conversion = _mm_set1_epi32(2147483648U); /* 2^31 */
      Match = _mm_set1_epi32(goal - 2147483648U);

      Q0 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 0), conversion),Match),32-1),
            _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 1), conversion),Match),28-1));
      Q1 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 2), conversion),Match),24-1),
            _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 3), conversion),Match),20-1));
      Q2 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 4), conversion),Match),16-1),
            _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 5), conversion),Match),12-1));
      Q3 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 6), conversion),Match),8-1),
            _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 7), conversion),Match),4-1));

#ifdef DEBUG
      printf("Q0: ");
      print_vector_hex(Q0);
      printf("Q1: ");
      print_vector_hex(Q1);
      printf("Q2: ");
      print_vector_hex(Q2);
      printf("Q3: ");
      print_vector_hex(Q3);
#endif

      F0 = _mm_or_si128(_mm_or_si128(Q0, Q1), _mm_or_si128(Q2, Q3));
      debug(printf("F0: "));
      debug(print_vector_hex(F0));

      if (
#ifdef HAVE_SSE4_1
      _mm_testz_si128(F0,F0)
#else
      _mm_movemask_epi8(_mm_cmpeq_epi8(F0,_mm_setzero_si128())) == 0xFFFF
#endif
      ) {
    debug(printf("Not found\n"));
    return (target - init_target);
      } else {
    hits = (UINT4 *) &F0;
#ifdef HAVE_AVX2
    F0 = _mm_sllv_epi32(F0,_mm_set_epi32(0,1,2,3));
    pos = __builtin_clz(hits[0] | hits[1] | hits[2] | hits[3]);
#else
    pos = __builtin_clz(hits[0] | (hits[1] << 1) | (hits[2] << 2) | (hits[3] << 3));
#endif
    debug(printf("pos = %d => %d\n",pos,32 - pos));
    return (target - init_target) + (32 - pos);
      }
    }
  }
}


long
Intersection_find_simdgallop_v3 (UINT4 goal, const UINT4 *target, long ntargets) {
  const UINT4 *end_target, *stop_target, *init_target;
  __m128i Match;
  __m128i F0, Q0, Q1, Q2, Q3;
  UINT4 *hits;
  int pos, base;
  const __m128i conversion =  _mm_set1_epi32(2147483648U); /* 2^31 */

  int low_offset, mid_offset, high_offset, j;


  init_target = target;
  stop_target = &(target[ntargets - V3_BLOCKSIZE]);
  end_target = &(target[ntargets]);

  if (target >= stop_target) {
    return Intersection_find_scalar(goal,target,ntargets);

  } else {
    /* Galloping search */
    debug(printf("Starting galloping search\n"));
    high_offset = 1;
    while (target + (j = V3_BLOCKSIZE * high_offset + V3_BLOCKSIZE) < stop_target &&
       target[j] < goal) {
      debug(printf("Comparing target[%d] = %u with goal %u\n",j,target[j],goal));
      high_offset <<= 1;    /* gallop by 2 */
    }
    debug(printf("Galloping finds offset to be %d\n",high_offset));

    /* Binary search to nearest offset */
    low_offset = high_offset/2;
    if (target + V3_BLOCKSIZE * high_offset + V3_BLOCKSIZE >= stop_target) {
      high_offset = (stop_target - target)/V3_BLOCKSIZE; /* Do not subtract 1 */
    }

    debug(printf("Starting binary search\n"));
    debug(printf("low offset %d, high_offset %d\n",low_offset,high_offset));
    while (low_offset < high_offset) {
      mid_offset = (low_offset + high_offset)/2;
      if (target[V3_BLOCKSIZE * mid_offset + V3_BLOCKSIZE] < goal) {
    low_offset = mid_offset + 1;
      } else {
    high_offset = mid_offset;
      }
      debug(printf("low offset %d, high_offset %d\n",low_offset,high_offset));
    }

    if (target + V3_BLOCKSIZE * high_offset + V3_BLOCKSIZE >= stop_target) {
      return Intersection_find_scalar(goal,target,/*ntargets*/(end_target - target));
    } else {
      target += V3_BLOCKSIZE * high_offset;

      /* SIMD search of block */
      //conversion = _mm_set1_epi32(2147483648U); /* 2^31 */
      Match = _mm_set1_epi32(goal - 2147483648U);

      if (target[SIMDWIDTH * 16] >= goal) {
    if (target[SIMDWIDTH * 8] >= goal) {
      base = 0;
      Q2 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 4), conversion),Match),16-1),
                _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 5), conversion),Match),12-1));
      Q3 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 6), conversion),Match),8-1),
                _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 7), conversion),Match),4-1));
      Q0 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 0), conversion),Match),32-1),
                _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 1), conversion),Match),28-1));
      Q1 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 2), conversion),Match),24-1),
                _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 3), conversion),Match),20-1));
    } else {
      base = 32;
      Q0 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 8), conversion),Match),32-1),
                _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 9), conversion),Match),28-1));
      Q1 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 10), conversion),Match),24-1),
                _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 11), conversion),Match),20-1));
      Q2 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 12), conversion),Match),16-1),
                _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 13), conversion),Match),12-1));
      Q3 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 14), conversion),Match),8-1),
                _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 15), conversion),Match),4-1));
    }
      } else {
    if (target[SIMDWIDTH * 24] >= goal) {
      base = 64;
      Q2 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 4 + 16), conversion),Match),16-1),
                _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 5 + 16), conversion),Match),12-1));
      Q3 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 6 + 16), conversion),Match),8-1),
                _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 7 + 16), conversion),Match),4-1));
      Q0 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 0 + 16), conversion),Match),32-1),
                _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 1 + 16), conversion),Match),28-1));
      Q1 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 2 + 16), conversion),Match),24-1),
                _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 3 + 16), conversion),Match),20-1));
    } else {
      base = 96;
      Q0 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 8 + 16), conversion),Match),32-1),
                _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 9 + 16), conversion),Match),28-1));
      Q1 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 10 + 16), conversion),Match),24-1),
                _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 11 + 16), conversion),Match),20-1));
      Q2 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 12 + 16), conversion),Match),16-1),
                _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 13 + 16), conversion),Match),12-1));
      Q3 = _mm_or_si128(_mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 14 + 16), conversion),Match),8-1),
                _mm_srli_epi32(_mm_cmplt_epi32(_mm_sub_epi32(_mm_loadu_si128((__m128i *) (target) + 15 + 16), conversion),Match),4-1));
    }
      }

#ifdef DEBUG
      printf("Q0: ");
      print_vector_hex(Q0);
      printf("Q1: ");
      print_vector_hex(Q1);
      printf("Q2: ");
      print_vector_hex(Q2);
      printf("Q3: ");
      print_vector_hex(Q3);
#endif

      F0 = _mm_or_si128(_mm_or_si128(Q0, Q1), _mm_or_si128(Q2, Q3));
      debug(printf("F0: "));
      debug(print_vector_hex(F0));

      if (
#ifdef HAVE_SSE4_1
      _mm_testz_si128(F0,F0)
#else
      _mm_movemask_epi8(_mm_cmpeq_epi8(F0,_mm_setzero_si128())) == 0xFFFF
#endif
      ) {
    debug(printf("Not found\n"));
    return (target - init_target) + base;
      } else {
    hits = (UINT4 *) &F0;
#ifdef HAVE_AVX2
    F0 = _mm_sllv_epi32(F0,_mm_set_epi32(0,1,2,3));
    pos = __builtin_clz(hits[0] | hits[1] | hits[2] | hits[3]);
#else
    pos = __builtin_clz(hits[0] | (hits[1] << 1) | (hits[2] << 2) | (hits[3] << 3));
#endif
    debug(printf("base = %d, pos = %d => %d\n",base,pos,32 - pos));
    return (target - init_target) + base + (32 - pos);
      }
    }
  }
}


