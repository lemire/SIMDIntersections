#ifndef _REPEAT_H
#define _REPEAT_H

// Usage: REPEAT_EXPAND(REPEAT_ADD_ONE(macro, times, start_n, macro_args... ))
//        Recursion allowed.  Only outermost level should use REPEAT_EXPAND.
//        This demo header only allows 3 layers of recursion and max n=10.
//        Sample code at bottom.

#define REPEAT_EXPAND(args...) _REPEAT_EXPAND_3(args)

#define REPEAT_ADD_ONE(macro, times, start_n, macro_args... )           \
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

int printf(const char *format, ...);

#define BODY(i) printf("%d\n", i);
void simple(void) {
    REPEAT_EXPAND(REPEAT_ADD_ONE(BODY, 5, 1));
}

#define INNER(k, j, i) \
    printf("(%d, %d, %d)\n", i, j, k);          \
    if (i == j && j == k) printf("Match!\n")
#define MIDDLE(j, i) REPEAT_ADD_ONE(INNER, 2, 2, j, i)
#define OUTER(i) REPEAT_ADD_ONE(MIDDLE, 3, 0, i)
void recursive(void) {
    REPEAT_EXPAND(REPEAT_ADD_ONE(OUTER, 2, 1));
}

int main() {
    simple();
    recursive();
    return 0;
}

#endif // SAMPLE_CODE 
