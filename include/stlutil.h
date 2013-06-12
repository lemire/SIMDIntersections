#ifndef STLUTIL_H_
#define STLUTIL_H_

#include "util.h"
#include "union.h"
#include "intersection.h"

vector<uint32_t> unite(const vector<uint32_t> &x, const vector<uint32_t> &y) {
    vector<uint32_t> ans(x.size() + y.size());
    ans.resize(unite(&x[0], x.size(), &y[0], y.size(), &ans[0]));
    return ans;
}


vector<uint32_t> intersect(const vector<uint32_t> &x,
                           const vector<uint32_t> &y) {
    vector<uint32_t> ans(x.size() + y.size());
    ans.resize(classicalintersection(&x[0], x.size(), &y[0], y.size(), &ans[0]));
    return ans;
}

/**
 * Returns the removed elements
 */
vector<uint32_t> removeRandom(vector<uint32_t> &x, size_t N) {
    auto i = shuffleFY(x.begin(), x.end(), N);
    vector<uint32_t> tmp(i, x.end());
    vector<uint32_t> ans(x.begin(), i);
    x.swap(tmp);
    return ans;
}

vector<uint32_t> getRandom(const vector<uint32_t> &x, size_t N) {
    vector<uint32_t> copy(x);
    auto i = shuffleFY(copy.begin(), copy.end(), N);
    vector<uint32_t> ans(copy.begin(), i);
    return ans;
}

/**
 * Like getRandom except that the provided vector is modified.
 */
vector<uint32_t> grabRandom(vector<uint32_t> &x, size_t N) {
    auto i = shuffleFY(x.begin(), x.end(), N);
    vector<uint32_t> ans(x.begin(), i);
    return ans;
}


vector<uint32_t> difference(const vector<uint32_t> &x,
                            const vector<uint32_t> &y) {
    vector<uint32_t> answer(x.size());
    answer.resize(
        std::set_difference(x.begin(), x.end(), y.begin(), y.end(), answer.begin())
        - answer.begin());
    return answer;

}
#endif
