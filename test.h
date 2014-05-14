#ifndef TEST_H
#define TEST_H

#include "multiarray.h"
#include <vector>
#include <random>

template<typename T,typename ... Types>
class Test
{
private:
    using idx_t=long long unsigned int;
    MultiArray<T> ma;
    std::vector<unsigned int> count;
    idx_t size;

    std::vector<T> values;
    int vi;

    template<typename...Types2>
    void fill(bool check, Types2...args) {
        constexpr int ndim=sizeof...(args);
        for(unsigned int i=0; i<count[ndim];++i) {
            fill(check,args...,i);
        }
    }

    void fill(bool check, Types ... args) {
        if(check) {
            assert(ma(args...)==values[vi]);
        }else {
            values[vi]=std::rand();
            ma(args...)=values[vi];
        }
        vi++;
    }

public:
    Test(Types... counts) : ma(counts...),count({counts...}) {
        size=1;
        for(auto i:count)
            size*=i;
        values.resize(size);
    }

    void run() {
        vi=0;
        fill(false);
        vi=0;
        fill(true);
    }
};

#endif // TEST_H
