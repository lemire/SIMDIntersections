/**
 * This code is released under the
 * Apache License Version 2.0 http://www.apache.org/licenses/.
 *
 * (c) Daniel Lemire, http://lemire.me/en/
 */
#include "intersectionfactory.h"
#include "common.h"
#include "timer.h"
#include "synthetic.h"
#include "util.h"


void printusage() {
    cout << " Try ./testintersection -r 40" << endl;
    cout << " Use the -s flag to specify just some scheme, choose from: "
            << endl;
    for(string x : allNames()) cout <<" "<< x << endl;
    cout << " Separate the schemes by a comma (e.g. -s schlegel,danscalar). "<< endl;
}

int main(int argc, char **argv) {
    size_t howmany = 0;
    bool natemode = false;
    bool safe = true;
    size_t loop = 1000;
    uint32_t S = 12;
    uint32_t ratio = 1;
    map<string, cardinalityintersectionfunction> myschemes(schemes);
    map<string, cardinalityintersectionfunctionpart> mypartschemes(partschemes);
    int c;
    while ((c = getopt(argc, argv, "uns:S:m:l:r:h")) != -1)
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
        case 's':
            myschemes.clear();
            mypartschemes.clear();
            {
                const string codecsstr(optarg);
                const vector < string > codecslst = split(codecsstr, ",:;");
                for (auto i = codecslst.begin(); i != codecslst.end(); ++i) {
                    if (schemes.find(*i) == schemes.end()) {
                        if (partschemes.find(*i) == partschemes.end()) {
                            cerr << " Warning!!! Warning: unrecognized: " << *i
                                    << endl;
                            printusage();
                            return -1;
                        } else {
                            const auto K = partschemes.find(*i);
                            const std::string name = K->first;
                            const cardinalityintersectionfunctionpart fn = K->second;
                            mypartschemes[name] =  fn;
                        }
                    } else {
                        const auto K = schemes.find(*i);
                        const std::string name = K->first;
                        const cardinalityintersectionfunction fn = K->second;
                        myschemes[name] =  fn;
                    }
                }
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
    if (howmany == 0) {
        if (natemode)
            howmany = 1;
        else
            howmany = 20;
    }
    uint32_t MaxBit = 31;
    if (natemode) {
        MaxBit = S + 7;
        if (MaxBit > 31)
            MaxBit = 31;
    }
    cout<<"# algo: ";
    for(auto algo : myschemes) {
        cout << algo.first<< " ";
    }
    for(auto algo : mypartschemes) {
            cout << algo.first<< " ";
    }
    cout << endl;
    const uint32_t N = 1U << S;
    if (!natemode)
        if (ratio != 1) {
            cout << "# ratio = " << ratio << endl;
            cout << "# size of largest array = " << N << endl;
            cout << "# size of smallest array = " << N / ratio << endl;
        } else {
            cout << "# size of arrays = " << N << endl;
        }
    else
        cout << "# in natemode" << endl;

    ClusteredDataGenerator cdg;
    WallClockTimer z;
    size_t bogus = 0;
    size_t volume = 0;
    uint64_t time = 0;
    cout << "# first column is relative size of intersection" << endl;
    if (ratio > 1) {
        cout
                << "# next two are estimated average bits per int for differential coding"
                << endl;
    } else {
        cout
                << "# next is estimated average bits per int for differential coding"
                << endl;
    }

    cout
            << "# other columns display speed in mis when computing the cardinality of the intersection"
            << endl;
    for (uint32_t gap = 0; gap + S <= MaxBit; gap += 1) {
        vector < vector<uint32_t> > data;
        for (size_t zz = 0; zz < howmany; ++zz) {
            if (natemode) {
                data.push_back(
                        cdg.generateClustered((1U << (MaxBit - gap)) / ratio,
                                1U << MaxBit));
                data.push_back(
                        cdg.generateClustered((1U << (MaxBit - gap)),
                                1U << MaxBit));
            } else {
                data.push_back(
                        cdg.generateClustered(N / ratio, 1U << (gap + S)));
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

        cout << std::fixed << std::setprecision(3) << static_cast<double>(intersize)
                / static_cast<double>(smallestsize) << "\t";
        cout.flush();
        if (ratio > 1) {
            if (natemode) {
                cout << log(
                        1 + (1U << MaxBit) * 1.0 / ((1U << (MaxBit - gap))
                                / ratio)) / log(2) << "\t";
                cout
                        << log(
                                1 + (1U << MaxBit) * 1.0 / ((1U << (MaxBit
                                        - gap)))) / log(2) << "\t";
            } else {
                cout << log(1 + (1U << (gap + S)) * 1.0 / (N / ratio)) / log(2)
                        << "\t";
                cout << log(1 + (1U << (gap + S)) * 1.0 / (N)) / log(2) << "\t";
            }
        } else {
            if (natemode) {
                cout
                        << log(
                                1 + (1U << MaxBit) * 1.0 / ((1U << (MaxBit
                                        - gap)))) / log(2) << "\t";
            } else {
                cout << log(1 + (1U << (gap + S)) * 1.0 / (N)) / log(2) << "\t";
            }

        }
        cout << "\t";
        cout.flush();
        for(auto algo : myschemes) {
            if (safe and buggyschemes.find(algo.first) == buggyschemes.end() )
            for (size_t k = 0; k < 2 * howmany; k += 2) {
                size_t correctanswer = classicalintersectioncardinality(
                        &data[k][0], data[k].size(), &data[k + 1][0],
                        data[k + 1].size());
                size_t thisschemesanswer = algo.second(
                        &data[k][0], data[k].size(), &data[k + 1][0],
                        data[k + 1].size());
                if (correctanswer != thisschemesanswer) {
                    cerr << "expecting cardinality of " << correctanswer;
                    cerr << " got " << thisschemesanswer << " instead."
                    << endl;
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
                    += algo.second(&data[k][0],
                            data[k].size(), &data[k + 1][0],
                            data[k + 1].size());
                }

            }
            time = z.split();
            cout << std::setprecision(0) << static_cast<double>(volume) / static_cast<double>(time) << "\t";
            cout.flush();
        }

        /**
         * From Schlegel et al., Fast Sorted-Set Intersection using SIMD Instructions
         */

        vector < vector<uint16_t> > pdata(data.size());
        assert(pdata.size() == 2 * howmany);
        for (size_t zz = 0; zz < data.size(); ++zz) {
            pdata[zz].resize(data[zz].size() * 4);
            const size_t c = partitioned::partition(&data[zz][0],
                    data[zz].size(), &pdata[zz][0], pdata[zz].size());
            pdata[zz].resize(c);
            vector<uint16_t> (pdata[zz]).swap(pdata[zz]);
            assert(pdata[zz].size() == c);

            if (safe) {
                vector < uint32_t > testvec(data[zz].size());
                size_t recovsize = partitioned::inverse_partition(&testvec[0],
                        testvec.size(), &pdata[zz][0], pdata[zz].size());
                if (testvec.size() != recovsize)
                    throw std::runtime_error("bug");
                if (testvec != data[zz])
                    throw std::runtime_error("bug");
            }

        }
        cout << "\t";
        //for (uint32_t whichalgo = 0; whichalgo < HOWMANYPARTALGO; ++whichalgo) {
        for(auto algo : mypartschemes) {
            volume = 0;
            if (safe)
            for (size_t k = 0; k < 2 * howmany; k += 2) {
                size_t correctanswer = classicalintersectioncardinality(
                        &data[k][0], data[k].size(), &data[k + 1][0],
                        data[k + 1].size());
                size_t thisschemesanswer = algo.second(
                        &pdata[k][0], &pdata[k + 1][0], pdata[k].size(),
                        pdata[k + 1].size());
                if (correctanswer != thisschemesanswer) {
                    cerr << "expecting cardinality of " << correctanswer;
                    cerr << " got " << thisschemesanswer << " instead."
                    << endl;
                    throw runtime_error("bug");
                }

            }

            z.reset();

            for (size_t k = 0; k < 2 * howmany; k += 2) {
                volume += data[k].size();
                volume += data[k + 1].size();

                bogus += algo.second(&pdata[k][0],
                        &pdata[k + 1][0], pdata[k].size(), pdata[k + 1].size());
            }

            time = z.split();
            cout << std::setprecision(0) << static_cast<double>(volume) / static_cast<double>(time) << "\t";

            cout.flush();
        }
        cout << endl;

    }
    cout << "# bogus = " << bogus << endl;
}
