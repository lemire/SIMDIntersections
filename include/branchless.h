
#ifndef BRANCHLESS_H_
#define BRANCHLESS_H_

#include <stdint.h>
#include <stddef.h>

/**
 * Failed (?) attempt at reproducing the good results of the branchless scheme
 * from Fast Sorted-Set Intersection using SIMD Instructions
 * originally by D. Lemire but combined with a design by N. Kurz.
 */
#ifndef __INTEL_COMPILER
__attribute__((optimize("unroll-loops"))) // this helps a lot with GCC
#endif
size_t branchlessintersection(const uint32_t * set1, const size_t length1,
        const uint32_t * set2, const size_t length2, uint32_t * out) {
    if ((0 == length1) or (0 == length2))
        return 0;
    const uint32_t * const initout(out);
    const uint32_t * const finalset1(set1 + length1);
    const uint32_t * const finalset2(set2 + length2);

    const unsigned int N = 4;

    // main loop
    while ((set1 +N <= finalset1) && (set2 +N <= finalset2)) {
#ifdef __INTEL_COMPILER
#pragma unroll(4)
#endif
        for (unsigned int k = 0; k < N; ++k) {
            // this is branchless... (in theory, maybe not in practice)
            const uint32_t a = *set1;
            const uint32_t b = *set2;
            *out = a;
            out = (a == b) ? out + 1 : out;
            set1 = (a <= b) ? set1 + 1 : set1;
            set2 = (b <= a) ? set2 + 1 : set2;
        }

    }
    while ((set1  < finalset1) && (set2 < finalset2)) {
            // this is branchless... (in theory, maybe not in practice)
            *out = *set1;
            const uint32_t a = *set1;
            const uint32_t b = *set2;
            out = (a == b) ? out + 1 : out;
            set1 = (a <= b) ? set1 + 1 : set1;
            set2 = (b <= a) ? set2 + 1 : set2;

    }

    return (out - initout);
}


/**
 * Branchless approach by N. Kurz.
 */
size_t scalar_branchless(const uint32_t *A, size_t lenA,
                         const uint32_t *B, size_t lenB,
                         uint32_t *Match) {

    const uint32_t *initMatch = Match;
    const uint32_t *endA = A + lenA;
    const uint32_t *endB = B + lenB;

    while (A < endA && B < endB) {
        int m = (*B == *A) ? 1 : 0;  // advance Match only if equal
        int a = (*B >= *A) ? 1 : 0;  // advance A if match or B ahead
        int b = (*B <= *A) ? 1 : 0;  // advance B if match or B behind

        *Match = *A;   // write the result regardless of match
        Match += m;    // but will be rewritten unless advanced
        A += a;
        B += b;
    }

    size_t count = Match - initMatch;
    return count;
}

size_t scalar_branchless_cached(const uint32_t *A, size_t lenA, 
                                const uint32_t *B, size_t lenB,
                                uint32_t *Match) {

    const uint32_t *initMatch = Match;
    const uint32_t *endA = A + lenA;
    const uint32_t *endB = B + lenB;

    uint32_t thisA = A[0];
    uint32_t thisB = B[0];

    while (A < endA && B < endB) {
        uint32_t nextA = A[1];
        uint32_t nextB = B[1];

        uint32_t oldA = thisA;
        uint32_t oldB = thisB;

        int m = (oldB == oldA) ? 1 : 0;  // advance Match only if equal
        int a = (oldB >= oldA) ? 1 : 0;  // advance A if match or B ahead
        int b = (oldB <= oldA) ? 1 : 0;  // advance B if match or B behind

        thisA = (oldB >= oldA) ? nextA : thisA;  // advance A if match or B ahead
        thisB = (oldB <= oldA) ? nextB : thisB;  // advance B if match or B behind

        *Match = *A;   // write the result regardless of match
        Match += m;    // but will be rewritten unless advanced
        A += a;        
        B += b;        

    }

    size_t count = Match - initMatch; 
    return count; 
}

// use in function below
#define BRANCHLESSMATCH() {                     \
        int m = (*B == *A) ? 1 : 0;             \
        int a = (*B >= *A) ? 1 : 0;             \
        int b = (*B <= *A) ? 1 : 0;             \
        *Match = *A;                            \
        Match += m;                             \
        A += a;                                 \
        B += b;                                 \
    }


/**
 * Unrolled branchless approach by N. Kurz.
 */
size_t scalar_branchless_unrolled(const uint32_t *A, size_t lenA,
                                  const uint32_t *B, size_t lenB,
                                  uint32_t *Match) {

    const size_t UNROLLED = 4;

    const uint32_t *initMatch = Match;
    const uint32_t *endA = A + lenA;
    const uint32_t *endB = B + lenB;

    if (lenA >= UNROLLED && lenB >= UNROLLED) {
        const uint32_t *stopA = endA - UNROLLED;
        const uint32_t *stopB = endB - UNROLLED;

        while (A < stopA && B < stopB) {
            BRANCHLESSMATCH();  // NOTE: number of calls must match UNROLLED
            BRANCHLESSMATCH();
            BRANCHLESSMATCH();
            BRANCHLESSMATCH();
        }
    }

    // Finish remainder without overstepping
    while (A < endA && B < endB) {
        BRANCHLESSMATCH();
    }

    size_t count = Match - initMatch;
    return count;
}

#undef BRANCHLESSMATCH


#if INTEL_DISASSEMBLY
  15:   mov    (%rdx),%r11d  # r11 = *B
  18:   mov    $0x1,%r8d     # r8 = 1
  1e:   mov    (%rdi),%eax   # eax = *A
  20:   cmp    %eax,%r11d    # *B <=> *A
  23:   mov    $0x0,%r11d    # r11 = 0
  29:   cmove  %r8,%r11      # if *B == *A r11 = 1
  2d:   mov    %eax,(%r9)    # *output = *A
  30:   lea    (%r9,%r11,4),%r9  # output += 4 * r11
  34:   mov    $0x0,%r11d    # r11 = 0
  3a:   cmovae %r8,%r11      # if *B >= *A r11 = 1
  3e:   lea    (%rdi,%r11,4),%rdi  # A += 4 * r11
  42:   mov    $0x0,%r11d    # r11 = 0
  48:   cmovbe %r8,%r11      # if *B <= *A r11 = 1
  4c:   lea    (%rdx,%r11,4),%rdx  # B += 4 * r11
#endif


#endif
