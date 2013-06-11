#include <stdint.h>
#include <stddef.h>

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

// CMP A, B
// if (==) thisA = 


#define BRANCHLESSMATCH() {                     \
        int m = (*B == *A) ? 1 : 0;             \
        int a = (*B >= *A) ? 1 : 0;             \
        int b = (*B <= *A) ? 1 : 0;             \
        *Match = *A;                            \
        Match += m;                             \
        A += a;                                 \
        B += b;                                 \
    }

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


