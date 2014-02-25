/*
 * This is not the totality of our testing, but rather, this is meant to include
 * very specific tests.
 *
 * See testintersection.cpp for more general tests.
 *
 */

// todo: add tests on large numbers

#include "common.h"
#include "intersectionfactory.h"

int test2(intersectionfunction f) {
    const uint32_t firstpost[5] = { 12635, 12921, 12923, 12924,
            12926};

    const uint32_t secondpost[173] = { 3756, 11996, 12044, 12049, 12109, 12128,
            12131, 12141, 12142, 12150, 12154, 12160, 12167, 12168, 12172,
            12177, 12201, 12208, 12215, 12216, 12223, 12228, 12232, 12233,
            12234, 12235, 12236, 12240, 12241, 12242, 12243, 12254, 12255,
            12256, 12257, 12259, 12260, 12261, 12262, 12264, 12265, 12266,
            12275, 12295, 12471, 12482, 12486, 12508, 12509, 12510, 12511,
            12512, 12530, 12536, 12572, 12573, 12589, 12607, 12609, 12611,
            12630, 12631, 12632, 12633, 12634, 12635, 12636, 12653, 12655,
            12657, 12668, 12672, 12685, 12702, 12716, 12721, 12741, 12745,
            12750, 12755, 12757, 12761, 12765, 12767, 12768, 12794, 12802,
            12803, 12823, 12842, 12851, 12871, 12891, 12893, 12894, 12895,
            12896, 12897, 12915, 12917, 12918, 12919, 12920, 12921, 12922,
            12923, 12924, 12925, 12927, 12929, 12932, 12933, 12934, 12935,
            12936, 12937, 12938, 12939, 12942, 12946, 12951, 12955, 12963,
            12972, 13011, 13013, 13014, 13015, 13017, 13032, 13033, 13036,
            13042, 13050, 13051, 13052, 13057, 13058, 13060, 13090, 13120,
            13132, 13136, 13147, 13185, 13191, 13192, 13193, 13194, 13195,
            13198, 13202, 13205, 13219, 13228, 13230, 13232, 13233, 13238,
            13240, 13246, 13248, 13277, 13278, 13281, 13282, 13283, 13284,
            13291, 13320, 13338, 13346, 13347 };
    vector < uint32_t > inter(173);
    size_t s = f(firstpost, 5, secondpost, 173, inter.data());
    inter.resize(s);
    vector < uint32_t > correct(173);
    size_t cs = classicalintersection(firstpost, 5, secondpost, 173,
            correct.data());
    correct.resize(cs);
    if (inter != correct) {
        cout << inter.size() << " " << correct.size() << endl;
        cout<<" correct answer:"<<endl;
        for (size_t i = 0; i < correct.size(); ++i)
            cout << i << " " << correct[i] << endl;
        cout<<" bad answer:"<<endl;
        for (size_t i = 0; i < inter.size(); ++i)
            cout << i << " " << inter[i] << endl;
        return 1;
    }
    return 0;

}

int test1(intersectionfunction f, bool testwriteback) {

    const uint32_t firstpost[13] = {27181,35350,39241,39277,39278,44682,64706,120447,120450,159274,159290,173895,173942,
};
    const uint32_t secondpost[13] = {25369,28789,28790,28792,28794,28797,37750,42317,68797,68877,68881,68990,85488};
    vector < uint32_t > inter(13);
    size_t s = f(firstpost, 13, secondpost, 13, inter.data());
    inter.resize(s);
    vector < uint32_t > correct(13);
    size_t cs = classicalintersection(firstpost, 13, secondpost, 13,
            correct.data());
    correct.resize(cs);
    if (inter != correct) {
        cout << inter.size() << " " << correct.size() << endl;
        for (size_t i = 0; (i < inter.size()) && (i < correct.size()); ++i)
            cout << i << " " << inter[i] << " " << correct[i] << endl;
        return 1;
    }
    if (!testwriteback)
        return 0;
    vector < uint32_t > inter2(firstpost, firstpost + 13);
    size_t s2 = f(inter2.data(), 13, secondpost, 13, inter2.data());
    inter2.resize(s2);
    if (inter2 != correct)
        return 2;
    return 0;

}

int main() {
    int error = 0;
    for (string n : allRealNames()) {
        cout<<"testing "<<n<<" ... ";
        cout.flush();
        int code;
        bool testwriteback = (n != "hssimd") && (n != "hssimddan");
        if((code = test1(realschemes[n],testwriteback))==0)
        cout<<"ok ";
        else {
            cout<<" Error"<<code<<" ";
            ++error;
        }
        if((code = test2(realschemes[n]))==0)
        cout<<"ok"<<endl;
        else {
            cout<<" Error"<<code<<endl;
            ++error;
        }

    }
    if (error == 0)
        cout << "Your code is maybe ok." << endl;
    else
        cout << "Your code is buggy, found " << error << " error(s)" << endl;
    return 0;
}
