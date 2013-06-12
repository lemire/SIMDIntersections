#ifndef UNION_H_
#define UNION_H_
#include "common.h"

size_t unite(const uint32_t * set1, const size_t length1,
        const uint32_t * set2, const size_t length2, uint32_t * out) {
    size_t pos = 0;
    size_t k1 = 0, k2 = 0;
    if (0 == length1) {
        for (size_t k = 0; k < length2; ++k)
            out[k] = set2[k];
        return length2;
    }
    if (0 == length2) {
        for (size_t k = 0; k < length1; ++k)
            out[k] = set1[k];
        return length1;
    }
    while (true) {
        if (set1[k1] < set2[k2]) {
            out[pos++] = set1[k1];
            ++k1;
            if (k1 >= length1) {
                for (; k2 < length2; ++k2)
                    out[pos++] = set2[k2];
                break;
            }
        } else if (set1[k1] == set2[k2]) {
            out[pos++] = set1[k1];
            ++k1;
            ++k2;
            if (k1 >= length1) {
                for (; k2 < length2; ++k2)
                    out[pos++] = set2[k2];
                break;
            }
            if (k2 >= length2) {
                for (; k1 < length1; ++k1)
                    out[pos++] = set1[k1];
                break;
            }
        } else {// if (set1[k1]>set2[k2]) {
            out[pos++] = set2[k2];
            ++k2;
            if (k2 >= length2) {
                for (; k1 < length1; ++k1)
                    out[pos++] = set1[k1];
                break;
            }
        }
    }
    return pos;
    }

#endif /* UNION_H_ */
