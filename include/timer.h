/**
 * This code is released under the
 * Apache License Version 2.0 http://www.apache.org/licenses/.
 *
 */

#ifndef TIMER_H_
#define TIMER_H_

#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

class WallClockTimer {
public:
	struct timeval t1, t2;
	WallClockTimer() :
		t1(), t2() {
		gettimeofday(&t1, 0);
		t2 = t1;
	}
	void reset() {
		gettimeofday(&t1, 0);
		t2 = t1;
	}
	uint64_t elapsed() {
		return ((t2.tv_sec - t1.tv_sec) * 1000ULL * 1000ULL) + ((t2.tv_usec
				- t1. tv_usec));
	}
	uint64_t split() {
		gettimeofday(&t2, 0);
		return elapsed();
	}
};

#endif /* TIMER_H_ */
