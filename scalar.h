#pragma once

#include <stdint.h>
#include <stddef.h>

size_t match_scalar(const uint32_t *A, const size_t lenA,
                    const uint32_t *B, const size_t lenB, 
                    uint32_t *out);
