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

#define _REPEAT_ADD_ONE(n, ignore) _REPEAT_ADD_ONE_ ## n
#define _REPEAT_INCREMENT(n, inc) n + inc
#define _REPEAT_UNCHANGED(unchanged, ignore) unchanged

#define _REPEAT_1(macro, n, func, i, args...) macro(n, ## args) 
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

#define _REPEAT_ADD_ONE_0 1
#define _REPEAT_ADD_ONE_1 2
#define _REPEAT_ADD_ONE_2 3
#define _REPEAT_ADD_ONE_3 4
#define _REPEAT_ADD_ONE_4 5
#define _REPEAT_ADD_ONE_5 6
#define _REPEAT_ADD_ONE_6 7
#define _REPEAT_ADD_ONE_7 8
#define _REPEAT_ADD_ONE_8 9
#define _REPEAT_ADD_ONE_9 10
#define _REPEAT_ADD_ONE_10 11
#define _REPEAT_ADD_ONE_11 12
#define _REPEAT_ADD_ONE_12 13
#define _REPEAT_ADD_ONE_13 14
#define _REPEAT_ADD_ONE_14 15
#define _REPEAT_ADD_ONE_15 16
#define _REPEAT_ADD_ONE_16 17
#define _REPEAT_ADD_ONE_17 18
#define _REPEAT_ADD_ONE_18 19
#define _REPEAT_ADD_ONE_19 20
#define _REPEAT_ADD_ONE_20 21
#define _REPEAT_ADD_ONE_21 22
#define _REPEAT_ADD_ONE_22 23
#define _REPEAT_ADD_ONE_23 24
#define _REPEAT_ADD_ONE_24 25
#define _REPEAT_ADD_ONE_25 26
#define _REPEAT_ADD_ONE_26 27
#define _REPEAT_ADD_ONE_27 28
#define _REPEAT_ADD_ONE_28 29
#define _REPEAT_ADD_ONE_29 30
#define _REPEAT_ADD_ONE_30 31
#define _REPEAT_ADD_ONE_31 32

// FUTURE: find a more generic manner of cycling?
// FUTURE: add cycles of different lengths

#define REPEAT_CYCLE(macro, times, start_n, cycle_len, macro_args... )   \
    REPEAT(macro, times, start_n, REPEAT_CYCLE ## cycle_len ## _NEXT, 0, ## macro_args)

#define REPEAT_CYCLE4_NEXT(n, ignore...) _REPEAT_CYCLE4_NEXT_ ## n
#define _REPEAT_CYCLE4_NEXT_0 1
#define _REPEAT_CYCLE4_NEXT_1 2
#define _REPEAT_CYCLE4_NEXT_2 3
#define _REPEAT_CYCLE4_NEXT_3 0

#define REPEAT_CYCLE4_PREV(n, ignore...) _REPEAT_CYCLE4_PREV_ ## n
#define _REPEAT_CYCLE4_PREV_0 3
#define _REPEAT_CYCLE4_PREV_1 0
#define _REPEAT_CYCLE4_PREV_2 1
#define _REPEAT_CYCLE4_PREV_3 2


#ifdef MAIN
#include <stdio.h>

#define PRINT(x) printf("%d\n", x)
#define NOARGS() printf("noargs\n")
#define PASTE(x, base) paste(base ## x)

int main() {
    REPEAT_ADD_ONE(PRINT, 32, 0);
    REPEAT_INCREMENT(PRINT, 16, 0, 2);
    REPEAT_NOARGS(NOARGS, 5);
    REPEAT_BLOCK({PRINT(1); PRINT(2)}, 3);
    REPEAT_SAME_ARGS(printf, 4, "functions OK too\n");
    uint32_t var = 3;  REPEAT_INCREMENT(PRINT, 5, var, 3);
    REPEAT_ADD_ONE(PASTE, 4, 0, var);
    REPEAT_CYCLE(PRINT, 8, 0, 4);
}

#endif
