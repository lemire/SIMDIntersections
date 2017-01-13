/*
 * multiSetIntersection.cpp
 * scalar algorithms for intersecting multiple sets.
 * Reference:
 * (1)	Culpepper, J. S., & Moffat, A. (2010).
 *  	Efficient set intersection for inverted indexing.
 *   	Acm Transactions on Information Systems, 29(1), 1.
 *
 * (2)	Barbay, J., L¨®pez-Ortiz, A., Lu, T., & Salinger, A.
 *		(2009). An experimental investigation of set
 *		intersection algorithms for text searching.
 *		Journal of Experimental Algorithmics, 14.
 *
 * (3)	Baezayates, R., & Salinger, A. (2010).
 *		Fast Intersection Algorithms for Sorted Sequences.
 *		Algorithms and Applications. Springer Berlin Heidelberg.
 *
 * (4)	Barbay, J., L¨®pezortiz, A., & Lu, T. (2006).
 *		Faster Adaptive Set Intersections for Text Searching.
 *		International Conference on Experimental Algorithms
 *		(Vol.4007, pp.146-157). Springer-Verlag.
 *
 *  Created on: 2016/12/20
 *      Author: SparkleXS
 */

#include <multiSetIntersection.hpp>

void msis::small_vs_small(const mySet &sets, std::vector<uint32_t> &out) {
	mySet::iterator it = sets.begin();
	vector<uint32_t> intersection(std::move(*it++));
	for (; it != sets.end(); it++) {
		// here we can change the intersection function to any regular scalar
		// or vector pair-set intersection algorithms.
		size_t inter_length = BSintersection(intersection.data(),
				intersection.size(), it->data(), it->size(),
				intersection.data());
		intersection.resize(inter_length);
	}
	out.swap(intersection);
}

// without swap
void msis::set_vs_set(const mySet &sets, std::vector<uint32_t> &out) {
	size_t count = 0, currentset = 0;
	out.resize(sets.begin()->size());

	mySet::iterator it = sets.begin();
	vector<uint32_t> candidate(std::move(*it++));
	const mySet::iterator it_start = it;

	auto value = out.begin();
	auto eliminator = candidate.begin();
	std::vector<size_t> index(sets.size() - 1, 0);

	while (eliminator != candidate.end()) {
		index[currentset] = binarySearch_wider(it->data(), index[currentset],
				it->size() - 1, *eliminator);
		if (it->at(index[currentset]) == *eliminator) {
			if (++currentset == sets.size() - 1) {
				*value++ = *eliminator;
				count++;
				// roll over
				currentset = 0;
				it = it_start;
				eliminator++;
			} else {
				++it;
			}
		} else {
			currentset = 0;
			it = it_start;
			eliminator++;
		}
	}
	out.resize(count);
}

void msis::swapping_set_vs_set(const mySet& sets, vector<uint32_t>& out) {
	std::vector<std::pair<mySet::iterator, size_t /*current index*/>> vsets;
	out.resize(sets.begin()->size());

	auto sort_remaining = [&] {std::sort(vsets.begin(), vsets.end(),
				[](const std::pair<mySet::iterator, size_t>& lhs,
						const std::pair<mySet::iterator, size_t>& rhs) {
					return (lhs.first->size() - lhs.second) <
					(rhs.first->size() - rhs.second);
				});};

	for (auto it = sets.begin(); it != sets.end(); it++)
		vsets.emplace_back(it, 0);

	auto value = out.begin();
	size_t count = 0, currentset = 1;

	while (vsets[0].second != vsets[0].first->size()) {
		vsets[currentset].second = binarySearch_wider(
				vsets[currentset].first->data(), vsets[currentset].second,
				vsets[currentset].first->size() - 1,
				vsets[0].first->at(vsets[0].second));

		if (_LIKELY(
				vsets[currentset].first->at(vsets[currentset].second)
						> vsets[0].first->at(vsets[0].second))) {
			// greater
			vsets[0].second++;
			sort_remaining();
			currentset = 1;
		} else if (vsets[currentset].first->at(vsets[currentset].second)
				== vsets[0].first->at(vsets[0].second)) {
			// equal
			vsets[currentset].second++;
			if (++currentset == vsets.size()) {
				*value++ = vsets[0].first->at(vsets[0].second);
				count++;
				vsets[0].second++;
				// roll over
				sort_remaining();
				currentset = 1;
			}
		} else {
			// less
			break;
		}
	}
	out.resize(count);
}

