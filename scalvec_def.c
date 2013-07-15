#include "vec4.h"
#include "compiler.h"
#include "scalar.h"
#include "macro.h"

#include <stdint.h>
#include <stddef.h>


#ifdef DEBUG
#include <stdio.h>
#define DEBUG_PRINT(args...) printf(args)
#else
#define DEBUG_PRINT(args...)
#endif

#define DEBUG_ASSERT(args...) 

#ifdef IACA
#include </opt/intel/iaca-lin32/include/iacaMarks.h>
#endif


#define _FUNC_NAME(numfreq, veclen, advance)    \
    match_scalvec_v ## veclen ## _f ## numfreq ## _a ## advance
#define FUNC_NAME(args...) _FUNC_NAME(args)

#define ADVANCE 0

#define NUMFREQ 1
#define SCALVEC_MATCH_ALL()                   
#include "match_scalvec.def"
#undef SCALVEC_MATCH_ALL
#undef NUMFREQ

#define NUMFREQ 2
#define SCALVEC_MATCH_ALL()                     \
    VEC_OR(F0, F1)
#include "match_scalvec.def"
#undef SCALVEC_MATCH_ALL
#undef NUMFREQ

#define NUMFREQ 3
#define SCALVEC_MATCH_ALL()                     \
    VEC_OR(F0, F1);                             \
    VEC_OR(F0, F2)
#include "match_scalvec.def"
#undef SCALVEC_MATCH_ALL
#undef NUMFREQ

#define NUMFREQ 4
#define SCALVEC_MATCH_ALL()                             \
    VEC_OR(F0, F1); VEC_OR(F2, F3);                     \
    VEC_OR(F0, F2);
#include "match_scalvec.def"
#undef SCALVEC_MATCH_ALL
#undef NUMFREQ

#define NUMFREQ 5
#define SCALVEC_MATCH_ALL()                             \
    VEC_OR(F0, F1); VEC_OR(F2, F3);                     \
    VEC_OR(F0, F2);                                     \
    VEC_OR(F0, F4)
#include "match_scalvec.def"
#undef SCALVEC_MATCH_ALL
#undef NUMFREQ

#define NUMFREQ 6
#define SCALVEC_MATCH_ALL()                             \
    VEC_OR(F0, F1); VEC_OR(F2, F3); VEC_OR(F4, F5);     \
    VEC_OR(F0, F2);                                     \
    VEC_OR(F0, F4)
#include "match_scalvec.def"
#undef SCALVEC_MATCH_ALL
#undef NUMFREQ

#define NUMFREQ 7
#define SCALVEC_MATCH_ALL()                                   \
    VEC_OR(F0, F1); VEC_OR(F2, F3); VEC_OR(F4, F5);           \
    VEC_OR(F0, F2); VEC_OR(F4, F6);                           \
    VEC_OR(F0, F4)

#include "match_scalvec.def"
#undef SCALVEC_MATCH_ALL
#undef NUMFREQ

#define NUMFREQ 8
#define SCALVEC_MATCH_ALL()                                             \
    VEC_OR(F0, F1); VEC_OR(F2, F3); VEC_OR(F4, F5); VEC_OR(F6, F7);     \
    VEC_OR(F0, F2); VEC_OR(F4, F6);                                     \
    VEC_OR(F0, F4)
#include "match_scalvec.def"
#undef SCALVEC_MATCH_ALL
#undef NUMFREQ

#undef ADVANCE
#define ADVANCE 1


#define NUMFREQ 2
#define SCALVEC_MATCH_ALL()                     \
    VEC_OR(F0, F1)
#include "match_scalvec.def"
#undef SCALVEC_MATCH_ALL
#undef NUMFREQ

#define NUMFREQ 3
#define SCALVEC_MATCH_ALL()                     \
    VEC_OR(F0, F1);                             \
    VEC_OR(F0, F2)
#include "match_scalvec.def"
#undef SCALVEC_MATCH_ALL
#undef NUMFREQ

#define NUMFREQ 4
#define SCALVEC_MATCH_ALL()                             \
    VEC_OR(F0, F1); VEC_OR(F2, F3);                     \
    VEC_OR(F0, F2);
#include "match_scalvec.def"
#undef SCALVEC_MATCH_ALL
#undef NUMFREQ

