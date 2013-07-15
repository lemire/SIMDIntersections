#pragma once


#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

typedef size_t (match_func_t) (const uint32_t *, size_t,
                               const uint32_t *, size_t, 
                               uint32_t *);

bool test_identical(match_func_t match_func, size_t len, int verbosity);

bool test_half(match_func_t match_func, size_t len, int verbosity);

bool test_random(match_func_t match_func, size_t len,
                 uint32_t increment, int verbosity);

bool test_all(match_func_t match_func, size_t len, int verbosity);