void msis::adaptive(const mySet &sets, std::vector<uint32_t> &out) {
	out.resize(sets.begin()->size());
	auto value = out.begin();
	*value = sets.begin()->at(0);
	size_t count = 0, spansize = 1, elimset = 0, currentset = 1;
	mySet::iterator it = sets.begin();
	it++;
	std::vector<size_t> index(sets.size(), 0);

	// Here we do not adopt the following conditional statement
	// "while (index[currentset] <= it->size() - 1 || it->back() >= *value)"
	// because one condition is indispensable inside the loop, assign it
	// on the precise position will reduce trivial repetitive judgements.
	while (true) {
		if (it->at(index[currentset] + spansize) >= *value) {
			//binary search
			index[currentset] = binarySearch_wider(it->data(),
					index[currentset], index[currentset] + spansize, *value);
			if (it->at(index[currentset]) == *value) {
				// found
				index[currentset]++;
				currentset = ++currentset % sets.size();
				it = ++it == sets.end() ? sets.begin() : it;
				if (currentset == elimset) {
					count++;
					// update eliminator and move to next set
					if (_UNLIKELY(index[currentset] == it->size() - 1))
						break;
					*++value = it->at(++index[currentset]);

					currentset = ++currentset % sets.size();
					it = ++it == sets.end() ? sets.begin() : it;
				}
			} else {
				// not found
				*value = it->at(index[currentset]);
				index[elimset]++;
				elimset = currentset;

				currentset = ++currentset % sets.size();
				it = ++it == sets.end() ? sets.begin() : it;
			}
			// here spansize is set to 0 to compare the last
			// element of the current set
			if (_LIKELY(index[currentset] + 1 < it->size()))
				spansize = 1;
			else if (index[currentset] + 1 == it->size())
				spansize = 0;
			else
				break;
			continue;
		} else if (_UNLIKELY(it->back() < *value))
			break;
		spansize =
				index[currentset] + (spansize << 1) < it->size() ?
						spansize << 1 : it->size() - index[currentset] - 1;
	}
	out.resize(count);
}

void msis::sequential(const mySet &sets, std::vector<uint32_t> &out) {
	out.resize(sets.begin()->size());
	auto value = out.begin();
	*value = sets.begin()->at(0);
	size_t count = 0, elimset = 0, currentset = 1;
	mySet::iterator it = sets.begin();
	mySet::iterator it_elim = it++;
	std::vector<size_t> index(sets.size(), 0);

	while (_LIKELY(index[currentset] < it->size())) {
		index[currentset] = gallopping(it->data(), index[currentset],
				it->size() - 1, *value);
		if (it->at(index[currentset]) == *value) {
			index[currentset]++;
			currentset = ++currentset % sets.size();
			it = ++it == sets.end() ? sets.begin() : it;
			if (currentset == elimset) {
				count++;
				// update eliminator and move to next set
				if (_UNLIKELY(++index[currentset] == it->size()))
					break;
				*++value = it->at(index[currentset]);

				currentset = ++currentset % sets.size();
				it = ++it == sets.end() ? sets.begin() : it;
			}
		} else if (_UNLIKELY(it->back() < *value)) {
			break;
		} else if (_UNLIKELY(++index[elimset] == it_elim->size()))
			break;
		else {
			// greater, however reaches the end.
			// XXX: following two sentences unreachable
//			if (_UNLIKELY(index[currentset] + 1 > it->size()))
//				break;
			// greater and doesn't reach the end
			// go on intersecting
			*value = it->at(index[currentset]);
			elimset = currentset;
			it_elim = it;

			currentset = ++currentset % sets.size();
			it = ++it == sets.end() ? sets.begin() : it;
		}
	}
	out.resize(count);
}