#define NUMFREQ 5
#define SCALVEC_MATCH_ALL()                             \
    VEC_OR(F0, F1); VEC_OR(F2, F3);                     \
    VEC_OR(F0, F2);                                     \
    VEC_OR(F0, F4)
#include "match_scalvec.def"
#undef SCALVEC_MATCH_ALL
#undef NUMFREQ

#define NUMFREQ 6
#define SCALVEC_MATCH_ALL()                             \
    VEC_OR(F0, F1); VEC_OR(F2, F3); VEC_OR(F4, F5);     \
    VEC_OR(F0, F2);                                     \
    VEC_OR(F0, F4)
#include "match_scalvec.def"
#undef SCALVEC_MATCH_ALL
#undef NUMFREQ

#define NUMFREQ 7
#define SCALVEC_MATCH_ALL()                                   \
    VEC_OR(F0, F1); VEC_OR(F2, F3); VEC_OR(F4, F5);           \
    VEC_OR(F0, F2); VEC_OR(F4, F6);                           \
    VEC_OR(F0, F4)

#include "match_scalvec.def"
#undef SCALVEC_MATCH_ALL
#undef NUMFREQ

#define NUMFREQ 8
#define SCALVEC_MATCH_ALL()                                             \
    VEC_OR(F0, F1); VEC_OR(F2, F3); VEC_OR(F4, F5); VEC_OR(F6, F7);     \
    VEC_OR(F0, F2); VEC_OR(F4, F6);                                     \
    VEC_OR(F0, F4)
#include "match_scalvec.def"
#undef SCALVEC_MATCH_ALL
#undef NUMFREQ

#undef ADVANCE
#define ADVANCE 2

#define NUMFREQ 3
#define SCALVEC_MATCH_ALL()                     \
    VEC_OR(F0, F1);                             \
    VEC_OR(F0, F2)
#include "match_scalvec.def"
#undef SCALVEC_MATCH_ALL
#undef NUMFREQ

#define NUMFREQ 4
#define SCALVEC_MATCH_ALL()                             \
    VEC_OR(F0, F1); VEC_OR(F2, F3);                     \
    VEC_OR(F0, F2);
#include "match_scalvec.def"
#undef SCALVEC_MATCH_ALL
#undef NUMFREQ

#define NUMFREQ 5
#define SCALVEC_MATCH_ALL()                             \
    VEC_OR(F0, F1); VEC_OR(F2, F3);                     \
    VEC_OR(F0, F2);                                     \
    VEC_OR(F0, F4)
#include "match_scalvec.def"
#undef SCALVEC_MATCH_ALL
#undef NUMFREQ

#define NUMFREQ 6
#define SCALVEC_MATCH_ALL()                             \
    VEC_OR(F0, F1); VEC_OR(F2, F3); VEC_OR(F4, F5);     \
    VEC_OR(F0, F2);                                     \
    VEC_OR(F0, F4)
#include "match_scalvec.def"
#undef SCALVEC_MATCH_ALL
#undef NUMFREQ

#define NUMFREQ 7
#define SCALVEC_MATCH_ALL()                                   \
    VEC_OR(F0, F1); VEC_OR(F2, F3); VEC_OR(F4, F5);           \
    VEC_OR(F0, F2); VEC_OR(F4, F6);                           \
    VEC_OR(F0, F4)

#include "match_scalvec.def"
#undef SCALVEC_MATCH_ALL
#undef NUMFREQ

#define NUMFREQ 8
#define SCALVEC_MATCH_ALL()                                             \
    VEC_OR(F0, F1); VEC_OR(F2, F3); VEC_OR(F4, F5); VEC_OR(F6, F7);     \
    VEC_OR(F0, F2); VEC_OR(F4, F6);                                     \
    VEC_OR(F0, F4)
#include "match_scalvec.def"
#undef SCALVEC_MATCH_ALL
#undef NUMFREQ

#undef ADVANCE
#define ADVANCE 3

#define NUMFREQ 4
#define SCALVEC_MATCH_ALL()                             \
    VEC_OR(F0, F1); VEC_OR(F2, F3);                     \
    VEC_OR(F0, F2);
#include "match_scalvec.def"
#undef SCALVEC_MATCH_ALL
#undef NUMFREQ

