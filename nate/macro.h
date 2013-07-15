#pragma once

#define MACRO_REPEAT(macro, times, start_n, func_next, func_arg, macro_args...) \
    _MACRO_EXPAND(_MACRO_ ## times(macro, start_n, func_next, func_arg, ## macro_args))
#define MACRO_REPEAT_INNER(macro, times, start_n, func_next, func_arg, macro_args...) \
    _MACRO_REPEAT_INNER(macro, times, start_n, func_next, func_arg, macro_args)
#define _MACRO_REPEAT_INNER(macro, times, start_n, func_next, func_arg, macro_args...) \
    _MACRO_ ## times(macro, start_n, func_next, func_arg, ## macro_args)

#define MACRO_REPEAT_ADDING_ONE(macro, times, start_n, macro_args... )           \
    MACRO_REPEAT(macro, times, start_n, _MACRO_ADD_ONE, 0, ## macro_args)
#define MACRO_REPEAT_ADDING_ONE_INNER(macro, times, start_n, macro_args... )     \
    MACRO_REPEAT_INNER(macro, times, start_n, _MACRO_ADD_ONE, 0, ## macro_args)

#define MACRO_REPEAT_ADDING(macro, times, start_n, increment, macro_args...)  \
    MACRO_REPEAT(macro, times, start_n, _MACRO_ADD, increment, ## macro_args)
#define MACRO_REPEAT_ADDING_INNER(macro, times, start_n, increment, macro_args...)  \
    MACRO_REPEAT_INNER(macro, times, start_n, _MACRO_ADD, increment, ## macro_args)

#define MACRO_REPEAT_SAME_ARGS(macro, times, first_macro_arg, other_macro_args...) \
    MACRO_REPEAT(macro, times, first_macro_arg, _MACRO_UNCHANGED, 0, ## other_macro_args)

#define MACRO_REPEAT_NO_ARGS(macro, times)            \
    MACRO_REPEAT(macro, times,, _MACRO_UNCHANGED, 0)

#define MACRO_REPEAT_BLOCK(block, times)              \
    MACRO_REPEAT(do block while, times, 0, _MACRO_UNCHANGED, 0)

#define MACRO_REPEAT_APPEND_LITERAL(macro, times, initial, literal)           \
    MACRO_REPEAT(macro, times, initial, _MACRO_APPEND_LITERAL, literal)

#define MACRO_REPEAT_CYCLE(macro, times, start_n, cycle, macro_args... )   \
    MACRO_REPEAT(macro, times, start_n, _MACRO_CYCLE ## cycle ## _NEXT, 0, ## macro_args)

#define MACRO_ADD(x, y) _MACRO_ADD(x, y)
#define MACRO_SUB(large, small) _MACRO_SUB(large, small)
#define MACRO_MUL(x, y)  _MACRO_MUL(x, y)

#define MACRO_DIV(number, divisor) _MACRO_EXPAND(_MACRO_DIV(number, divisor))
#define MACRO_DIV_INNER(number, divisor) _MACRO_DIV(number, divisor)

// FUTURE: need MACRO_MUL_INNER also?

// INTERNALS

#define _MACRO_EMPTY()
#define _MACRO_DEFER(token) token _MACRO_EMPTY()

#define _MACRO_EXPAND(args...) _MACRO_EXPAND_9(_MACRO_EXPAND_9(_MACRO_EXPAND_9(args)))
#define _MACRO_EXPAND_9(args...) _MACRO_EXPAND_3(_MACRO_EXPAND_3(_MACRO_EXPAND_3(args)))
#define _MACRO_EXPAND_3(args...) _MACRO_EXPAND_1(_MACRO_EXPAND_1(_MACRO_EXPAND_1(args)))
#define _MACRO_EXPAND_1(args...) args

// For REPEAT using a variable number of arguments instead
// http://stackoverflow.com/questions/1872220/is-it-possible-to-iterate-over-arguments-in-variadic-macros

#define _MACRO_ADD(n, inc) _MACRO_ADD_ ## n(inc)
#define _MACRO_ADD_ONE(n, ignore...) _MACRO_ADD_ONE_ ## n
#define _MACRO_UNCHANGED(unchanged, ignore...) unchanged
#define _MACRO_APPEND_LITERAL(n, literal) n literal

// FUTURE: improve this interface?
#ifndef MACRO_SEP
#define MACRO_SEP ;
#endif

#define _MACRO_0(args...)  /* empty */
#define _MACRO_1(macro, n, func, i, args...) _MACRO_DEFER(macro)(n, ## args) 
#define _MACRO_2(m, n, f, i, a...) _MACRO_DEFER(m)(n, ## a) MACRO_SEP _MACRO_1(m, f(n, i), f, i, ## a)
#define _MACRO_3(m, n, f, i, a...) _MACRO_DEFER(m)(n, ## a) MACRO_SEP _MACRO_2(m, f(n, i), f, i, ## a)
#define _MACRO_4(m, n, f, i, a...) _MACRO_DEFER(m)(n, ## a) MACRO_SEP _MACRO_3(m, f(n, i), f, i, ## a)
#define _MACRO_5(m, n, f, i, a...) _MACRO_DEFER(m)(n, ## a) MACRO_SEP _MACRO_4(m, f(n, i), f, i, ## a)
#define _MACRO_6(m, n, f, i, a...) _MACRO_DEFER(m)(n, ## a) MACRO_SEP _MACRO_5(m, f(n, i), f, i, ## a)
#define _MACRO_7(m, n, f, i, a...) _MACRO_DEFER(m)(n, ## a) MACRO_SEP _MACRO_6(m, f(n, i), f, i, ## a)
#define _MACRO_8(m, n, f, i, a...) _MACRO_DEFER(m)(n, ## a) MACRO_SEP _MACRO_7(m, f(n, i), f, i, ## a)
#define _MACRO_9(m, n, f, i, a...) _MACRO_DEFER(m)(n, ## a) MACRO_SEP _MACRO_8(m, f(n, i), f, i, ## a)
#define _MACRO_10(m, n, f, i, a...) _MACRO_DEFER(m)(n, ## a) MACRO_SEP _MACRO_9(m, f(n, i), f, i, ## a)
#define _MACRO_11(m, n, f, i, a...) _MACRO_DEFER(m)(n, ## a) MACRO_SEP _MACRO_10(m, f(n, i), f, i, ## a)
#define _MACRO_12(m, n, f, i, a...) _MACRO_DEFER(m)(n, ## a) MACRO_SEP _MACRO_11(m, f(n, i), f, i, ## a)
#define _MACRO_13(m, n, f, i, a...) _MACRO_DEFER(m)(n, ## a) MACRO_SEP _MACRO_12(m, f(n, i), f, i, ## a)
#define _MACRO_14(m, n, f, i, a...) _MACRO_DEFER(m)(n, ## a) MACRO_SEP _MACRO_13(m, f(n, i), f, i, ## a)
#define _MACRO_15(m, n, f, i, a...) _MACRO_DEFER(m)(n, ## a) MACRO_SEP _MACRO_14(m, f(n, i), f, i, ## a)
#define _MACRO_16(m, n, f, i, a...) _MACRO_DEFER(m)(n, ## a) MACRO_SEP _MACRO_15(m, f(n, i), f, i, ## a)
#define _MACRO_17(m, n, f, i, a...) _MACRO_DEFER(m)(n, ## a) MACRO_SEP _MACRO_16(m, f(n, i), f, i, ## a)
#define _MACRO_18(m, n, f, i, a...) _MACRO_DEFER(m)(n, ## a) MACRO_SEP _MACRO_17(m, f(n, i), f, i, ## a)
#define _MACRO_19(m, n, f, i, a...) _MACRO_DEFER(m)(n, ## a) MACRO_SEP _MACRO_18(m, f(n, i), f, i, ## a)
#define _MACRO_20(m, n, f, i, a...) _MACRO_DEFER(m)(n, ## a) MACRO_SEP _MACRO_19(m, f(n, i), f, i, ## a)
#define _MACRO_21(m, n, f, i, a...) _MACRO_DEFER(m)(n, ## a) MACRO_SEP _MACRO_20(m, f(n, i), f, i, ## a)
#define _MACRO_22(m, n, f, i, a...) _MACRO_DEFER(m)(n, ## a) MACRO_SEP _MACRO_21(m, f(n, i), f, i, ## a)
#define _MACRO_23(m, n, f, i, a...) _MACRO_DEFER(m)(n, ## a) MACRO_SEP _MACRO_22(m, f(n, i), f, i, ## a)
#define _MACRO_24(m, n, f, i, a...) _MACRO_DEFER(m)(n, ## a) MACRO_SEP _MACRO_23(m, f(n, i), f, i, ## a)
#define _MACRO_25(m, n, f, i, a...) _MACRO_DEFER(m)(n, ## a) MACRO_SEP _MACRO_24(m, f(n, i), f, i, ## a)
#define _MACRO_26(m, n, f, i, a...) _MACRO_DEFER(m)(n, ## a) MACRO_SEP _MACRO_25(m, f(n, i), f, i, ## a)
#define _MACRO_27(m, n, f, i, a...) _MACRO_DEFER(m)(n, ## a) MACRO_SEP _MACRO_26(m, f(n, i), f, i, ## a)
#define _MACRO_28(m, n, f, i, a...) _MACRO_DEFER(m)(n, ## a) MACRO_SEP _MACRO_27(m, f(n, i), f, i, ## a)
#define _MACRO_29(m, n, f, i, a...) _MACRO_DEFER(m)(n, ## a) MACRO_SEP _MACRO_28(m, f(n, i), f, i, ## a)
#define _MACRO_30(m, n, f, i, a...) _MACRO_DEFER(m)(n, ## a) MACRO_SEP _MACRO_29(m, f(n, i), f, i, ## a)
#define _MACRO_31(m, n, f, i, a...) _MACRO_DEFER(m)(n, ## a) MACRO_SEP _MACRO_30(m, f(n, i), f, i, ## a)
#define _MACRO_32(m, n, f, i, a...) _MACRO_DEFER(m)(n, ## a) MACRO_SEP _MACRO_31(m, f(n, i), f, i, ## a)


#define _MACRO_ADD_ONE_0 1
#define _MACRO_ADD_ONE_1 2
#define _MACRO_ADD_ONE_2 3
#define _MACRO_ADD_ONE_3 4
#define _MACRO_ADD_ONE_4 5
#define _MACRO_ADD_ONE_5 6
#define _MACRO_ADD_ONE_6 7
#define _MACRO_ADD_ONE_7 8
#define _MACRO_ADD_ONE_8 9
#define _MACRO_ADD_ONE_9 10
#define _MACRO_ADD_ONE_10 11
#define _MACRO_ADD_ONE_11 12
#define _MACRO_ADD_ONE_12 13
#define _MACRO_ADD_ONE_13 14
#define _MACRO_ADD_ONE_14 15
#define _MACRO_ADD_ONE_15 16
#define _MACRO_ADD_ONE_16 17
#define _MACRO_ADD_ONE_17 18
#define _MACRO_ADD_ONE_18 19
#define _MACRO_ADD_ONE_19 20
#define _MACRO_ADD_ONE_20 21
#define _MACRO_ADD_ONE_21 22
#define _MACRO_ADD_ONE_22 23
#define _MACRO_ADD_ONE_23 24
#define _MACRO_ADD_ONE_24 25
#define _MACRO_ADD_ONE_25 26
#define _MACRO_ADD_ONE_26 27
#define _MACRO_ADD_ONE_27 28
#define _MACRO_ADD_ONE_28 29
#define _MACRO_ADD_ONE_29 30
#define _MACRO_ADD_ONE_30 31
#define _MACRO_ADD_ONE_31 32

#define _MACRO_ADD_0(x) x
#define _MACRO_ADD_1(x) _MACRO_ADD_ONE(x)
#define _MACRO_ADD_2(x) _MACRO_ADD_1(_MACRO_ADD_ONE(x))
#define _MACRO_ADD_3(x) _MACRO_ADD_2(_MACRO_ADD_ONE(x))
#define _MACRO_ADD_4(x) _MACRO_ADD_3(_MACRO_ADD_ONE(x))
#define _MACRO_ADD_5(x) _MACRO_ADD_4(_MACRO_ADD_ONE(x))
#define _MACRO_ADD_6(x) _MACRO_ADD_5(_MACRO_ADD_ONE(x))
#define _MACRO_ADD_7(x) _MACRO_ADD_6(_MACRO_ADD_ONE(x))
#define _MACRO_ADD_8(x) _MACRO_ADD_7(_MACRO_ADD_ONE(x))
#define _MACRO_ADD_9(x) _MACRO_ADD_8(_MACRO_ADD_ONE(x))
#define _MACRO_ADD_10(x) _MACRO_ADD_9(_MACRO_ADD_ONE(x))
#define _MACRO_ADD_11(x) _MACRO_ADD_10(_MACRO_ADD_ONE(x))
#define _MACRO_ADD_12(x) _MACRO_ADD_11(_MACRO_ADD_ONE(x))
#define _MACRO_ADD_13(x) _MACRO_ADD_12(_MACRO_ADD_ONE(x))
#define _MACRO_ADD_14(x) _MACRO_ADD_13(_MACRO_ADD_ONE(x))
#define _MACRO_ADD_15(x) _MACRO_ADD_14(_MACRO_ADD_ONE(x))
#define _MACRO_ADD_16(x) _MACRO_ADD_15(_MACRO_ADD_ONE(x))
#define _MACRO_ADD_17(x) _MACRO_ADD_16(_MACRO_ADD_ONE(x))
#define _MACRO_ADD_18(x) _MACRO_ADD_17(_MACRO_ADD_ONE(x))
#define _MACRO_ADD_19(x) _MACRO_ADD_18(_MACRO_ADD_ONE(x))
#define _MACRO_ADD_20(x) _MACRO_ADD_19(_MACRO_ADD_ONE(x))
#define _MACRO_ADD_21(x) _MACRO_ADD_20(_MACRO_ADD_ONE(x))
#define _MACRO_ADD_22(x) _MACRO_ADD_21(_MACRO_ADD_ONE(x))
#define _MACRO_ADD_23(x) _MACRO_ADD_22(_MACRO_ADD_ONE(x))
#define _MACRO_ADD_24(x) _MACRO_ADD_23(_MACRO_ADD_ONE(x))
#define _MACRO_ADD_25(x) _MACRO_ADD_24(_MACRO_ADD_ONE(x))
#define _MACRO_ADD_26(x) _MACRO_ADD_25(_MACRO_ADD_ONE(x))
#define _MACRO_ADD_27(x) _MACRO_ADD_26(_MACRO_ADD_ONE(x))
#define _MACRO_ADD_28(x) _MACRO_ADD_27(_MACRO_ADD_ONE(x))
#define _MACRO_ADD_29(x) _MACRO_ADD_28(_MACRO_ADD_ONE(x))
#define _MACRO_ADD_30(x) _MACRO_ADD_29(_MACRO_ADD_ONE(x))
#define _MACRO_ADD_31(x) _MACRO_ADD_30(_MACRO_ADD_ONE(x))

#define _MACRO_SUB_ONE(n) _MACRO_SUB_ONE_ ## n
#define _MACRO_SUB_ONE_0 MACRO_SUB_ERROR_LIMIT_ZERO ## ! 
#define _MACRO_SUB_ONE_1 0
#define _MACRO_SUB_ONE_2 1
#define _MACRO_SUB_ONE_3 2
#define _MACRO_SUB_ONE_4 3
#define _MACRO_SUB_ONE_5 4
#define _MACRO_SUB_ONE_6 5
#define _MACRO_SUB_ONE_7 6
#define _MACRO_SUB_ONE_8 7
#define _MACRO_SUB_ONE_9 8
#define _MACRO_SUB_ONE_10 9
#define _MACRO_SUB_ONE_11 10
#define _MACRO_SUB_ONE_12 11
#define _MACRO_SUB_ONE_13 12
#define _MACRO_SUB_ONE_14 13
#define _MACRO_SUB_ONE_15 14
#define _MACRO_SUB_ONE_16 15
#define _MACRO_SUB_ONE_17 16
#define _MACRO_SUB_ONE_18 17
#define _MACRO_SUB_ONE_19 18
#define _MACRO_SUB_ONE_20 19
#define _MACRO_SUB_ONE_21 20
#define _MACRO_SUB_ONE_22 21
#define _MACRO_SUB_ONE_23 22
#define _MACRO_SUB_ONE_24 23
#define _MACRO_SUB_ONE_25 24
#define _MACRO_SUB_ONE_26 25
#define _MACRO_SUB_ONE_27 26
#define _MACRO_SUB_ONE_28 27
#define _MACRO_SUB_ONE_29 28
#define _MACRO_SUB_ONE_30 29
#define _MACRO_SUB_ONE_31 30
#define _MACRO_SUB_ONE_32 31

#define _MACRO_SUB(large, small) _MACRO_SUB_ ## small(large)
#define _MACRO_SUB_0(x) x
#define _MACRO_SUB_1(x) _MACRO_SUB_ONE(x)
#define _MACRO_SUB_2(x) _MACRO_SUB_1(_MACRO_SUB_ONE(x))
#define _MACRO_SUB_3(x) _MACRO_SUB_2(_MACRO_SUB_ONE(x))
#define _MACRO_SUB_4(x) _MACRO_SUB_3(_MACRO_SUB_ONE(x))
#define _MACRO_SUB_5(x) _MACRO_SUB_4(_MACRO_SUB_ONE(x))
#define _MACRO_SUB_6(x) _MACRO_SUB_5(_MACRO_SUB_ONE(x))
#define _MACRO_SUB_7(x) _MACRO_SUB_6(_MACRO_SUB_ONE(x))
#define _MACRO_SUB_8(x) _MACRO_SUB_7(_MACRO_SUB_ONE(x))
#define _MACRO_SUB_9(x) _MACRO_SUB_8(_MACRO_SUB_ONE(x))
#define _MACRO_SUB_10(x) _MACRO_SUB_9(_MACRO_SUB_ONE(x))
#define _MACRO_SUB_11(x) _MACRO_SUB_10(_MACRO_SUB_ONE(x))
#define _MACRO_SUB_12(x) _MACRO_SUB_11(_MACRO_SUB_ONE(x))
#define _MACRO_SUB_13(x) _MACRO_SUB_12(_MACRO_SUB_ONE(x))
#define _MACRO_SUB_14(x) _MACRO_SUB_13(_MACRO_SUB_ONE(x))
#define _MACRO_SUB_15(x) _MACRO_SUB_14(_MACRO_SUB_ONE(x))
#define _MACRO_SUB_16(x) _MACRO_SUB_15(_MACRO_SUB_ONE(x))
#define _MACRO_SUB_17(x) _MACRO_SUB_16(_MACRO_SUB_ONE(x))
#define _MACRO_SUB_18(x) _MACRO_SUB_17(_MACRO_SUB_ONE(x))
#define _MACRO_SUB_19(x) _MACRO_SUB_18(_MACRO_SUB_ONE(x))
#define _MACRO_SUB_20(x) _MACRO_SUB_19(_MACRO_SUB_ONE(x))
#define _MACRO_SUB_21(x) _MACRO_SUB_20(_MACRO_SUB_ONE(x))
#define _MACRO_SUB_22(x) _MACRO_SUB_21(_MACRO_SUB_ONE(x))
#define _MACRO_SUB_23(x) _MACRO_SUB_22(_MACRO_SUB_ONE(x))
#define _MACRO_SUB_24(x) _MACRO_SUB_23(_MACRO_SUB_ONE(x))
#define _MACRO_SUB_25(x) _MACRO_SUB_24(_MACRO_SUB_ONE(x))
#define _MACRO_SUB_26(x) _MACRO_SUB_25(_MACRO_SUB_ONE(x))
#define _MACRO_SUB_27(x) _MACRO_SUB_26(_MACRO_SUB_ONE(x))
#define _MACRO_SUB_28(x) _MACRO_SUB_27(_MACRO_SUB_ONE(x))
#define _MACRO_SUB_29(x) _MACRO_SUB_28(_MACRO_SUB_ONE(x))
#define _MACRO_SUB_30(x) _MACRO_SUB_29(_MACRO_SUB_ONE(x))
#define _MACRO_SUB_31(x) _MACRO_SUB_30(_MACRO_SUB_ONE(x))
#define _MACRO_SUB_32(x) _MACRO_SUB_31(_MACRO_SUB_ONE(x))


#define _MACRO_MUL(x, y) _MACRO_EXPAND(_MACRO_MUL_ ## x(y))
#define _MACRO_MUL_0(n) 0
#define _MACRO_MUL_1(n) n
#define _MACRO_MUL_2(n) _MACRO_DEFER(_MACRO_ADD_ ## n)(n)
#define _MACRO_MUL_3(n) _MACRO_DEFER(_MACRO_ADD_ ## n)(_MACRO_MUL_2(n))
#define _MACRO_MUL_4(n) _MACRO_DEFER(_MACRO_ADD_ ## n)(_MACRO_MUL_3(n))
#define _MACRO_MUL_5(n) _MACRO_DEFER(_MACRO_ADD_ ## n)(_MACRO_MUL_4(n))
#define _MACRO_MUL_6(n) _MACRO_DEFER(_MACRO_ADD_ ## n)(_MACRO_MUL_5(n))
#define _MACRO_MUL_7(n) _MACRO_DEFER(_MACRO_ADD_ ## n)(_MACRO_MUL_6(n))
#define _MACRO_MUL_8(n) _MACRO_DEFER(_MACRO_ADD_ ## n)(_MACRO_MUL_7(n))
#define _MACRO_MUL_9(n) _MACRO_DEFER(_MACRO_ADD_ ## n)(_MACRO_MUL_8(n))
#define _MACRO_MUL_10(n) _MACRO_DEFER(_MACRO_ADD_ ## n)(_MACRO_MUL_9(n))
#define _MACRO_MUL_11(n) _MACRO_DEFER(_MACRO_ADD_ ## n)(_MACRO_MUL_10(n))
#define _MACRO_MUL_12(n) _MACRO_DEFER(_MACRO_ADD_ ## n)(_MACRO_MUL_11(n))
#define _MACRO_MUL_13(n) _MACRO_DEFER(_MACRO_ADD_ ## n)(_MACRO_MUL_12(n))
#define _MACRO_MUL_14(n) _MACRO_DEFER(_MACRO_ADD_ ## n)(_MACRO_MUL_13(n))
#define _MACRO_MUL_15(n) _MACRO_DEFER(_MACRO_ADD_ ## n)(_MACRO_MUL_14(n))
#define _MACRO_MUL_16(n) _MACRO_DEFER(_MACRO_ADD_ ## n)(_MACRO_MUL_15(n))
#define _MACRO_MUL_17(n) _MACRO_DEFER(_MACRO_ADD_ ## n)(_MACRO_MUL_16(n))
#define _MACRO_MUL_18(n) _MACRO_DEFER(_MACRO_ADD_ ## n)(_MACRO_MUL_17(n))
#define _MACRO_MUL_19(n) _MACRO_DEFER(_MACRO_ADD_ ## n)(_MACRO_MUL_18(n))
#define _MACRO_MUL_20(n) _MACRO_DEFER(_MACRO_ADD_ ## n)(_MACRO_MUL_19(n))
#define _MACRO_MUL_21(n) _MACRO_DEFER(_MACRO_ADD_ ## n)(_MACRO_MUL_20(n))
#define _MACRO_MUL_22(n) _MACRO_DEFER(_MACRO_ADD_ ## n)(_MACRO_MUL_21(n))
#define _MACRO_MUL_23(n) _MACRO_DEFER(_MACRO_ADD_ ## n)(_MACRO_MUL_22(n))
#define _MACRO_MUL_24(n) _MACRO_DEFER(_MACRO_ADD_ ## n)(_MACRO_MUL_23(n))
#define _MACRO_MUL_25(n) _MACRO_DEFER(_MACRO_ADD_ ## n)(_MACRO_MUL_24(n))
#define _MACRO_MUL_26(n) _MACRO_DEFER(_MACRO_ADD_ ## n)(_MACRO_MUL_25(n))
#define _MACRO_MUL_27(n) _MACRO_DEFER(_MACRO_ADD_ ## n)(_MACRO_MUL_26(n))
#define _MACRO_MUL_28(n) _MACRO_DEFER(_MACRO_ADD_ ## n)(_MACRO_MUL_27(n))
#define _MACRO_MUL_29(n) _MACRO_DEFER(_MACRO_ADD_ ## n)(_MACRO_MUL_28(n))
#define _MACRO_MUL_30(n) _MACRO_DEFER(_MACRO_ADD_ ## n)(_MACRO_MUL_29(n))
#define _MACRO_MUL_31(n) _MACRO_DEFER(_MACRO_ADD_ ## n)(_MACRO_MUL_30(n))
#define _MACRO_MUL_32(n) _MACRO_DEFER(_MACRO_ADD_ ## n)(_MACRO_MUL_31(n))

#define _MACRO_DIV(number, divisor) __MACRO_DIV(number, divisor)
#define __MACRO_DIV(number, divisor) _MACRO_DIV_ ## divisor(number)
#define _MACRO_DIV_0(n) MACRO_DIV_ZERO_ERROR ## !
#define _MACRO_DIV_1(n) n
#define _MACRO_DIV_2(n) _MACRO_DIV_2_ ## n
#define _MACRO_DIV_2_2 1
#define _MACRO_DIV_2_4 2
#define _MACRO_DIV_2_6 3
#define _MACRO_DIV_2_8 4
#define _MACRO_DIV_2_10 5
#define _MACRO_DIV_2_12 6
#define _MACRO_DIV_2_14 7
#define _MACRO_DIV_2_16 8
#define _MACRO_DIV_2_18 9
#define _MACRO_DIV_2_20 10
#define _MACRO_DIV_2_22 11
#define _MACRO_DIV_2_24 12
#define _MACRO_DIV_2_26 13
#define _MACRO_DIV_2_28 14
#define _MACRO_DIV_2_30 15
#define _MACRO_DIV_2_32 16
#define _MACRO_DIV_3(n) _MACRO_DIV_3_ ## n
#define _MACRO_DIV_3_3 1
#define _MACRO_DIV_3_6 2
#define _MACRO_DIV_3_9 3
#define _MACRO_DIV_3_12 4
#define _MACRO_DIV_3_15 5
#define _MACRO_DIV_3_18 6
#define _MACRO_DIV_3_21 7
#define _MACRO_DIV_3_24 8
#define _MACRO_DIV_3_27 9
#define _MACRO_DIV_3_30 10
#define _MACRO_DIV_4(n) _MACRO_DEFER(_MACRO_DIV_2)(_MACRO_DIV_2(n))
#define _MACRO_DIV_5(n) _MACRO_DIV_5_ ## n
#define _MACRO_DIV_5_5 1
#define _MACRO_DIV_5_10 2
#define _MACRO_DIV_5_15 3
#define _MACRO_DIV_5_20 4
#define _MACRO_DIV_5_25 5
#define _MACRO_DIV_5_30 6
#define _MACRO_DIV_6(n) _MACRO_DEFER(_MACRO_DIV_2)(_MACRO_DIV_3(n))
#define _MACRO_DIV_7(n) _MACRO_DIV_7_ ## n
#define _MACRO_DIV_7_7 1
#define _MACRO_DIV_7_14 2
#define _MACRO_DIV_7_21 3
#define _MACRO_DIV_7_28 4
#define _MACRO_DIV_8(n) _MACRO_DEFER(_MACRO_DIV_2)(_MACRO_DIV_4(n))
#define _MACRO_DIV_9(n) _MACRO_DIV_9_ ## n
#define _MACRO_DIV_9_9 1
#define _MACRO_DIV_9_18 2
#define _MACRO_DIV_9_27 3
#define _MACRO_DIV_10(n) _MACRO_DEFER(_MACRO_DIV_2)(_MACRO_DIV_5(n))
#define _MACRO_DIV_11(n) _MACRO_DIV_11_ ## n
#define _MACRO_DIV_11_11 1
#define _MACRO_DIV_11_22 2
#define _MACRO_DIV_12(n) _MACRO_DEFER(_MACRO_DIV_3)(_MACRO_DIV_6(n))
#define _MACRO_DIV_13(n) _MACRO_DIV_13_ ## n
#define _MACRO_DIV_13_13 1
#define _MACRO_DIV_13_26 2
#define _MACRO_DIV_14(n) _MACRO_DEFER(_MACRO_DIV_2)(_MACRO_DIV_7(n))
#define _MACRO_DIV_15(n) _MACRO_DIV_15_ ## n
#define _MACRO_DIV_15_15 1
#define _MACRO_DIV_15_30 2
#define _MACRO_DIV_16(n) _MACRO_DEFER(_MACRO_DIV_2)(_MACRO_DIV_8(n))
// ...

// FUTURE: find a more generic manner of cycling?
// FUTURE: add cycles of different lengths

#define _MACRO_CYCLE4_NEXT(n, ignore...) _MACRO_CYCLE4_NEXT_ ## n
#define _MACRO_CYCLE4_NEXT_0 1
#define _MACRO_CYCLE4_NEXT_1 2
#define _MACRO_CYCLE4_NEXT_2 3
#define _MACRO_CYCLE4_NEXT_3 0

#define _MACRO_CYCLE4_PREV(n, ignore...) _MACRO_CYCLE4_PREV_ ## n
#define _MACRO_CYCLE4_PREV_0 3
#define _MACRO_CYCLE4_PREV_1 0
#define _MACRO_CYCLE4_PREV_2 1
#define _MACRO_CYCLE4_PREV_3 2


#ifdef MACRO_MAIN

int printf(const char *format, ...);

#define PRINTD(x) printf("%d\n", x)
void simple_add_one() {
    MACRO_REPEAT_ADDING_ONE(PRINTD, 16, 1);
}

#define NUMOUTER 2
#define NUMINNER 3
#define INNER(freqnum, rarenum) printf("(%d, %d)\n", freqnum, rarenum)
#define OUTER(n) MACRO_REPEAT_ADDING_ONE_INNER(INNER, NUMINNER, 1, n)
void recursive_add_one(void) {
    MACRO_REPEAT_ADDING_ONE(OUTER, NUMOUTER, 0);
}

#define NO_ARGS() printf("noargs\n")
void no_args() {
    MACRO_REPEAT_NO_ARGS(NO_ARGS, 5);
}

#define ONE 1
#define TWO 2
void repeat_adding() {
    MACRO_REPEAT_ADDING(PRINTD, 16, 0, MACRO_MUL(ONE, TWO));
}

void same_args() {
    MACRO_REPEAT_SAME_ARGS(printf, MACRO_DIV(8, 2), "functions OK too\n");
}

void block() {
    MACRO_REPEAT_BLOCK({PRINTD(MACRO_SUB(10,9)); PRINTD(MACRO_ADD(1,1));}, 3);
}

#define DECLARE(x, base) int base ## x = x
void declare() {
    MACRO_REPEAT_ADDING_ONE(DECLARE, 3, 0, var);
    printf("sum: %d\n", var0 + var1 + var2);
}

void cycle4() {
    MACRO_REPEAT_CYCLE(PRINTD, 8, 0, 4);
}

#define PRINTS(s) printf("%s\n", s)
void literal(int var) {
    MACRO_REPEAT_APPEND_LITERAL(PRINTD, 4, var, + 3);
    MACRO_REPEAT_APPEND_LITERAL(PRINTS, 3, "foo", "bar");
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


#endif // MACRO_MAIN
