#define REPEAT_ADD(macro, times, start, increment)         \
    _REPEAT_ ## times(macro, _REPEAT_ADD, start, increment)

#define REPEAT_SAME(macro, times, arg)                     \
    _REPEAT_ ## times(macro, _REPEAT_SAME, arg, 0)

// INTERNALS

#define _REPEAT_ADD(n, i) (n + i)
#define _REPEAT_SAME(n, i) (n)

#define _REPEAT_1(macro, next, n, i) macro(n) 
#define _REPEAT_2(macro, next, n, i) macro(n); _REPEAT_1(macro, next, next(n, i), i)
#define _REPEAT_3(macro, next, n, i) macro(n); _REPEAT_2(macro, next, next(n, i), i)
#define _REPEAT_4(macro, next, n, i) macro(n); _REPEAT_3(macro, next, next(n, i), i)
#define _REPEAT_5(macro, next, n, i) macro(n); _REPEAT_4(macro, next, next(n, i), i)
#define _REPEAT_6(macro, next, n, i) macro(n); _REPEAT_5(macro, next, next(n, i), i)
#define _REPEAT_7(macro, next, n, i) macro(n); _REPEAT_6(macro, next, next(n, i), i)
#define _REPEAT_8(macro, next, n, i) macro(n); _REPEAT_7(macro, next, next(n, i), i)
#define _REPEAT_9(macro, next, n, i) macro(n); _REPEAT_8(macro, next, next(n, i), i)
#define _REPEAT_10(macro, next, n, i) macro(n); _REPEAT_9(macro, next, next(n, i), i)
#define _REPEAT_11(macro, next, n, i) macro(n); _REPEAT_10(macro, next, next(n, i), i)
#define _REPEAT_12(macro, next, n, i) macro(n); _REPEAT_11(macro, next, next(n, i), i)
#define _REPEAT_13(macro, next, n, i) macro(n); _REPEAT_12(macro, next, next(n, i), i)
#define _REPEAT_14(macro, next, n, i) macro(n); _REPEAT_13(macro, next, next(n, i), i)
#define _REPEAT_15(macro, next, n, i) macro(n); _REPEAT_14(macro, next, next(n, i), i)
#define _REPEAT_16(macro, next, n, i) macro(n); _REPEAT_15(macro, next, next(n, i), i)
#define _REPEAT_17(macro, next, n, i) macro(n); _REPEAT_16(macro, next, next(n, i), i)
#define _REPEAT_18(macro, next, n, i) macro(n); _REPEAT_17(macro, next, next(n, i), i)
#define _REPEAT_19(macro, next, n, i) macro(n); _REPEAT_18(macro, next, next(n, i), i)
#define _REPEAT_20(macro, next, n, i) macro(n); _REPEAT_19(macro, next, next(n, i), i)
#define _REPEAT_21(macro, next, n, i) macro(n); _REPEAT_20(macro, next, next(n, i), i)
#define _REPEAT_22(macro, next, n, i) macro(n); _REPEAT_21(macro, next, next(n, i), i)
#define _REPEAT_23(macro, next, n, i) macro(n); _REPEAT_22(macro, next, next(n, i), i)
#define _REPEAT_24(macro, next, n, i) macro(n); _REPEAT_23(macro, next, next(n, i), i)
#define _REPEAT_25(macro, next, n, i) macro(n); _REPEAT_24(macro, next, next(n, i), i)
#define _REPEAT_26(macro, next, n, i) macro(n); _REPEAT_25(macro, next, next(n, i), i)
#define _REPEAT_27(macro, next, n, i) macro(n); _REPEAT_26(macro, next, next(n, i), i)
#define _REPEAT_28(macro, next, n, i) macro(n); _REPEAT_27(macro, next, next(n, i), i)
#define _REPEAT_29(macro, next, n, i) macro(n); _REPEAT_28(macro, next, next(n, i), i)
#define _REPEAT_30(macro, next, n, i) macro(n); _REPEAT_29(macro, next, next(n, i), i)
#define _REPEAT_31(macro, next, n, i) macro(n); _REPEAT_30(macro, next, next(n, i), i)
#define _REPEAT_32(macro, next, n, i) macro(n); _REPEAT_31(macro, next, next(n, i), i)

