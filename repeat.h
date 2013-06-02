#define REPEAT(macro, times, start_n, next_func, next_arg, macro_args...) \
    _REPEAT_ ## times(macro, start_n, next_func, next_arg, ## macro_args)

#define REPEAT_ADD_ONE(macro, times, start_n, macro_args... )                    \
    REPEAT(macro, times, start_n, _REPEAT_ADD_ONE, 0, ## macro_args)

#define REPEAT_INCREMENT(macro, times, start_n, increment, macro_args...)                 \
    REPEAT(macro, times, start_n, _REPEAT_INCREMENT, increment, ## macro_args)

#define REPEAT_SAME_ARGS(macro, times, first_macro_arg, other_macro_args...)      \
    REPEAT(macro, times, first_macro_arg, _REPEAT_UNCHANGED, 0, ## other_macro_args)

#define REPEAT_SAME_ARG(macro, times, first_macro_arg)      \
    REPEAT(macro, times, first_macro_arg, _REPEAT_UNCHANGED, 0)

#define REPEAT_NOARGS(macro, times)      \
    REPEAT(macro, times,, _REPEAT_UNCHANGED, 0)

#define REPEAT_BLOCK(block, times)              \
    REPEAT(do block while, times, 0, _REPEAT_UNCHANGED, 0)

// INTERNALS

#define _REPEAT_ADD_ONE(n, ignore) n + 1
#define _REPEAT_INCREMENT(n, inc) n + inc
#define _REPEAT_UNCHANGED(unchanged, ignore) unchanged

#define _REPEAT_1(m, n, f, i, a...) m(n, ## a) 
#define _REPEAT_2(m, n, f, i, a...) m(n, ## a); _REPEAT_1(m, f(n, i), f, i, ## a)
#define _REPEAT_3(m, n, f, i, a...) m(n, ## a); _REPEAT_2(m, f(n, i), f, i, ## a)
#define _REPEAT_4(m, n, f, i, a...) m(n, ## a); _REPEAT_3(m, f(n, i), f, i, ## a)
#define _REPEAT_5(m, n, f, i, a...) m(n, ## a); _REPEAT_4(m, f(n, i), f, i, ## a)
#define _REPEAT_6(m, n, f, i, a...) m(n, ## a); _REPEAT_5(m, f(n, i), f, i, ## a)
#define _REPEAT_7(m, n, f, i, a...) m(n, ## a); _REPEAT_6(m, f(n, i), f, i, ## a)
#define _REPEAT_8(m, n, f, i, a...) m(n, ## a); _REPEAT_7(m, f(n, i), f, i, ## a)
#define _REPEAT_9(m, n, f, i, a...) m(n, ## a); _REPEAT_8(m, f(n, i), f, i, ## a)
#define _REPEAT_10(m, n, f, i, a...) m(n, ## a); _REPEAT_9(m, f(n, i), f, i, ## a)
#define _REPEAT_11(m, n, f, i, a...) m(n, ## a); _REPEAT_10(m, f(n, i), f, i, ## a)
#define _REPEAT_12(m, n, f, i, a...) m(n, ## a); _REPEAT_11(m, f(n, i), f, i, ## a)
#define _REPEAT_13(m, n, f, i, a...) m(n, ## a); _REPEAT_12(m, f(n, i), f, i, ## a)
#define _REPEAT_14(m, n, f, i, a...) m(n, ## a); _REPEAT_13(m, f(n, i), f, i, ## a)
#define _REPEAT_15(m, n, f, i, a...) m(n, ## a); _REPEAT_14(m, f(n, i), f, i, ## a)
#define _REPEAT_16(m, n, f, i, a...) m(n, ## a); _REPEAT_15(m, f(n, i), f, i, ## a)
#define _REPEAT_17(m, n, f, i, a...) m(n, ## a); _REPEAT_16(m, f(n, i), f, i, ## a)
#define _REPEAT_18(m, n, f, i, a...) m(n, ## a); _REPEAT_17(m, f(n, i), f, i, ## a)
#define _REPEAT_19(m, n, f, i, a...) m(n, ## a); _REPEAT_18(m, f(n, i), f, i, ## a)
#define _REPEAT_20(m, n, f, i, a...) m(n, ## a); _REPEAT_19(m, f(n, i), f, i, ## a)
#define _REPEAT_21(m, n, f, i, a...) m(n, ## a); _REPEAT_20(m, f(n, i), f, i, ## a)
#define _REPEAT_22(m, n, f, i, a...) m(n, ## a); _REPEAT_21(m, f(n, i), f, i, ## a)
#define _REPEAT_23(m, n, f, i, a...) m(n, ## a); _REPEAT_22(m, f(n, i), f, i, ## a)
#define _REPEAT_24(m, n, f, i, a...) m(n, ## a); _REPEAT_23(m, f(n, i), f, i, ## a)
#define _REPEAT_25(m, n, f, i, a...) m(n, ## a); _REPEAT_24(m, f(n, i), f, i, ## a)
#define _REPEAT_26(m, n, f, i, a...) m(n, ## a); _REPEAT_25(m, f(n, i), f, i, ## a)
#define _REPEAT_27(m, n, f, i, a...) m(n, ## a); _REPEAT_26(m, f(n, i), f, i, ## a)
#define _REPEAT_28(m, n, f, i, a...) m(n, ## a); _REPEAT_27(m, f(n, i), f, i, ## a)
#define _REPEAT_29(m, n, f, i, a...) m(n, ## a); _REPEAT_28(m, f(n, i), f, i, ## a)
#define _REPEAT_30(m, n, f, i, a...) m(n, ## a); _REPEAT_29(m, f(n, i), f, i, ## a)
#define _REPEAT_31(m, n, f, i, a...) m(n, ## a); _REPEAT_30(m, f(n, i), f, i, ## a)
#define _REPEAT_32(m, n, f, i, a...) m(n, ## a); _REPEAT_31(m, f(n, i), f, i, ## a)


#ifdef MAIN

#include <stdio.h>

#define PRINT(x) printf("%d\n", x)

#define NOARGS() printf("noargs\n")

int main() {
    REPEAT_ADD_ONE(PRINT, 32, 0);
    REPEAT_INCREMENT(PRINT, 16, 0, 2);
    REPEAT_NOARGS(NOARGS, 5);
    REPEAT_BLOCK({PRINT(1); PRINT(2)}, 3);
    REPEAT_SAME_ARGS(printf, 4, "functions OK too\n");
    uint32_t var = 1;
    REPEAT_ADD_ONE(PRINT, 32, n);
}

#endif