void msis::small_adaptive(const mySet &sets, std::vector<uint32_t> &out) {
	std::vector<std::pair<mySet::iterator, size_t /*current index*/>> vsets;
	out.resize(sets.begin()->size());

	auto sort_remaining = [&] {std::sort(vsets.begin(), vsets.end(),
				[](const std::pair<mySet::iterator, size_t>& lhs,
						const std::pair<mySet::iterator, size_t>& rhs) {
					return (lhs.first->size() - lhs.second) <
					(rhs.first->size() - rhs.second);
				});};

	for (auto it = sets.begin(); it != sets.end(); it++)
		vsets.emplace_back(it, 0);

	auto value = out.begin();
	size_t count = 0, spansize = 1, currentset = 1;

	while (vsets[0].second != vsets[0].first->size()) {
		if (vsets[currentset].first->at(vsets[currentset].second + spansize)
				>= vsets[0].first->at(vsets[0].second)) {
			// gallopping overshot
			vsets[currentset].second = binarySearch_wider(
					vsets[currentset].first->data(), vsets[currentset].second,
					vsets[currentset].second + spansize,
					vsets[0].first->at(vsets[0].second));

			if (vsets[currentset].first->at(vsets[currentset].second)
					== vsets[0].first->at(vsets[0].second)) {
				// find the candidate
				vsets[currentset].second++;
				if (++currentset == vsets.size()) {
					// verify the full intersection
					*value++ = vsets[0].first->at(vsets[0].second);
					count++;
					vsets[0].second++;
					// roll over
					sort_remaining();
					currentset = 1;
				}
			} else {
				// doesn't find the candidate
				// reorder the sets and update candidate
//				vsets[currentset].second++;
				vsets[0].second++;
				sort_remaining();
				currentset = 1;
			}
			// here spansize is set to 0 to compare the last
			// element of the current set
			if (_LIKELY(
					vsets[currentset].second + 1
							< vsets[currentset].first->size()))
				spansize = 1;
			else if (vsets[currentset].second + 1
					== vsets[currentset].first->size())
				spansize = 0;
			else
				break;
			continue;
		} else if (_UNLIKELY(
				vsets[currentset].first->back()
						< vsets[0].first->at(vsets[0].second)))
			break;
		spansize =
				vsets[currentset].second + (spansize << 1)
						< vsets[currentset].first->size() ?
						spansize << 1 :
						vsets[currentset].first->size()
								- vsets[currentset].second - 1;
	}
	out.resize(count);
}

void msis::max(const mySet &sets, std::vector<uint32_t> &out) {
	out.resize(sets.begin()->size());
	auto value = out.begin();
	*value = sets.begin()->at(0);
	size_t count = 0, intersect_count = 0, elimset = 0, currentset = 1;
	mySet::iterator it = sets.begin();
	mySet::iterator it_start = it++, it_elim = it_start;
	std::vector<size_t> index(sets.size(), 0);

	while (_LIKELY(index[currentset] < it->size())) {
		index[currentset] = gallopping(it->data(), index[currentset],
				it->size() - 1, *value);
		if (it->at(index[currentset]) == *value) {
			intersect_count++;
			index[currentset]++;
			++currentset;
			++it;
//			currentset = ++currentset % sets.size();
//			it = ++it == sets.end() ? sets.begin() : it;
			if (intersect_count == sets.size() - 1) {
				// ensure positions of all the sets advance 1
				index[elimset]++;
				count++;
				// update eliminator from set 0 and move to set 1
				if (_UNLIKELY(index[0] == it_start->size()))
					break;
				intersect_count = 0;
				*++value = it_start->at(index[0]);
				currentset = 1;
				elimset = 0;
				it_elim = it = it_start;
				it++;
			} else if (currentset == elimset) {
				++currentset;
				++it;
//				currentset = ++currentset % sets.size();
//				it = ++it == sets.end() ? sets.begin() : it;
			}
		} else if (_UNLIKELY(it->back() < *value)) {
			break;
		} else if (_UNLIKELY(++index[elimset] == it_elim->size()))
			break;
		else {
			intersect_count = 0;
			if (currentset == 0
					|| it_start->at(index[0]) > it->at(index[currentset])) {
				*value = it_start->at(index[0]);
				elimset = 0;
				currentset = 1;

				it_elim = it = it_start;
				it++;
			} else {
				*value = it->at(index[currentset]);
				elimset = currentset;
				currentset = 0;

				it_elim = it;
				it = it_start;
			}
		}
	}
	out.resize(count);
}