#define NUMFREQ 5
#define SCALVEC_MATCH_ALL()                             \
    VEC_OR(F0, F1); VEC_OR(F2, F3);                     \
    VEC_OR(F0, F2);                                     \
    VEC_OR(F0, F4)
#include "match_scalvec.def"
#undef SCALVEC_MATCH_ALL
#undef NUMFREQ

#define NUMFREQ 6
#define SCALVEC_MATCH_ALL()                             \
    VEC_OR(F0, F1); VEC_OR(F2, F3); VEC_OR(F4, F5);     \
    VEC_OR(F0, F2);                                     \
    VEC_OR(F0, F4)
#include "match_scalvec.def"
#undef SCALVEC_MATCH_ALL
#undef NUMFREQ

#define NUMFREQ 7
#define SCALVEC_MATCH_ALL()                                   \
    VEC_OR(F0, F1); VEC_OR(F2, F3); VEC_OR(F4, F5);           \
    VEC_OR(F0, F2); VEC_OR(F4, F6);                           \
    VEC_OR(F0, F4)

#include "match_scalvec.def"
#undef SCALVEC_MATCH_ALL
#undef NUMFREQ

#define NUMFREQ 8
#define SCALVEC_MATCH_ALL()                                             \
    VEC_OR(F0, F1); VEC_OR(F2, F3); VEC_OR(F4, F5); VEC_OR(F6, F7);     \
    VEC_OR(F0, F2); VEC_OR(F4, F6);                                     \
    VEC_OR(F0, F4)
#include "match_scalvec.def"
#undef SCALVEC_MATCH_ALL
#undef NUMFREQ

#undef ADVANCE
#define ADVANCE 4

#define NUMFREQ 5
#define SCALVEC_MATCH_ALL()                             \
    VEC_OR(F0, F1); VEC_OR(F2, F3);                     \
    VEC_OR(F0, F2);                                     \
    VEC_OR(F0, F4)
#include "match_scalvec.def"
#undef SCALVEC_MATCH_ALL
#undef NUMFREQ

#define NUMFREQ 6
#define SCALVEC_MATCH_ALL()                             \
    VEC_OR(F0, F1); VEC_OR(F2, F3); VEC_OR(F4, F5);     \
    VEC_OR(F0, F2);                                     \
    VEC_OR(F0, F4)
#include "match_scalvec.def"
#undef SCALVEC_MATCH_ALL
#undef NUMFREQ

#define NUMFREQ 7
#define SCALVEC_MATCH_ALL()                                   \
    VEC_OR(F0, F1); VEC_OR(F2, F3); VEC_OR(F4, F5);           \
    VEC_OR(F0, F2); VEC_OR(F4, F6);                           \
    VEC_OR(F0, F4)

#include "match_scalvec.def"
#undef SCALVEC_MATCH_ALL
#undef NUMFREQ

#define NUMFREQ 8
#define SCALVEC_MATCH_ALL()                                             \
    VEC_OR(F0, F1); VEC_OR(F2, F3); VEC_OR(F4, F5); VEC_OR(F6, F7);     \
    VEC_OR(F0, F2); VEC_OR(F4, F6);                                     \
    VEC_OR(F0, F4)
#include "match_scalvec.def"
#undef SCALVEC_MATCH_ALL
#undef NUMFREQ


#ifdef TEST

#include "test.h"
#include <stdio.h>

#define _MACRO_QUOTE(arg) #arg
#define MACRO_QUOTE(arg) _MACRO_QUOTE(arg)
#define TEST_FUNC(numfreq, advance)                                     \
    printf("Testing %s()\n",                                            \
           MACRO_QUOTE(FUNC_NAME(numfreq, VECLEN, advance)));           \
    success &= test_all(FUNC_NAME(numfreq, VECLEN, advance), len, verbosity);

int main(void) {
    int verbosity = 1;
    int len = 2048;
    bool success = 1;

    MACRO_REPEAT_ADDING_ONE(TEST_FUNC, 8, 1, 0);
    MACRO_REPEAT_ADDING_ONE(TEST_FUNC, 7, 2, 1);
    MACRO_REPEAT_ADDING_ONE(TEST_FUNC, 6, 3, 2);
    MACRO_REPEAT_ADDING_ONE(TEST_FUNC, 5, 4, 3);
    MACRO_REPEAT_ADDING_ONE(TEST_FUNC, 4, 5, 4);
    
    return success;
}

#endif // TEST
