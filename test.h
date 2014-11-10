#ifndef TEST_H
#define TEST_H

#include "multiarray.h"
#include <vector>
#include <random>
#include <cassert>
#include <iostream>

template<typename T,typename ... Types>
class Test;

template<typename T,typename D>
void test_slice_2(Test<T,D>&) {}

template<typename T,typename ... Types>
void test_slice_2(Test<T,Types...> &test);

template<typename T,typename D>
void test_slice_3(Test<T,D>&) {}

template<typename T,typename D, typename E>
void test_slice_3(Test<T,D,E>&) {}

template<typename T,typename ... Types>
void test_slice_3(Test<T,Types...> &test);

template<typename T,typename ... Types>
class Test
{
private:
    using idx_t=long long unsigned int;
    MultiArray<T,sizeof...(Types)> ma;
    std::array<unsigned int,sizeof...(Types)> count;
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

    template<unsigned int N, typename T1, typename...Types1> struct tuple_gen : tuple_gen<N-1,T1,T1,Types1...> {};
    template<typename T1, typename ... Types1> struct tuple_gen<0,T1,Types1...> { typedef std::tuple<Types1...> type; };

    template<unsigned int ...I>
    typename tuple_gen<sizeof...(I),range>::type make_slice(sequtils::seq<I...>) {
        return std::make_tuple(range(0,count[I]-1)...);
    }

    template<unsigned int ...I>
    typename tuple_gen<sizeof...(I),range>::type make_slice2(sequtils::seq<I...>) {
        return typename tuple_gen<sizeof...(I),range>::type();
    }

    friend void test_slice_2<>(Test&);
    friend void test_slice_3<>(Test&);

public:
    Test(Types... counts) : ma(counts...),count({{counts...}}) {
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
        //array index check
        {
            vi=0;
            for(auto i=ma.const_begin(); i!=ma.const_end(); ++i) {
                assert(ma(i.index())==values[vi++]);
            }
            assert(vi==vi_max);
            vi=0;
            for(auto i=ma.begin(); i!=ma.end(); ++i) {
                assert(ma(i.index())==values[vi++]);
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
        //out_of_range check
        {
            bool pass=false;
            try {
                auto it=ma.end();
                *it=0;
            } catch (std::out_of_range &e) {
                pass=true;
                assert(std::string(e.what())=="MultiArray index out of range");
            }
            assert(pass);
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
            ma=mva;
        }
        //copy-assigment check
        {
            decltype(ma) ca;
            assert(!ca.valid());
            ca=ma; //shallow copy
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
        //move-assignment check
        {
            decltype(ma) mva;
            assert(!mva.valid());
            mva=std::move(ma);
            assert(!ma.valid());
            vi=0;
            for(auto i=mva.const_begin(); i!=mva.const_end(); ++i) {
                assert(*i==values[vi++]);
            }
            assert(vi==vi_max);
            ma=mva;
        }
        //slice check
        {
            //basically copy, range(...)
            {
                auto slice_arg = make_slice(typename sequtils::gens<sizeof...(Types)>::type());
                auto slice=ma.slice(slice_arg);
                vi=0;
                for(auto i=slice.const_begin(); i!=slice.const_end(); ++i) {
                    assert(*i==values[vi++]);
                }
            }
            //no last dimension, range(...), range{...}
            {
                auto slice_arg = make_slice(typename sequtils::gens<sizeof...(Types)-1>::type());
                auto slice=ma.slice(std::tuple_cat(slice_arg,std::make_tuple(range{0})));
                vi=0;
                for(auto i=slice.const_begin(); i!=slice.const_end(); ++i) {
                    assert(*i==values[vi]);
                    vi+=count.back();
                }
            }
            //no first/last dimension
            {
                test_slice_2(*this);
            }
            //no first/last 2 dimensions
            {
                test_slice_3(*this);
            }
        }
        //logic_error check
        {
            auto mva=std::move(ma);
            bool pass=false;
            try {
                auto it=ma.begin();
                *it=0;
            } catch (std::logic_error &e) {
                pass=true;
                assert(std::string(e.what())=="Using invalid MultiArray");
            }
            assert(pass);
            ma=mva;
        }
    }
};

#endif // TEST_H

template<typename T,typename ... Types>
void test_slice_2(Test<T,Types...> &test) {
    {//no last dim
        auto slice_arg = test.make_slice2(typename sequtils::gens<sizeof...(Types)-1>::type());
        auto slice=test.ma.slice(std::tuple_cat(slice_arg,std::make_tuple(0u)));
        int vi=0;
        for(auto i=slice.const_begin(); i!=slice.const_end(); ++i) {
            assert(*i==test.values[vi]);
            vi+=test.count.back();
        }
    }
    {//no first dim
        auto slice_arg = test.make_slice2(typename sequtils::gens<sizeof...(Types)-1>::type());
        auto slice=test.ma.slice(std::tuple_cat(std::make_tuple(0u),slice_arg));
        int vi=0;
        for(auto i=slice.const_begin(); i!=slice.const_end(); ++i) {
            assert(*i==test.values[vi++]);
        }
    }
}

template<typename T,typename ... Types>
void test_slice_3(Test<T,Types...> &test) {
    {//no last 2 dims
        auto slice_arg = test.make_slice2(typename sequtils::gens<sizeof...(Types)-2>::type());
        auto slice=test.ma.slice(std::tuple_cat(slice_arg,std::make_tuple(0u,1u)));
        int vi=0;
        for(auto i=slice.const_begin(); i!=slice.const_end(); ++i) {
            assert(*i==test.values[vi+1]);
            vi+=test.count.back()*test.count[sizeof...(Types)-2];
        }
    }
    {//no first no last, range()
        auto slice_arg = test.make_slice2(typename sequtils::gens<sizeof...(Types)-2>::type());
        auto slice=test.ma.slice(std::tuple_cat(std::make_tuple(0u),slice_arg,std::make_tuple(1u)));
        int vi=0;
        for(auto i=slice.const_begin(); i!=slice.const_end(); ++i) {
            assert(*i==test.values[vi+1]);
            vi+=test.count.back();
        }
    }
    {//no 2 first, range()
        auto slice_arg = test.make_slice2(typename sequtils::gens<sizeof...(Types)-2>::type());
        auto slice=test.ma.slice(std::tuple_cat(std::make_tuple(0u,0u),slice_arg));
        int vi=0;
        for(auto i=slice.const_begin(); i!=slice.const_end(); ++i) {
            assert(*i==test.values[vi++]);
        }
    }
}
