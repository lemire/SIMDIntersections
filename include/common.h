/**
 * This code is released under the
 * Apache License Version 2.0 http://www.apache.org/licenses/.
 *
 * (c) Daniel Lemire, http://lemire.me/en/
 */
#ifndef COMMON_H_
#define COMMON_H_

#include <errno.h>
#include <fcntl.h>
#include <immintrin.h>
#include <iso646.h>
#include <limits.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>

#include <algorithm>
#include <cmath>
#include <cassert>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <numeric>
#include <queue>
#include <set>
#include <string>
#include <stdexcept>
#include <sstream>
//#include <tr1/memory>
//#include <tr1/unordered_set>
#include <unordered_set>
#include <functional>
#include <vector>

#define _LIKELY(x) __builtin_expect(!!(x), 1)
#define _UNLIKELY(x) __builtin_expect(!!(x), 0)
#define _NOINLINE __attribute__((noinline))
#define _ALWAYSINLINE __attribute__((always_inline))
typedef std::set<std::vector<uint32_t>,
		std::function<
				bool(const std::vector<uint32_t>&, const std::vector<uint32_t>&)>>mySet;

#endif /* COMMON_H_ */