// further optimization can be search @freq[0] and @freq[freq_end] in @rare,
// which helps target the real overlap of @rare and @freq
void msis::BYintersect_sorted(const uint32_t *freq, const size_t &freq_end,
		const uint32_t *rare, const size_t &rare_end, uint32_t **out,
		uint32_t &count) {
	if (_LIKELY(
			freq_end == -1 || rare_end == -1 || freq[0] > rare[rare_end]
					|| rare[0] > freq[freq_end]))
		return;
	else {
		size_t rare_mid = rare_end / 2;
		size_t freq_mid = msis::binarySearch_wider(freq, 0, freq_end,
				rare[rare_mid]);
		if (freq_mid > rare_mid)
			BYintersect_sorted(freq, freq_mid - 1, rare, rare_mid - 1, out,
					count);
		else
			BYintersect_sorted(rare, rare_mid - 1, freq, freq_mid - 1, out,
					count);
		if (freq[freq_mid] == rare[rare_mid++]) {
			*(*out)++ = freq[freq_mid++];
			count++;
		}
//		if (_UNLIKELY(freq_mid == freq_end || rare_mid == rare_end)) {
//			if (freq[freq_mid] == rare[rare_mid]) {
//				*(*out)++ = freq[freq_mid];
//				i++;
//			}
//		} else {
		if (freq_end - freq_mid > rare_end - rare_mid)
			BYintersect_sorted(freq + freq_mid, freq_end - freq_mid,
					rare + rare_mid, rare_end - rare_mid, out, count);
		else
			BYintersect_sorted(rare + rare_mid, rare_end - rare_mid,
					freq + freq_mid, freq_end - freq_mid, out, count);
//		}
	}
}

void msis::BaezaYates(const mySet &sets, std::vector<uint32_t> &out) {
	mySet::iterator it = sets.begin();
	vector<uint32_t> intersection(std::move(*it++));

	uint32_t* out_init = intersection.data();
	uint32_t** pout = &out_init;

	for (; it != sets.end(); it++) {
		uint32_t count = 0;

		BYintersect_sorted(it->data(), it->size() - 1, intersection.data(),
				intersection.size() - 1, pout, count);
		intersection.resize(count);

		out_init = intersection.data();
		pout = &out_init;
	}
	out.swap(intersection);
}

