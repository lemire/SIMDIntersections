#ifndef _REPEAT_H
#define _REPEAT_H

// Usage: REPEAT_ADD_ONE(macro, times, start_n, macro_args... )
//        Recursion allowed if inner macros use REPEAT_ADD_ONE_INNER().
//        This demo header only allows 3 layers of recursion and max n=10.
//        Sample code at bottom.

#define REPEAT_ADD_ONE(macro, times, start_n, macro_args... )           \
    _REPEAT_EXPAND_3(REPEAT_ADD_ONE_INNER(macro, times, start_n, ## macro_args))

#define REPEAT_ADD_ONE_INNER(macro, times, start_n, macro_args... )     \
    _REPEAT_ ## times(macro, start_n, _REPEAT_ADD_ONE, ## macro_args)

#define _REPEAT_0(args...)  /* empty */
#define _REPEAT_1(macro, n, func, args...) _REPEAT_DEFER(macro)(n, ## args)
#define _REPEAT_2(m, n, f, a...) _REPEAT_DEFER(m)(n, ## a); _REPEAT_1(m, f(n), f, ## a)
#define _REPEAT_3(m, n, f, a...) _REPEAT_DEFER(m)(n, ## a); _REPEAT_2(m, f(n), f, ## a)
#define _REPEAT_4(m, n, f, a...) _REPEAT_DEFER(m)(n, ## a); _REPEAT_3(m, f(n), f, ## a)
#define _REPEAT_5(m, n, f, a...) _REPEAT_DEFER(m)(n, ## a); _REPEAT_4(m, f(n), f, ## a)
#define _REPEAT_6(m, n, f, a...) _REPEAT_DEFER(m)(n, ## a); _REPEAT_5(m, f(n), f, ## a)
#define _REPEAT_7(m, n, f, a...) _REPEAT_DEFER(m)(n, ## a); _REPEAT_6(m, f(n), f, ## a)
#define _REPEAT_8(m, n, f, a...) _REPEAT_DEFER(m)(n, ## a); _REPEAT_7(m, f(n), f, ## a)
#define _REPEAT_9(m, n, f, a...) _REPEAT_DEFER(m)(n, ## a); _REPEAT_8(m, f(n), f, ## a)
#define _REPEAT_10(m, n, f, a...) _REPEAT_DEFER(m)(n, ## a); _REPEAT_9(m, f(n), f, ## a)
// ...

#define _REPEAT_ADD_ONE(n, ignore...) _REPEAT_ADD_ONE_ ## n
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
// ...

#define REPEAT_DIV(number, divisor) _REPEAT_EXPAND(_REPEAT_DIV_ ## divisor(number))
#define _REPEAT_DIV_1(n) n
#define _REPEAT_DIV_2(n) _REPEAT_DIV_2_ ## n
#define _REPEAT_DIV_2_2 1
#define _REPEAT_DIV_2_4 2
#define _REPEAT_DIV_2_6 3
#define _REPEAT_DIV_2_8 4
#define _REPEAT_DIV_2_10 5
#define _REPEAT_DIV_2_12 6
#define _REPEAT_DIV_2_14 7
#define _REPEAT_DIV_2_16 8
#define _REPEAT_DIV_2_18 9
#define _REPEAT_DIV_2_20 10
#define _REPEAT_DIV_2_22 11
#define _REPEAT_DIV_2_24 12
#define _REPEAT_DIV_2_26 13
#define _REPEAT_DIV_2_28 14
#define _REPEAT_DIV_2_30 15
#define _REPEAT_DIV_2_32 16
#define _REPEAT_DIV_3(n) _REPEAT_DIV_3_ ## n
#define _REPEAT_DIV_3_3 1
#define _REPEAT_DIV_3_6 2
#define _REPEAT_DIV_3_9 3
#define _REPEAT_DIV_3_12 4
#define _REPEAT_DIV_3_15 5
#define _REPEAT_DIV_3_18 6
#define _REPEAT_DIV_3_21 7
#define _REPEAT_DIV_3_24 8
#define _REPEAT_DIV_3_27 9
#define _REPEAT_DIV_3_30 10
#define _REPEAT_DIV_4(n) _REPEAT_DEFER(_REPEAT_DIV_2)(_REPEAT_DIV_2(n))
#define _REPEAT_DIV_5(n) _REPEAT_DIV_5_ ## n
#define _REPEAT_DIV_5_5 1
#define _REPEAT_DIV_5_10 2
#define _REPEAT_DIV_5_15 3
#define _REPEAT_DIV_5_20 4
#define _REPEAT_DIV_5_25 5
#define _REPEAT_DIV_5_30 6
#define _REPEAT_DIV_6(n) _REPEAT_DEFER(_REPEAT_DIV_2)(_REPEAT_DIV_3(n))
#define _REPEAT_DIV_7(n) _REPEAT_DIV_7_ ## n
#define _REPEAT_DIV_7_7 1
#define _REPEAT_DIV_7_14 2
#define _REPEAT_DIV_7_21 3
#define _REPEAT_DIV_7_28 4
#define _REPEAT_DIV_8(n) _REPEAT_DEFER(_REPEAT_DIV_2)(_REPEAT_DIV_4(n))
#define _REPEAT_DIV_9(n) _REPEAT_DIV_9_ ## n
#define _REPEAT_DIV_9_9 1
#define _REPEAT_DIV_9_18 2
#define _REPEAT_DIV_9_27 3
#define _REPEAT_DIV_10(n) _REPEAT_DEFER(_REPEAT_DIV_2)(_REPEAT_DIV_5(n))
#define _REPEAT_DIV_11(n) _REPEAT_DIV_11_ ## n
#define _REPEAT_DIV_11_11 1
#define _REPEAT_DIV_11_22 2
#define _REPEAT_DIV_12(n) _REPEAT_DEFER(_REPEAT_DIV_3)(_REPEAT_DIV_6(n))
#define _REPEAT_DIV_13(n) _REPEAT_DIV_13_ ## n
#define _REPEAT_DIV_13_13 1
#define _REPEAT_DIV_13_26 2
#define _REPEAT_DIV_14(n) _REPEAT_DEFER(_REPEAT_DIV_2)(_REPEAT_DIV_7(n))
#define _REPEAT_DIV_15(n) _REPEAT_DIV_15_ ## n
#define _REPEAT_DIV_15_15 1
#define _REPEAT_DIV_15_30 2
#define _REPEAT_DIV_16(n) _REPEAT_DEFER(_REPEAT_DIV_2)(_REPEAT_DIV_8(n))

#define _REPEAT_EMPTY()
#define _REPEAT_DEFER(token) token _REPEAT_EMPTY()

#define _REPEAT_EXPAND_3(args...) _REPEAT_EXPAND(_REPEAT_EXPAND(_REPEAT_EXPAND(args)))
#define _REPEAT_EXPAND(args...) args
// ...

#endif // _REPEAT_H

#ifdef SAMPLE_CODE
// to generate code:   cpp -DSAMPLE_CODE sample.c 
// or easier to read:  cpp -DSAMPLE_CODE sample.c > out.c; astyle out.c; less out.c
// to compile and run: gcc  -Wall -O3 -DSAMPLE_CODE sample.c -o sample

int a = REPEAT_DIV(8, 2);
int b = REPEAT_DIV(16, 4);
int c = REPEAT_DIV(18, 6);
int d = REPEAT_DIV(24, 3);
int e = REPEAT_DIV(10, 10);

int printf(const char *format, ...);

#define BODY(i) printf("%d\n", i);
void simple(void) {
    REPEAT_ADD_ONE(BODY, 5, 1);
}

#define INNER(k, j, i) \
    printf("(%d, %d, %d)\n", i, j, k);          \
    if (i == j && j == k) printf("Match!\n")
#define MIDDLE(j, i) REPEAT_ADD_ONE_INNER(INNER, 2, 2, j, i)
#define OUTER(i) REPEAT_ADD_ONE_INNER(MIDDLE, 3, 0, i)
void recursive(void) {
    REPEAT_ADD_ONE(OUTER, 2, 1);
}

int main() {
    simple();
    recursive();
    return 0;
}

#endif // SAMPLE_CODE 
