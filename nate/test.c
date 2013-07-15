#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "test.h"

#define TEST_PRINT(verbosity, level, args...)    \
    do { if (verbosity >= level) printf(args); } while (0)

size_t match_scalar(const uint32_t *rare, size_t lenRare,
                    const uint32_t *freq, size_t lenFreq, 
                    uint32_t *matchOut);


bool test_identical(match_func_t match_func, size_t len, int verbosity) {
                             
                               
    uint64_t lenFreq = len;
    uint64_t lenRare = len;
    uint64_t lenOut = lenFreq > lenRare ? lenFreq : lenRare;

    uint32_t freq[lenFreq];
    for (int i = 0; i < lenFreq; i++) {
        freq[i] = i;
    }

    uint32_t rare[lenRare];
    for (int i = 0; i < lenRare; i++) {
        rare[i] = i;
    }

    uint32_t out[lenOut];
    for (int i = 0; i < lenOut; i++) {
        out[i] = 0;
    }

    size_t count = (match_func)(rare, lenRare, freq, lenFreq, out);
    size_t correct = len;

    bool success = (count == correct);

    TEST_PRINT(verbosity, 1, 
               "%s - test_identical(%zd): Got %zd, expected %zd\n", 
               success ? "ok" : "FAIL", len, count, correct);
    
    if (! success) {
        for (int i = 0; i < lenOut; i++) {
            TEST_PRINT(verbosity, 1, " %d: %d\n", i, out[i]);
        }
    }

    return success;
}

bool test_half(match_func_t match_func, size_t len, int verbosity) {

    uint64_t lenFreq = len;
    uint64_t lenRare = len / 2;
    uint64_t lenOut = lenRare;

    uint32_t freq[lenFreq];
    for (int i = 0; i < lenFreq; i++) {
        freq[i] = i;
    }

    uint32_t rare[lenRare];
    for (int i = 0; i < lenRare; i++) {
        rare[i] = 2 * i;
    }

    uint32_t outForward[lenOut];
    uint32_t outReverse[lenOut];
    uint32_t outScalar[lenOut];
    for (int i = 0; i < lenOut; i++) {
        outForward[i] = 0;
        outReverse[i] = 0;
        outScalar[i] = 0;
    }

    size_t countForward = (match_func)(rare, lenRare, freq, 
                                       lenFreq, outForward);
    size_t countReverse = (match_func)(freq, lenFreq, rare, 
                                       lenRare, outReverse);
    size_t countScalar = match_scalar(rare, lenRare, freq, 
                                      lenFreq, outScalar);

    bool success = (countForward == countScalar) 
        && (countReverse == countScalar)
        && (memcmp(outForward, outScalar, countScalar) == 0) 
        && (memcmp(outReverse, outScalar, countScalar) == 0);

    TEST_PRINT(verbosity, 1, 
               "%s - test_half(%zd): Forward %zd, Reverse %zd, Scalar %zd\n", 
               success ? "ok" : "FAIL", len, 
               countForward, countReverse, countScalar);

    if (! success) {

        for (int i = 0; i < lenOut; i++) {
            TEST_PRINT(verbosity, 2, " %d: %d %d %d\n", i, outForward[i], 
                       outReverse[i], outScalar[i]);
        }
    }

    return success;
}

bool test_random(match_func_t match_func, size_t len, 
                 uint32_t increment, int verbosity) {
    
    uint64_t lenFreq = len;
    uint64_t lenRare = len / increment;
    uint64_t lenOut = lenRare;

    uint32_t freq[lenFreq];
    freq[0] = rand() % increment;
    for (int i = 1; i < lenFreq; i++) {
        freq[i] = freq[i - 1] + rand() % increment + 1; 
    }

    uint32_t rare[lenRare];
    rare[0] = rand() % increment;
    for (int i = 1; i < lenRare; i++) {
        rare[i] = rare[i - 1] + rand() % increment + 1; 
    }

    uint32_t outForward[lenOut];
    uint32_t outReverse[lenOut];
    uint32_t outScalar[lenOut];
    for (int i = 0; i < lenOut; i++) {
        outForward[i] = 0;
        outReverse[i] = 0;
        outScalar[i] = 0;
    }

    size_t countForward = (match_func)(rare, lenRare, freq, 
                                       lenFreq, outForward);
    size_t countReverse = (match_func)(freq, lenFreq, rare, 
                                       lenRare, outReverse);
    size_t countScalar = match_scalar(rare, lenRare, freq, 
                                      lenFreq, outScalar);

    bool success = (countForward == countScalar) 
        && (countReverse == countScalar)
        && (memcmp(outForward, outScalar, countScalar) == 0) 
        && (memcmp(outReverse, outScalar, countScalar) == 0);

    TEST_PRINT(verbosity, 1, 
               "%s - test_random(%zd, %d): " 
               "Forward %zd, Reverse %zd, Scalar %zd\n", 
               success ? "ok" : "FAIL", len, increment,
               countForward, countReverse, countScalar);

    if (! success) {
        for (int i = 0; i < lenOut; i++) {
            TEST_PRINT(verbosity, 2, " %d: %d %d %d\n", i, 
                       outForward[i], outReverse[i], outScalar[i]);
        }
    }

    return success;
}

bool test_all(match_func_t match_func, size_t len, int verbosity) {
    bool success = test_identical(match_func, len, verbosity);

    success &= test_half(match_func, len, verbosity);

    for (int i = 1; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            success &= test_random(match_func, len, i, verbosity);
        }
    }

    if (success) TEST_PRINT(verbosity, 1, "All tests passed.\n");
    else TEST_PRINT(verbosity, 1, "TESTS FAILED\n");

    return success;
}