int main() {
	using namespace msis;

	uint32_t logMinLength = 14; // log of minimal array size
	uint32_t MaxBit = 31; // largest bit-length of element
	const uint32_t minlength = 1U << logMinLength;

	ClusteredDataGenerator cdg;
	WallClockTimer timer;
	mySet MultiSets;
	vector<uint32_t> out;

#ifdef __INTEL_COMPILER
// Intel's support for C++ sucks
	vector<float> intersectionsratios;
	intersectionsratios.push_back(1.00);
	intersectionsratios.push_back(0.80);
	intersectionsratios.push_back(0.60);
	intersectionsratios.push_back(0.20);
	intersectionsratios.push_back(0.10);
	intersectionsratios.push_back(0.05);
	intersectionsratios.push_back(0.01);
	vector < uint32_t > sizeratios;
	sizeratios.push_back(1);
	sizeratios.push_back(2);
	sizeratios.push_back(3);
	sizeratios.push_back(5);
	sizeratios.push_back(10);
	sizeratios.push_back(20);
	sizeratios.push_back(40);
	sizeratios.push_back(80);
	sizeratios.push_back(200);
	sizeratios.push_back(500);
	sizeratios.push_back(1000);
#else
// proper C++
	vector<float> intersectionsratios = { 1.00, 0.80, 0.60, 0.20, 0.10, 0.05,
			0.01 };
	vector<uint32_t> sizeratios = { 1, 2, 3, 5, 10, 20, 40, 80, 200, 500, 1000 };
#endif

	cout << "########### Intersection benchmark ###########" << endl;

	cout << "# columns are size ratios" << endl;
	cout << "# rows are intersection ratios" << endl;
// sxs: namely log(U/n)
	cout << "# average gaps in bits for smallest array: "

	<< std::setprecision(3) << log(1 + (1U << MaxBit) * 1.0 / minlength)
			<< " (use -S and -M flag to change)" << endl;
	cout << "# average gaps in bits for last largest array: "

	<< std::setprecision(3)
			<< log(1 + (1U << MaxBit) * 1.0 / (sizeratios.back() * minlength))
			<< endl;
	cout << "#############################################" << endl << endl;

//	auto MultiSets = genMultipleSets(cdg, minlength, 3, 1U << MaxBit, 1, 0.8);
//	auto it = MultiSets.begin();
//	vector<uint32_t> final_intersection = intersect(*it++, *it++);
//
//	for (; it != MultiSets.end(); it++)
//		final_intersection = intersect(final_intersection, *it);
//	small_adaptive(MultiSets, out);

	size_t time = 0;
	const int TIMES = 5;
	for (float ir : intersectionsratios) {
		printf("intersection ratio: \e[32m%3.0f%%\e[0m\n", ir * 100);
		for (uint32_t sr : sizeratios) {
			printf("  size ratio: \e[32m%4d\e[0m\n", sr);
			for (int i = 3; i < 11; i++) {
				printf("    num: \e[32m%2d\e[0m  ", i);
				// generate sets
				MultiSets = genMultipleSets(cdg, minlength, i, 1U << MaxBit,
						static_cast<float>(sr), ir);
				auto it = MultiSets.begin();

				// verification
				vector<uint32_t> final_intersection = intersect(*it++, *it++);
				for (; it != MultiSets.end(); it++)
					final_intersection = intersect(final_intersection, *it);

				// start intersection

				/*********************1************************/
				/***************small_vs_small*****************/
				/*********************1************************/
				timer.reset();
				for (int howmany = 0; howmany < TIMES; ++howmany) {
					small_vs_small(MultiSets, out);
				}
				time = timer.split();
				printf("small_vs_small: \e[31m%6.0f\e[0m  ",
						(double) time / TIMES);

				/*********************2************************/
				/*****************set_vs_set*******************/
				/*********************2************************/
				timer.reset();
				for (int howmany = 0; howmany < TIMES; ++howmany) {
					set_vs_set(MultiSets, out);
				}
				time = timer.split();
				printf("set_vs_set: \e[31m%6.0f\e[0m  ", (double) time / TIMES);

				/*********************3************************/
				/************swapping_set_vs_set***************/
				/*********************3************************/
				timer.reset();
				for (int howmany = 0; howmany < TIMES; ++howmany) {
					swapping_set_vs_set(MultiSets, out);
				}
				time = timer.split();
				printf("swapping_set_vs_set: \e[31m%6.0f\e[0m  ",
						(double) time / TIMES);

				/*********************4************************/
				/*****************adaptive*********************/
				/*********************4************************/
				timer.reset();
				for (int howmany = 0; howmany < TIMES; ++howmany) {
					adaptive(MultiSets, out);
				}
				time = timer.split();
				printf("adaptive: \e[31m%6.0f\e[0m  ", (double) time / TIMES);

				/*********************5************************/
				/*****************sequential*******************/
				/*********************5************************/
				timer.reset();
				for (int howmany = 0; howmany < TIMES; ++howmany) {
					sequential(MultiSets, out);
				}
				time = timer.split();
				printf("sequential: \e[31m%6.0f\e[0m  ", (double) time / TIMES);

				/*********************6************************/
				/***************small_adaptive*****************/
				/*********************6************************/
				timer.reset();
				for (int howmany = 0; howmany < TIMES; ++howmany) {
					small_adaptive(MultiSets, out);
				}
				time = timer.split();
				printf("small_adaptive: \e[31m%6.0f\e[0m  ",
						(double) time / TIMES);

				/*********************7************************/
				/********************max***********************/
				/*********************7************************/
				timer.reset();
				for (int howmany = 0; howmany < TIMES; ++howmany) {
					max(MultiSets, out);
				}
				time = timer.split();
				printf("max: \e[31m%6.0f\e[0m  ", (double) time / TIMES);

				/*********************8************************/
				/*****************BaezaYates*******************/
				/*********************8************************/
				timer.reset();
				for (int howmany = 0; howmany < TIMES; ++howmany) {
					BaezaYates(MultiSets, out);
				}
				time = timer.split();
				printf("BaezaYates: \e[31m%6.0f\e[0m\n", (double) time / TIMES);

				if (out != final_intersection) {
					std::cerr << "bad result!\n" << std::endl;
					return 1;
				} else
					printf("good!\n");
				fflush(stdout);
			}
		}
	}
}
