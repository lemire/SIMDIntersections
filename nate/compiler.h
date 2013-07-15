#pragma once

// NOTE: the same 'symbol' can only be used once per compilation unit
#define COMPILER_ASSERT(test)                           \
    _COMPILER_ASSERT_LINE(test, __LINE__, #test )
#define _COMPILER_ASSERT_LINE(test, line, string)                       \
    _COMPILER_ASSERT_PASTE(test, COMPILER_LINE_, line, string)
#define _COMPILER_ASSERT_PASTE(test, prefix, line, string)             \
    _COMPILER_ASSERT_SYMBOL(test, prefix ## line, string)
#define _COMPILER_ASSERT_SYMBOL(test, symbol, string)          \
    typedef char symbol [ ((test) ? 1 : -1) ]
    // typedef char symbol [ ((test) ? 1 : string) ]

#define COMPILER_BARRIER asm volatile("" ::: "memory")


#define COMPILER_LIKELY(x)     __builtin_expect((x),1)
#define COMPILER_RARELY(x)     __builtin_expect((x),0)
