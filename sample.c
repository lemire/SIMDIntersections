#define EXPAND(args...) args

#define REPEAT(macro, times, start_n, next_func, next_arg, macro_args...) \
    _REPEAT_ ## times(macro, start_n, next_func, next_arg, ## macro_args)

#define REPEAT_ADD_ONE(macro, times, start_n, macro_args... )                    \
    EXPAND(REPEAT(macro, times, start_n, _REPEAT_ADD_ONE, 0, ## macro_args))

#define _REPEAT_ADD_ONE(n, ignore...) _REPEAT_ADD_ONE_ ## n

#define _REPEAT_0(args...)  /* empty */
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

#define _REPEAT_ADD_0(x) x
#define _REPEAT_ADD_1(x) _REPEAT_ADD_ONE(x)
#define _REPEAT_ADD_2(x) _REPEAT_ADD_1(_REPEAT_ADD_ONE(x))
#define _REPEAT_ADD_3(x) _REPEAT_ADD_2(_REPEAT_ADD_ONE(x))
#define _REPEAT_ADD_4(x) _REPEAT_ADD_3(_REPEAT_ADD_ONE(x))
#define _REPEAT_ADD_5(x) _REPEAT_ADD_4(_REPEAT_ADD_ONE(x))
#define _REPEAT_ADD_6(x) _REPEAT_ADD_5(_REPEAT_ADD_ONE(x))
#define _REPEAT_ADD_7(x) _REPEAT_ADD_6(_REPEAT_ADD_ONE(x))
#define _REPEAT_ADD_8(x) _REPEAT_ADD_7(_REPEAT_ADD_ONE(x))
#define _REPEAT_ADD_9(x) _REPEAT_ADD_8(_REPEAT_ADD_ONE(x))
#define _REPEAT_ADD_10(x) _REPEAT_ADD_9(_REPEAT_ADD_ONE(x))

#define REPEAT_EMPTY()
#define REPEAT_DEFER(token) token REPEAT_EMPTY()
#define REPEAT_EXPAND(args...) args

#define REPEAT_ADD_ONE_INNER REPEAT_DEFER(_REPEAT_ADD_ONE_INNER)
#define _REPEAT_ADD_ONE_INNER(args...) REPEAT(args)
//    REPEAT(macro, times, start_n, _REPEAT_ADD_ONE, 0, ## macro_args)
#define _REPEAT_ADD_ONE_INNER(args...) REPEAT_ADD_ONE(args)

#define REPEAT_ADD_ONE_INNER2 REPEAT_DEFER(_REPEAT_ADD_ONE_INNER2)
#define _REPEAT_ADD_ONE_INNER2(args...) REPEAT_ADD_ONE(args)

#define INNER(i, j, k) if (i == j == k) printf("Match\n")
#define INNER_MACRO(inner, outer) REPEAT_ADD_ONE_INNER2(INNER, 2, 2, inner, outer)
#define OUTER_MACRO(outer) REPEAT_ADD_ONE_INNER(INNER_MACRO, 3, 0, outer)
void working() {
    REPEAT_EXPAND(REPEAT_EXPAND(REPEAT_ADD_ONE(OUTER_MACRO, 2, 1)));
}


#ifdef WORKS
#define INNER_MACRO(inner, outer) if (inner == outer) printf("Match\n")
#define OUTER_MACRO(outer) DEFER(REPEAT_ADD_ONE)(INNER_MACRO, 3, 0, outer)
// can be DEFER or OBSTRUCT in OUTER_MACRO_INNER_MACRO
void outer_macro_inner_macro() {
    EVAL(REPEAT(OUTER_MACRO, 2, 1, _REPEAT_ADD_ONE, 0))
}
#endif // WORKS

#ifdef WORKS2
#define INNER_MACRO(inner, outer) if (inner == outer) printf("Match\n")
#define OUTER_MACRO(outer) DEFER(RAO)(INNER_MACRO, 3, 0, outer)
#define RAO(args...) REPEAT_ADD_ONE(args)
void outer_macro_inner_macro() {
    EVAL(REPEAT_ADD_ONE(OUTER_MACRO, 2, 1));
}
#endif

#ifdef WORKS3
#define REPEAT_ADD_ONE_RECURSE(macro, times, start_n, macro_args... )      \
    EXPAND(REPEAT(macro, times, start_n, _REPEAT_ADD_ONE, 0, ## macro_args))
#define INNER_MACRO(inner, outer) if (inner == outer) printf("Match\n")
#define OUTER_MACRO(outer) DEFER(REPEAT_ADD_ONE)(INNER_MACRO, 3, 0, outer)
void outer_macro_inner_macro() {
    REPEAT_ADD_ONE_RECURSE(OUTER_MACRO, 2, 1));
}
#endif

void desired_results() {
   {
       if (0 == 1) printf("Match\n");
       if (1 == 1) printf("Match\n");
       if (2 == 1) printf("Match\n");
   };
   {
       if (0 == 2) printf("Match\n");
       if (1 == 2) printf("Match\n");
       if (2 == 2) printf("Match\n");
   };
}


// #define INNER_BLOCK  { if (inner == outer) printf("Match\n"); }
// #define OUTER_BLOCK_INNER_MACRO { REPEAT_ADD_ONE(INNER_MACRO, 3, 0, outer); }
// #define OUTER_MACRO_INNER_BLOCK(outer) REPEAT_ADD_ONE(INNER_BLOCK, 3, 0, outer)
// #define OUTER_BLOCK_INNER_BLOCK { REPEAT_ADD_ONE(INNER_BLOCK, 3, 0, outer); }
// void outer_macro_inner_block() {
//     REPEAT_ADD_ONE(OUTER_MACRO_INNER_BLOCK, 2, 1);
// }
// 
// void outer_block_inner_macro() {
//     REPEAT_ADD_ONE(OUTER_BLOCK_INNER_MACRO, 2, 1);
// }
// 
// void outer_block_inner_block() {
//     REPEAT_ADD_ONE(OUTER_BLOCK_INNER_BLOCK, 2, 1);
// }
