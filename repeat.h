#define REPEAT(macro, times, start_n, next_func, next_arg, macro_args...) \
    _REPEAT_EXPAND(_REPEAT_ ## times(macro, start_n, next_func, next_arg, ## macro_args))
#define REPEAT_INNER(macro, times, start_n, next_func, next_arg, macro_args...) \
    _REPEAT_ ## times(macro, start_n, next_func, next_arg, ## macro_args)


#define REPEAT_ADD_ONE(macro, times, start_n, macro_args... )           \
    REPEAT(macro, times, start_n, _REPEAT_ADD_ONE, 0, ## macro_args)
#define REPEAT_ADD_ONE_INNER(macro, times, start_n, macro_args... )     \
    REPEAT_INNER(macro, times, start_n, _REPEAT_ADD_ONE, 0, ## macro_args)


#define REPEAT_ADDING(macro, times, start_n, increment, macro_args...)  \
    REPEAT(macro, times, start_n, _REPEAT_ADDING, increment, ## macro_args)

#define REPEAT_SAME_ARGS(macro, times, first_macro_arg, other_macro_args...) \
    REPEAT(macro, times, first_macro_arg, _REPEAT_UNCHANGED, 0, ## other_macro_args)

#define REPEAT_SAME_ARG(macro, times, first_macro_arg)          \
    REPEAT(macro, times, first_macro_arg, _REPEAT_UNCHANGED, 0)


#define REPEAT_NO_ARGS(macro, times)            \
    REPEAT(macro, times,, _REPEAT_UNCHANGED, 0)
#define REPEAT_NO_ARGS_INNER(macro, times)      \
    REPEAT_INNER(macro, times,, _REPEAT_UNCHANGED, 0)


#define REPEAT_BLOCK(block, times)              \
    REPEAT(do block while, times, 0, _REPEAT_UNCHANGED, 0)

#define REPEAT_APPEND_LITERAL(macro, times, initial, literal)           \
    REPEAT(macro, times, initial, _REPEAT_APPEND_LITERAL, literal)

// FUTURE: REPEAT_SUB, REPEAT_MUL, REPEAT_DIV
#define REPEAT_ADD(x, y) _REPEAT_ADDING(x, y)

// INTERNALS

#define _REPEAT_EMPTY()
#define _REPEAT_DEFER(token) token _REPEAT_EMPTY()

#define _REPEAT_EXPAND(args...) _REPEAT_EXPAND_9(args)
#define _REPEAT_EXPAND_9(args...) _REPEAT_EXPAND_3(_REPEAT_EXPAND_3(_REPEAT_EXPAND_3(args)))
#define _REPEAT_EXPAND_3(args...) _REPEAT_EXPAND_1(_REPEAT_EXPAND_1(_REPEAT_EXPAND_1(args)))
#define _REPEAT_EXPAND_1(args...) args

// For REPEAT using a variable number of arguments instead
// http://stackoverflow.com/questions/1872220/is-it-possible-to-iterate-over-arguments-in-variadic-macros


#define _REPEAT_ADDING(n, inc) _REPEAT_ADD_ ## n(inc)
#define _REPEAT_ADD_ONE(n, ignore...) _REPEAT_ADD_ONE_ ## n
#define _REPEAT_UNCHANGED(unchanged, ignore...) unchanged
#define _REPEAT_APPEND_LITERAL(n, literal) n literal


#define _REPEAT_0(args...)  /* empty */
#define _REPEAT_1(macro, n, func, i, args...) _REPEAT_DEFER(macro)(n, ## args) 
#define _REPEAT_2(m, n, f, i, a...) _REPEAT_DEFER(m)(n, ## a); _REPEAT_1(m, f(n, i), f, i, ## a)
#define _REPEAT_3(m, n, f, i, a...) _REPEAT_DEFER(m)(n, ## a); _REPEAT_2(m, f(n, i), f, i, ## a)
#define _REPEAT_4(m, n, f, i, a...) _REPEAT_DEFER(m)(n, ## a); _REPEAT_3(m, f(n, i), f, i, ## a)
#define _REPEAT_5(m, n, f, i, a...) _REPEAT_DEFER(m)(n, ## a); _REPEAT_4(m, f(n, i), f, i, ## a)
#define _REPEAT_6(m, n, f, i, a...) _REPEAT_DEFER(m)(n, ## a); _REPEAT_5(m, f(n, i), f, i, ## a)
#define _REPEAT_7(m, n, f, i, a...) _REPEAT_DEFER(m)(n, ## a); _REPEAT_6(m, f(n, i), f, i, ## a)
#define _REPEAT_8(m, n, f, i, a...) _REPEAT_DEFER(m)(n, ## a); _REPEAT_7(m, f(n, i), f, i, ## a)
#define _REPEAT_9(m, n, f, i, a...) _REPEAT_DEFER(m)(n, ## a); _REPEAT_8(m, f(n, i), f, i, ## a)
#define _REPEAT_10(m, n, f, i, a...) _REPEAT_DEFER(m)(n, ## a); _REPEAT_9(m, f(n, i), f, i, ## a)
#define _REPEAT_11(m, n, f, i, a...) _REPEAT_DEFER(m)(n, ## a); _REPEAT_10(m, f(n, i), f, i, ## a)
#define _REPEAT_12(m, n, f, i, a...) _REPEAT_DEFER(m)(n, ## a); _REPEAT_11(m, f(n, i), f, i, ## a)
#define _REPEAT_13(m, n, f, i, a...) _REPEAT_DEFER(m)(n, ## a); _REPEAT_12(m, f(n, i), f, i, ## a)
#define _REPEAT_14(m, n, f, i, a...) _REPEAT_DEFER(m)(n, ## a); _REPEAT_13(m, f(n, i), f, i, ## a)
#define _REPEAT_15(m, n, f, i, a...) _REPEAT_DEFER(m)(n, ## a); _REPEAT_14(m, f(n, i), f, i, ## a)
#define _REPEAT_16(m, n, f, i, a...) _REPEAT_DEFER(m)(n, ## a); _REPEAT_15(m, f(n, i), f, i, ## a)
#define _REPEAT_17(m, n, f, i, a...) _REPEAT_DEFER(m)(n, ## a); _REPEAT_16(m, f(n, i), f, i, ## a)
#define _REPEAT_18(m, n, f, i, a...) _REPEAT_DEFER(m)(n, ## a); _REPEAT_17(m, f(n, i), f, i, ## a)
#define _REPEAT_19(m, n, f, i, a...) _REPEAT_DEFER(m)(n, ## a); _REPEAT_18(m, f(n, i), f, i, ## a)
#define _REPEAT_20(m, n, f, i, a...) _REPEAT_DEFER(m)(n, ## a); _REPEAT_19(m, f(n, i), f, i, ## a)
#define _REPEAT_21(m, n, f, i, a...) _REPEAT_DEFER(m)(n, ## a); _REPEAT_20(m, f(n, i), f, i, ## a)
#define _REPEAT_22(m, n, f, i, a...) _REPEAT_DEFER(m)(n, ## a); _REPEAT_21(m, f(n, i), f, i, ## a)
#define _REPEAT_23(m, n, f, i, a...) _REPEAT_DEFER(m)(n, ## a); _REPEAT_22(m, f(n, i), f, i, ## a)
#define _REPEAT_24(m, n, f, i, a...) _REPEAT_DEFER(m)(n, ## a); _REPEAT_23(m, f(n, i), f, i, ## a)
#define _REPEAT_25(m, n, f, i, a...) _REPEAT_DEFER(m)(n, ## a); _REPEAT_24(m, f(n, i), f, i, ## a)
#define _REPEAT_26(m, n, f, i, a...) _REPEAT_DEFER(m)(n, ## a); _REPEAT_25(m, f(n, i), f, i, ## a)
#define _REPEAT_27(m, n, f, i, a...) _REPEAT_DEFER(m)(n, ## a); _REPEAT_26(m, f(n, i), f, i, ## a)
#define _REPEAT_28(m, n, f, i, a...) _REPEAT_DEFER(m)(n, ## a); _REPEAT_27(m, f(n, i), f, i, ## a)
#define _REPEAT_29(m, n, f, i, a...) _REPEAT_DEFER(m)(n, ## a); _REPEAT_28(m, f(n, i), f, i, ## a)
#define _REPEAT_30(m, n, f, i, a...) _REPEAT_DEFER(m)(n, ## a); _REPEAT_29(m, f(n, i), f, i, ## a)
#define _REPEAT_31(m, n, f, i, a...) _REPEAT_DEFER(m)(n, ## a); _REPEAT_30(m, f(n, i), f, i, ## a)
#define _REPEAT_32(m, n, f, i, a...) _REPEAT_DEFER(m)(n, ## a); _REPEAT_31(m, f(n, i), f, i, ## a)

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
#define _REPEAT_ADD_11(x) _REPEAT_ADD_10(_REPEAT_ADD_ONE(x))
#define _REPEAT_ADD_12(x) _REPEAT_ADD_11(_REPEAT_ADD_ONE(x))
#define _REPEAT_ADD_13(x) _REPEAT_ADD_12(_REPEAT_ADD_ONE(x))
#define _REPEAT_ADD_14(x) _REPEAT_ADD_13(_REPEAT_ADD_ONE(x))
#define _REPEAT_ADD_15(x) _REPEAT_ADD_14(_REPEAT_ADD_ONE(x))
#define _REPEAT_ADD_16(x) _REPEAT_ADD_15(_REPEAT_ADD_ONE(x))
#define _REPEAT_ADD_17(x) _REPEAT_ADD_16(_REPEAT_ADD_ONE(x))
#define _REPEAT_ADD_18(x) _REPEAT_ADD_17(_REPEAT_ADD_ONE(x))
#define _REPEAT_ADD_19(x) _REPEAT_ADD_18(_REPEAT_ADD_ONE(x))
#define _REPEAT_ADD_20(x) _REPEAT_ADD_19(_REPEAT_ADD_ONE(x))
#define _REPEAT_ADD_21(x) _REPEAT_ADD_20(_REPEAT_ADD_ONE(x))
#define _REPEAT_ADD_22(x) _REPEAT_ADD_21(_REPEAT_ADD_ONE(x))
#define _REPEAT_ADD_23(x) _REPEAT_ADD_22(_REPEAT_ADD_ONE(x))
#define _REPEAT_ADD_24(x) _REPEAT_ADD_23(_REPEAT_ADD_ONE(x))
#define _REPEAT_ADD_25(x) _REPEAT_ADD_24(_REPEAT_ADD_ONE(x))
#define _REPEAT_ADD_26(x) _REPEAT_ADD_25(_REPEAT_ADD_ONE(x))
#define _REPEAT_ADD_27(x) _REPEAT_ADD_26(_REPEAT_ADD_ONE(x))
#define _REPEAT_ADD_28(x) _REPEAT_ADD_27(_REPEAT_ADD_ONE(x))
#define _REPEAT_ADD_29(x) _REPEAT_ADD_28(_REPEAT_ADD_ONE(x))
#define _REPEAT_ADD_30(x) _REPEAT_ADD_29(_REPEAT_ADD_ONE(x))
#define _REPEAT_ADD_31(x) _REPEAT_ADD_30(_REPEAT_ADD_ONE(x))


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

int printf(const char *format, ...);

#define PRINTD(x) printf("%d\n", x)
void simple_add_one() {
    REPEAT_ADD_ONE(PRINTD, 16, 1);
}

#define NUMOUTER 2
#define NUMINNER 3
#define INNER(freqnum, rarenum) printf("(%d, %d)\n", freqnum, rarenum)
#define OUTER(n) REPEAT_ADD_ONE_INNER(INNER, NUMINNER, 1, n)
void recursive_add_one(void) {
    REPEAT_ADD_ONE(OUTER, NUMOUTER, 0);
}

#define NO_ARGS() printf("noargs\n")
void no_args() {
    REPEAT_NO_ARGS(NO_ARGS, 5);
}

void repeat_adding() {
    REPEAT_ADDING(PRINTD, 16, 0, 2);
}

void same_args() {
    REPEAT_SAME_ARGS(printf, 4, "functions OK too\n");
}

void block() {
    REPEAT_BLOCK({PRINTD(1); PRINTD(2);}, 3);
}

#define DECLARE(x, base) int base ## x = x
void declare() {
    REPEAT_ADD_ONE(DECLARE, 3, 0, var);
    printf("sum: %d\n", var0 + var1 + var2);
}

void cycle4() {
    REPEAT_CYCLE(PRINTD, 8, 0, 4);
}

#define PRINTS(s) printf("%s\n", s)
void literal(int var) {
    REPEAT_APPEND_LITERAL(PRINTD, 4, var, + 3);
    REPEAT_APPEND_LITERAL(PRINTS, 3, "foo", "bar");
}

int main() {
    recursive_add_one();
    simple_add_one();
    no_args();
    repeat_adding();
    same_args();
    block();
    declare();
    cycle4();
    int var = 2; literal(var); 
    return 0;
}


#endif
