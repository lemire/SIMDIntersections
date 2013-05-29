
1234    2222 3333 4444 5555 6666

abcd    bbbb cccc dddd eeee ffff

// Also low gear:  1-3 vectors (4-12) compared with 1-10/1-3 (5-15 total)

A(rare);
NUMVECS scales B(freq);

pshufd nextA -> M1, M2, M3, M4;

if A[3] < B[0]  no match possible, A++;   // assymetric, A is rare
(low gear?)

reload A[3]
load nextA:

if B[3]  < A[0]  no match possible, B++;  // non-branching cmov
if B[7]  < A[0]  no match possible, B++;
if B[11] < A[0]  no match possible, B++;  // configurable LOOKAHEADS

reload B[3], B[7], ...

cmpeq M1,vecB0;
cmpeq M2,vecB0;
cmpeq M3,vecB0;
cmpeq M4,vecB0;
load vecB0;

or results; 

cmpeq M1,vecB1;
cmpeq M2,vecB1;
cmpeq M3,vecB1;
cmpeq M4,vecB1;
load vecB1;

or results; 

...NUMVECS

movmsk, popcnt, cmov add to count;

repeat




1 2 3 4  5  6  7  8   

4 3 2 1

2 4 6 8 10 12 14 16 

A0 < A1 < A2 < A3
B0 < B1 < B2 < B3


