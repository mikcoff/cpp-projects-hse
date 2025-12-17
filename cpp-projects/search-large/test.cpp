#include <random>
#include <algorithm>
#include <unordered_set>
#include <chrono>
#include <cstdlib>
#include <sstream>

#include "search.h"

#include <catch2/catch_test_macros.hpp>

bool SearchStl(const std::vector<int>& data, int value) {
    return std::binary_search(data.begin(), data.end(), value);
}

template <typename F>
int64_t BenchSingle(const F& binsearch, const std::vector<int>& sorted,
                    const std::vector<int>& unsorted, size_t nlookups) {
    auto start = std::chrono::high_resolution_clock::now();
    auto lookup = [&](size_t i) {
        if (!binsearch(sorted, unsorted[i])) {
            INFO("Fail binsearch");
        }
    };
    for (size_t j = 0; j < nlookups / unsorted.size(); ++j) {
        for (size_t i = 0; i < unsorted.size(); ++i) {
            lookup(i);
        }
    }
    for (size_t i = 0; i < nlookups % unsorted.size(); ++i) {
        lookup(i);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    return duration;
}

template <typename F>
double BenchBest(const F& binsearch, const std::vector<int>& sorted,
                 const std::vector<int>& unsorted) {
    size_t nlookups = std::max(1000UL, unsorted.size());
    nlookups = std::min(nlookups, 1UL << 16);
    auto best = BenchSingle(binsearch, sorted, unsorted, nlookups);
    for (int i = 0; i < 10; ++i) {
        asm volatile("");
        best = std::min(best, BenchSingle(binsearch, sorted, unsorted, nlookups));
    }
    return double(best) / nlookups;
}

void Bench(const std::vector<int>& sorted, const std::vector<int>& unsorted) {
    double basic = BenchBest(SearchStl, sorted, unsorted);
    double good = BenchBest(Search, sorted, unsorted);
    double ratio = basic / good;

    std::stringstream ss;
    ss << "Search in " << sorted.size()
       << " elements.\n"
          " std::binary_search "
       << basic
       << "ns per search.\n"
          " Yours "
       << good << "ns per search. Speedup: " << ratio << std::endl;

    INFO(ss.str());
    REQUIRE(ratio > 2);
}

void TestPerformance(int n) {
    std::vector<int> data(n);
    std::vector<int> sdata(n);
    std::random_device device;
    std::mt19937 mt(device());
    for (int i = 0; i < n; ++i) {
        data[i] = mt();
        sdata[i] = data[i];
    }
    std::sort(sdata.begin(), sdata.end());
    Bench(sdata, data);
}

void Validate(int n) {
    std::vector<int> data(n);
    std::vector<int> sdata(n);
    std::random_device device;
    std::mt19937 mt(device());
    std::unordered_set<int> map;
    for (int i = 0; i < n; ++i) {
        data[i] = mt();
        sdata[i] = data[i];
        map.insert(data[i]);
    }
    std::sort(sdata.begin(), sdata.end());
    for (auto v : data) {
        REQUIRE(Search(sdata, v));
    }
    for (int i = 0; i < n; ++i) {
        int v = mt();
        REQUIRE(Search(sdata, v) == (map.find(v) != map.end()));
    }
}

TEST_CASE("Correctness") {
    for (int i = 0; i < 20; ++i) {
        Validate(1 << i);
    }
}

TEST_CASE("Performance") {
    for (int i = 20; i > 15; --i) {
        TestPerformance(1 << i);
    }
}
