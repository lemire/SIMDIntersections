/**
 * This code is released under the
 * Apache License Version 2.0 http://www.apache.org/licenses/.
 *
 * (c) Daniel Lemire, http://lemire.me/en/
 */
#include <unistd.h>

#include "intersection.h"
#include "common.h"
#include "util.h"
#include "timer.h"
#include "synthetic.h"

void printusage() {
	cout << " Try ./testintersection -r 40" << endl;
}

int main(int argc, char **argv) {
	size_t howmany = 0;
	bool natemode = false;
	bool safe = true;
	size_t loop = 1000;
	uint32_t S = 12;
	uint32_t ratio = 1;
	int c;
	while ((c = getopt(argc, argv, "unS:m:l:r:h")) != -1)
		switch (c) {
		case 'u':
			safe = false;
			break;

		case 'n':
			natemode = true;
			break;
		case 'h':
			printusage();
			return 0;
		case 'S':
			S = atoi(optarg);
			if ((S < 1) or (S > 31)) {
				printusage();
				return -1;
			}
			break;
		case 'm':
			howmany = atoi(optarg);
			if (howmany < 1) {
				printusage();
				return -1;
			}
			break;
		case 'l':
			loop = atoi(optarg);
			if (loop < 1) {
				printusage();
				return -1;
			}
			break;
		case 'r':
			ratio = atoi(optarg);
			if (ratio < 1) {
				printusage();
				return -1;
			}
			break;
		default:
			abort();
		}
        if(howmany == 0) {
          if(natemode) howmany = 1; else howmany = 20;
        }
	uint32_t MaxBit = 31;
	if(natemode) {
		MaxBit = S+7;
		if(MaxBit>31) MaxBit = 31;
	}

	const uint32_t N = 1U << S;
	if (!natemode)
		if (ratio != 1) {
			cout << "# ratio = " << ratio << endl;
			cout << "# size of largest array = " << N << endl;
			cout << "# size of smallest array = " << N / ratio << endl;
		} else {
			cout << "# size of arrays = " << N << endl;
		}
	else cout <<"# in natemode" << endl;

	ClusteredDataGenerator cdg;
	WallClockTimer z;
	size_t bogus = 0;
	size_t volume = 0;
	uint64_t time = 0;
	cardinalityintersectionfunction mycardinterfunc[] = {
			danielshybridintersectioncardinality,
			widevector2_cardinality_intersect,
			intersectioncardinality,
			frogintersectioncardinality,
			onesidedgallopingintersectioncardinality,
			classicalintersectioncardinality,
			highlyscalablewordpresscom::cardinality_intersect_scalar,
			highlyscalablewordpresscom::cardinality_intersect_SIMD,
			highlyscalablewordpresscom::opti_cardinality_intersect_SIMD,
			highlyscalablewordpresscom::opti2_cardinality_intersect_SIMD,
                        nate_count_medium,
                        nate2_count_medium,
                        nate3_count_medium,
			 };
	cardinalityintersectionfunctionpart mypartcardinterfunc[] = {
			highlyscalablewordpresscom::cardinality_intersect_partitioned,
			highlyscalablewordpresscom::faster2_cardinality_intersect_partitioned
	};
	cout<<"# "<<endl;
	cout<<"#schemes are : "<<endl;
	cout<<"# Daniel's super hyper hybrid "<<endl;
	cout<<"# Daniel's 'widevector' SIMD merge (256-bit) "<<endl;
	cout<<"# Daniel's scalar merge "<<endl;
	cout<<"# 'frog' (galloping) scalar merge "<<endl;
	cout<<"# 'frog' (galloping) scalar merge -- one-sided "<<endl;
	cout<<"# textbook scalar merge (Daniel's version) "<<endl;
	cout<<"# another textbook scalar merge "<<endl;
	cout<<"# SIMD taken from web "<<endl;
	cout<<"# Daniel's version of the SIMD taken from web "<<endl;
	cout<<"# Daniel's version of the SIMD taken from web (2) "<<endl;
	cout<<"# Nate's medium range SSE for -r 20 to -r 200"<<endl;
	cout<<"# Hacked version of Nate's by D. Lemire (1)"<<endl;
	cout<<"# Hacked version of Nate's by D. Lemire (2)"<<endl;

	cout<<"# Something from a paper by Intel guys (*not* over sorted arrays) "<<endl;
	cout<<"# Daniel's version of the previous scheme (*not* over sorted arrays) "<<endl;
	cout<<"# "<<endl;
	const size_t HOWMANYALGO = 13;
	const size_t HOWMANYPARTALGO = 2;
	cout << "# first column is relative size of intersection" << endl;
	if(ratio > 1) {
		cout << "# next two are estimated average bits per int for differential coding"<<endl;
	} else {
		cout << "# next is estimated average bits per int for differential coding"<<endl;
	}

	cout
			<< "# other columns display speed in mis when computing the cardinality of the intersection"
			<< endl;
	//	cout << "# schemes are dan's, frog, binary search, classical,hsclassical, simd" << endl;
	for (uint32_t gap = 0; gap + S <= MaxBit; gap += 1) {
		vector < vector<uint32_t> > data;
		for (size_t zz = 0; zz < howmany; ++zz) {
			if(natemode) {
				data.push_back(cdg.generateClustered((1U << (MaxBit-gap)) / ratio, 1U << MaxBit));
				data.push_back(cdg.generateClustered((1U << (MaxBit-gap)), 1U << MaxBit));
			} else {
				data.push_back(cdg.generateClustered(N / ratio, 1U << (gap + S)));
				data.push_back(cdg.generateClustered(N, 1U << (gap + S)));
			}
		}
		size_t intersize = 0;
		size_t smallestsize = 0;
		for (size_t k = 0; k < howmany; k++) {
			intersize += classicalintersectioncardinality(&data[2 * k][0],
					data[2 * k].size(), &data[2 * k + 1][0],
					data[2 * k + 1].size());
			smallestsize
					+= data[2 * k + 1].size() < data[2 * k].size() ? data[2 * k
							+ 1].size() : data[2 * k].size();
		}

		cout << std::fixed << std::setprecision(3) << intersize * 1.0
				/ smallestsize << "\t";
		cout.flush();
		if(ratio > 1) {
			if(natemode) {
				cout<< log( 1+(1U << MaxBit) * 1.0 /((1U << (MaxBit-gap)) / ratio) ) / log(2) <<"\t";
				cout<< log(1+(1U << MaxBit) * 1.0 /((1U << (MaxBit-gap)) )) / log(2) << "\t";
			} else {
				cout<< log(1+(1U << (gap + S)) * 1.0 /(N / ratio) ) / log(2)<<"\t";
				cout<< log(1+(1U << (gap + S)) * 1.0 /(N )) / log(2) << "\t";
			}
		} else {
			if(natemode) {
				cout<< log(1+(1U << MaxBit) * 1.0 /((1U << (MaxBit-gap)) ) ) / log(2)<< "\t";
			} else {
				cout<< log(1+(1U << (gap + S)) * 1.0 /(N )) / log(2) << "\t";
			}

		}
		cout << "\t" ;
		cout.flush();
		for (uint32_t whichalgo = 0; whichalgo < HOWMANYALGO; ++whichalgo) {
			if(safe) for (size_t k = 0; k < 2 * howmany; k += 2) {
				size_t correctanswer = classicalintersectioncardinality(&data[k][0],
						data[k].size(), &data[k + 1][0], data[k + 1].size());
				size_t thisschemesanswer = mycardinterfunc[whichalgo](&data[k][0],
						data[k].size(), &data[k + 1][0],
						data[k + 1].size());
				if(correctanswer != thisschemesanswer) {
					cerr << "expecting cardinality of "<< correctanswer;
					cerr << " got " << thisschemesanswer << " instead." <<endl;
					throw runtime_error("bug");
				}
			}
			volume = 0;
			z.reset();
			for (size_t L = 0; L < loop; ++L) {

				for (size_t k = 0; k < 2 * howmany; k += 2) {
					volume += data[k].size();
					volume += data[k + 1].size();
					bogus
							+= mycardinterfunc[whichalgo](&data[k][0],
									data[k].size(), &data[k + 1][0],
									data[k + 1].size());
				}

			}
			time = z.split();
			cout << std::setprecision(0) << volume * 1.0 / (time) << "\t";
			cout.flush();
		}

		/**
		 * From Schlegel et al., Fast Sorted-Set Intersection using SIMD Instructions
		 */

		vector < vector<uint16_t> > pdata(data.size());
		assert(pdata.size() == 2 * howmany);
		for (size_t zz = 0; zz < data.size(); ++zz) {
			pdata[zz].resize(data[zz].size() * 4);
			const size_t c = highlyscalablewordpresscom::partition(
					&data[zz][0], data[zz].size(), &pdata[zz][0],
					pdata[zz].size());
			pdata[zz].resize(c);
			vector<uint16_t> (pdata[zz]).swap(pdata[zz]);
			assert(pdata[zz].size() == c);

			if (safe) {
				vector < uint32_t > testvec(data[zz].size());
				size_t recovsize =
						highlyscalablewordpresscom::inverse_partition(
								&testvec[0], testvec.size(), &pdata[zz][0],
								pdata[zz].size());
				if (testvec.size() != recovsize)
					throw std::runtime_error("bug");
				if (testvec != data[zz])
					throw std::runtime_error("bug");
			}

		}
		cout <<"\t";
		for (uint32_t whichalgo = 0; whichalgo < HOWMANYPARTALGO; ++whichalgo) {

			volume = 0;
			if (safe)
			for (size_t k = 0; k < 2*howmany; k+=2) {
				size_t correctanswer = classicalintersectioncardinality(&data[k][0],
						data[k].size(), &data[k + 1][0], data[k + 1].size());
				size_t thisschemesanswer = mypartcardinterfunc[whichalgo](
						&pdata[ k][0], &pdata[ k + 1][0],
						pdata[ k].size(), pdata[ k + 1].size());
				if(correctanswer != thisschemesanswer) {
					cerr << "expecting cardinality of "<< correctanswer;
					cerr << " got " << thisschemesanswer << " instead." <<endl;
					throw runtime_error("bug");
				}

			}

			z.reset();

			for (size_t k = 0; k < 2*howmany; k+=2) {
				volume += data[k].size();
				volume += data[k + 1].size();

				bogus
						+= mypartcardinterfunc[whichalgo](
								&pdata[k][0], &pdata[k + 1][0],
								pdata[k].size(), pdata[k + 1].size());
			}

			time = z.split();
			cout << std::setprecision(0) << volume * 1.0 / (time) << "\t";

			cout.flush();
		}
		cout << endl;

	}
	cout << "# bogus = " << bogus << endl;
}
