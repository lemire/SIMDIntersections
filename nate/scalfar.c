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


#define _FUNC_NAME(numfreq, numprobe, veclen)           \
    match_scalvec_v ## veclen ## _f ## numfreq ## _p ## numprobe

#define FUNC_NAME(args...) _FUNC_NAME(args)

#define NUMPROBE 0

#define NUMFREQ 2
#define SCALFAR_MATCH_ALL()                     \
    VEC_OR(F0, F1)
#include "scalfar.def"
#undef SCALFAR_MATCH_ALL
#undef NUMFREQ


#define NUMFREQ 4
#define SCALFAR_MATCH_ALL()                             \
    VEC_OR(F0, F1); VEC_OR(F2, F3);                     \
    VEC_OR(F0, F2);
#include "scalfar.def"
#undef SCALFAR_MATCH_ALL
#undef NUMFREQ

#define NUMFREQ 8
#define SCALFAR_MATCH_ALL()                                             \
    VEC_OR(F0, F1); VEC_OR(F2, F3); VEC_OR(F4, F5); VEC_OR(F6, F7);     \
    VEC_OR(F0, F2); VEC_OR(F4, F6);                                     \
    VEC_OR(F0, F4)
#include "scalfar.def"
#undef SCALFAR_MATCH_ALL
#undef NUMFREQ

#undef NUMPROBE
#define NUMPROBE 1

#define NUMFREQ 2
#define SCALFAR_MATCH_ALL()                     \
    VEC_OR(F0, F1)
#include "scalfar.def"
#undef SCALFAR_MATCH_ALL
#undef NUMFREQ


#define NUMFREQ 4
#define SCALFAR_MATCH_ALL()                             \
    VEC_OR(F0, F1); VEC_OR(F2, F3);                     \
    VEC_OR(F0, F2);
#include "scalfar.def"
#undef SCALFAR_MATCH_ALL
#undef NUMFREQ

#define NUMFREQ 8
#define SCALFAR_MATCH_ALL()                                             \
    VEC_OR(F0, F1); VEC_OR(F2, F3); VEC_OR(F4, F5); VEC_OR(F6, F7);     \
    VEC_OR(F0, F2); VEC_OR(F4, F6);                                     \
    VEC_OR(F0, F4)
#include "scalfar.def"
#undef SCALFAR_MATCH_ALL
#undef NUMFREQ

#undef NUMPROBE
#define NUMPROBE 2

#define NUMFREQ 2
#define SCALFAR_MATCH_ALL()                     \
    VEC_OR(F0, F1)
#include "scalfar.def"
#undef SCALFAR_MATCH_ALL
#undef NUMFREQ


#define NUMFREQ 4
#define SCALFAR_MATCH_ALL()                             \
    VEC_OR(F0, F1); VEC_OR(F2, F3);                     \
    VEC_OR(F0, F2);
#include "scalfar.def"
#undef SCALFAR_MATCH_ALL
#undef NUMFREQ

#define NUMFREQ 8
#define SCALFAR_MATCH_ALL()                                             \
    VEC_OR(F0, F1); VEC_OR(F2, F3); VEC_OR(F4, F5); VEC_OR(F6, F7);     \
    VEC_OR(F0, F2); VEC_OR(F4, F6);                                     \
    VEC_OR(F0, F4)
#include "scalfar.def"
#undef SCALFAR_MATCH_ALL
#undef NUMFREQ

#undef NUMPROBE
#define NUMPROBE 3

#define NUMFREQ 2
#define SCALFAR_MATCH_ALL()                     \
    VEC_OR(F0, F1)
#include "scalfar.def"
#undef SCALFAR_MATCH_ALL
#undef NUMFREQ


#define NUMFREQ 4
#define SCALFAR_MATCH_ALL()                             \
    VEC_OR(F0, F1); VEC_OR(F2, F3);                     \
    VEC_OR(F0, F2);
#include "scalfar.def"
#undef SCALFAR_MATCH_ALL
#undef NUMFREQ

#define NUMFREQ 8
#define SCALFAR_MATCH_ALL()                                             \
    VEC_OR(F0, F1); VEC_OR(F2, F3); VEC_OR(F4, F5); VEC_OR(F6, F7);     \
    VEC_OR(F0, F2); VEC_OR(F4, F6);                                     \
    VEC_OR(F0, F4)
#include "scalfar.def"
#undef SCALFAR_MATCH_ALL
#undef NUMFREQ
 

#ifdef TEST

#include "test.h"
#include <stdio.h>

#define _MACRO_QUOTE(arg) #arg
#define MACRO_QUOTE(arg) _MACRO_QUOTE(arg)
#define TEST_FUNC(numprobe, numfreq)                                    \
    printf("Testing %s()\n",                                            \
           MACRO_QUOTE(FUNC_NAME(numfreq, numprobe, VECLEN)));          \
    success &= test_all(FUNC_NAME(numfreq, numprobe, VECLEN), len, verbosity);

int main(void) {
    int verbosity = 1;
    int len = 8096;
    bool success = 1;

    MACRO_REPEAT_ADDING_ONE(TEST_FUNC, 4, 0, 2);

    MACRO_REPEAT_ADDING_ONE(TEST_FUNC, 4, 0, 4);

    MACRO_REPEAT_ADDING_ONE(TEST_FUNC, 4, 0, 8);

    return success == 1;
}

#endif // TEST
