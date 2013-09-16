Implement galloping swapping svs

swapping svs [10] finds values from the list with the smaller number of remaining integers in the other list, thus potentially swapping the roles of the lists.

E. Demaine, A. Lo ́pez-Ortiz, and J. Ian Munro.
Experiments on adaptive set intersections for text retrieval systems. Algorithm Engineering and Experimentation (ALENEX), pages 91–104, 2001.

The Baeza Yates algorithm [2] finds the median value of the smaller list in the larger list, splits the lists and recurses. Adding matches to the list at the end of the recursion [3] produces a sorted result list. The adaptive binary forward search uses binary search within the recursed list bound- aries, rather than using the original list boundaries.

[2] R. Baeza-Yates. A fast set intersection algorithm for sorted sequences. In Combinatorial Pattern Matching, pages 400–408. Springer, 2004.
