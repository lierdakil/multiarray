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
    MultiArray<T,sizeof...(Types)> ma;
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
        auto vi_max=vi;
        //assignment check
        {
            vi=0;
            fill(false);
            vi_max=vi;
            vi=0;
            fill(true);
            assert(vi==vi_max);
        }
        //iterator check
        {
            vi=0;
            for(auto i=ma.const_begin(); i!=ma.const_end(); ++i) {
                assert(*i==values[vi++]);
            }
            assert(vi==vi_max);
            vi=0;
            for(auto i=ma.begin(); i!=ma.end(); ++i) {
                assert(*i==values[vi++]);
            }
            assert(vi==vi_max);
        }
        //copy check
        {
            const auto ca=ma; //shallow copy
            auto i=ma.const_begin(),j=ca.const_begin();
            for(; i!=ma.const_end() && j!=ca.const_end(); ++i, ++j) {
                assert(&*i==&*j);
            }
            assert(i==ma.const_end()&&j==ca.const_end());
            auto cvalues=values;
            vi=0;
            fill(false);//reset ma, should cow
            i=ma.const_begin(),j=ca.const_begin();
            for(; i!=ma.const_end() && j!=ca.const_end(); ++i, ++j) {
                assert(&*i!=&*j);
            }
            assert(i==ma.const_end()&&j==ca.const_end());
            vi=0;
            for(auto i=ma.const_begin(); i!=ma.const_end(); ++i) {
                assert(*i==values[vi++]);
            }
            assert(vi==vi_max);
            vi=0;
            for(auto i=ca.const_begin(); i!=ca.const_end(); ++i) {
                assert(*i==cvalues[vi++]);
            }
            assert(vi==vi_max);
        }
        //move check
        {
            auto mva=std::move(ma);
            assert(!ma.valid());
            vi=0;
            for(auto i=mva.const_begin(); i!=mva.const_end(); ++i) {
                assert(*i==values[vi++]);
            }
            assert(vi==vi_max);
        }
    }
};

#endif // TEST_H
