/*
 * Algorithm 3:  1:1 2:1 3:1 5:1 10:1 20:1 40:1 80:1 200:1 500:1 1000:1
 100%             xx  xx   xx ...
 80%               xx  xx   xx ...
 60%
 20%
 10%
 5%
 1%
 */

#include "common.h"
#include "intersectionfactory.h"
#include "timer.h"
#include "synthetic.h"
#include "util.h"

void printusage() {
    cout << " Try ./getmatrix -s scalarnate" << endl;
    cout << " Use the -s flag to specify  some scheme, choose from: " << endl;
    for(string x : allRealNames()) cout <<" "<< x << endl;
    cout
            << " The -M flag allows you to specific the range in bits (default 31)."
            << endl;
    cout
            << " The -S flag allows you to specific the log. of the minimal array size (default 10)."
            << endl;
}

int main(int argc, char **argv) {
    size_t howmany = 0;
    size_t loop = 10;
    uint32_t S = 10;
    string name;
    intersectionfunction myscheme = NULL;
    uint32_t MaxBit = 31;
    int c;
    while ((c = getopt(argc, argv, "ns:m:M:S:l:r:h")) != -1)
        switch (c) {
        case 'h':
            printusage();
            return 0;
        case 'S':
            S = atoi(optarg);
            break;
        case 'M':
            MaxBit = atoi(optarg);
            if (MaxBit < 1) {
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
            name = optarg;
            if (realschemes.find(name) == realschemes.end()) {
                cerr << " Warning!!! Warning: unrecognized: " << name << endl;
                printusage();
                return -1;

            } else {

                myscheme = realschemes.find(name)->second;
            }
            break;
        case 'l':
            loop = atoi(optarg);
            if (loop < 1) {
                printusage();
                return -1;
            }
            break;
        default:
            abort();
        }
    if (howmany == 0) {
        howmany = 5;
    }
    const uint32_t minlength = 1U << S;
    cout << "########### Intersection benchmark ###########" << endl;

    cout << "# speeds in mis" << endl;
    cout << "# columns are size ratios" << endl;
    cout << "# rows are intersection ratios" << endl;
    cout << "# average gaps in bits for smallest array: " << std::setprecision(
            3) << log(1 + (1U << MaxBit) * 1.0 / minlength) << " (use -S and -M flag to change)"<< endl;
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
    vector < uint32_t > sizeratios = {1, 2, 3, 5, 10, 20,40,80,200,500,1000};
#endif
    cout<<"# average gaps in bits for last largest array: "<<std::setprecision(3)<<log(
             1 + (1U << MaxBit) * 1.0 / (sizeratios.back()*minlength))<<endl;
    cout << "#############################################" << endl << endl;

    cout<< name << endl << "\t\t";
    ClusteredDataGenerator cdg;
    WallClockTimer z;
    size_t bogus = 0;


    for(uint32_t sr :  sizeratios) {
        cout<<"1:"<<sr << "\t";
    }
    cout<<endl;
    size_t time = 0;
    for(float ir : intersectionsratios) {
        cout<< std::setprecision(3)<< (100*ir) << "%\t\t";
        cout.flush();
        for(uint32_t sr :  sizeratios) {
            vector<uint32_t> buffer((sr*minlength + 15)/16*16);
            vector <
            pair<
            vector<uint32_t>, vector<uint32_t>
            >
            > data(howmany);
            for(size_t k = 0; k < howmany; ++k)
                data[k] = getPair(cdg, minlength,1U<<MaxBit, static_cast<float>(sr), ir);
            size_t volume = 0;
            z.reset();
            for (size_t L = 0; L < loop; ++L) {

                for (auto x : data) {
                    volume += (x.first).size();
                    volume += (x.second).size();
                    bogus
                    += myscheme(&(x.first)[0],
                            (x.first).size(), &(x.second)[0],
                            (x.second).size(),&buffer[0]);
                }

            }
            time = z.split();
            cout << std::setprecision(4) << static_cast<double>(volume) / static_cast<double>(time) << "\t";
            cout.flush();


        }
        cout<<endl;

    }

    cout << "# bogus = " << bogus << endl;
}

