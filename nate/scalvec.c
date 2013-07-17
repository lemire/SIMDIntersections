#include "vec4.h"
#include "compiler.h"
#include "scalar.h"
#include "macro.h"
#include "asm.h"

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


#define _FUNC_NAME(numfreq, veclen)             \
    match_scalvec_v ## veclen ## _f ## numfreq

#define FUNC_NAME(args...) _FUNC_NAME(args)

#define NUMFREQ 1
#define SCALVEC_MATCH_ALL()                   
#include "scalvec.def"
#undef SCALVEC_MATCH_ALL
#undef NUMFREQ

#define NUMFREQ 2
#define SCALVEC_MATCH_ALL()                     \
    VEC_OR(F0, F1)
#include "scalvec.def"
#undef SCALVEC_MATCH_ALL
#undef NUMFREQ

#define NUMFREQ 3
#define SCALVEC_MATCH_ALL()                     \
    VEC_OR(F0, F1);                             \
    VEC_OR(F0, F2)
#include "scalvec.def"
#undef SCALVEC_MATCH_ALL
#undef NUMFREQ

#define NUMFREQ 4
#define SCALVEC_MATCH_ALL()                             \
    VEC_OR(F0, F1); VEC_OR(F2, F3);                     \
    VEC_OR(F0, F2);
#include "scalvec.def"
#undef SCALVEC_MATCH_ALL
#undef NUMFREQ

#define NUMFREQ 5
#define SCALVEC_MATCH_ALL()                             \
    VEC_OR(F0, F1); VEC_OR(F2, F3);                     \
    VEC_OR(F0, F2);                                     \
    VEC_OR(F0, F4)
#include "scalvec.def"
#undef SCALVEC_MATCH_ALL
#undef NUMFREQ

#define NUMFREQ 6
#define SCALVEC_MATCH_ALL()                             \
    VEC_OR(F0, F1); VEC_OR(F2, F3); VEC_OR(F4, F5);     \
    VEC_OR(F0, F2);                                     \
    VEC_OR(F0, F4)
#include "scalvec.def"
#undef SCALVEC_MATCH_ALL
#undef NUMFREQ

#define NUMFREQ 7
#define SCALVEC_MATCH_ALL()                                   \
    VEC_OR(F0, F1); VEC_OR(F2, F3); VEC_OR(F4, F5);           \
    VEC_OR(F0, F2); VEC_OR(F4, F6);                           \
    VEC_OR(F0, F4)
#include "scalvec.def"
#undef SCALVEC_MATCH_ALL
#undef NUMFREQ

#define NUMFREQ 8
#define SCALVEC_MATCH_ALL()                                             \
    VEC_OR(F0, F1); VEC_OR(F2, F3); VEC_OR(F4, F5); VEC_OR(F6, F7);     \
    VEC_OR(F0, F2); VEC_OR(F4, F6);                                     \
    VEC_OR(F0, F4)
#include "scalvec.def"
#undef SCALVEC_MATCH_ALL
#undef NUMFREQ

#define NUMFREQ 9
#define SCALVEC_MATCH_ALL()                                             \
    VEC_OR(F0, F1); VEC_OR(F2, F3); VEC_OR(F4, F5); VEC_OR(F6, F7);     \
    VEC_OR(F0, F2); VEC_OR(F4, F6);                                     \
    VEC_OR(F0, F4);                                                     \
    VEC_OR(F0, F8);
#include "scalvec.def"
#undef SCALVEC_MATCH_ALL
#undef NUMFREQ

#define NUMFREQ 10
#define SCALVEC_MATCH_ALL()                                             \
    VEC_OR(F0, F1); VEC_OR(F2, F3); VEC_OR(F4, F5); VEC_OR(F6, F7);     \
    VEC_OR(F0, F2); VEC_OR(F4, F6);                                     \
    VEC_OR(F0, F4); VEC_OR(F8, F9);                                     \
    VEC_OR(F0, F8);
#include "scalvec.def"
#undef SCALVEC_MATCH_ALL
#undef NUMFREQ

#define NUMFREQ 11
#define SCALVEC_MATCH_ALL()                                             \
    VEC_OR(F0, F1); VEC_OR(F2, F3); VEC_OR(F4, F5); VEC_OR(F6, F7);     \
    VEC_OR(F0, F2); VEC_OR(F4, F6); VEC_OR(F8, F9);                     \
    VEC_OR(F0, F4); VEC_OR(F8, F10);                                    \
    VEC_OR(F0, F8);
#include "scalvec.def"
#undef SCALVEC_MATCH_ALL
#undef NUMFREQ

#define NUMFREQ 12
#define SCALVEC_MATCH_ALL()                                             \
    VEC_OR(F0, F1); VEC_OR(F2, F3); VEC_OR(F4, F5); VEC_OR(F6, F7);     \
    VEC_OR(F0, F2); VEC_OR(F4, F6); VEC_OR(F8, F9); VEC_OR(F10, F11);   \
    VEC_OR(F0, F4); VEC_OR(F8, F10);                                    \
    VEC_OR(F0, F8);
#include "scalvec.def"
#undef SCALVEC_MATCH_ALL
#undef NUMFREQ

#ifdef TEST

#include "test.h"
#include <stdio.h>

#define _MACRO_QUOTE(arg) #arg
#define MACRO_QUOTE(arg) _MACRO_QUOTE(arg)
#define TEST_FUNC(numfreq)                                              \
    printf("Testing %s()\n",                                            \
           MACRO_QUOTE(FUNC_NAME(numfreq, VECLEN)));           \
    success &= test_all(FUNC_NAME(numfreq, VECLEN), len, verbosity);

int main(void) {
    int verbosity = 1;
    int len = 2048;
    bool success = 1;

    MACRO_REPEAT_ADDING_ONE(TEST_FUNC, 12, 1);

    return success == 1;
}

#endif // TEST
