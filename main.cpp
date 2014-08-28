#include "test.h"
#include <iostream>

using namespace std;

template<typename T,typename ... Types>
void testinner(T,Types... args) {
    Test<T,Types...> test(args...);
    test.run();
}

template<unsigned int ... args>
void test() {
    testinner(int(),args...);
    testinner(float(),args...);
    testinner(double(),args...);
    std::cerr<<__PRETTY_FUNCTION__<<" test: Success!"<<std::endl;
}

int main()
{
    test<100>();
    test<30,15>();
    test<10,20,30>();
    test<5,9,7,4>();

    //example
    auto arr=make_array<double>(4u,5u);//create an array of double 4x5 size

    for(uint i=0; i<arr.size()[0]; ++i)
        for(uint j=0; j<arr.size()[1]; ++j)
            arr(i,j)=i*5+j;//assignment

    auto arr2=arr;

    for(uint i=0; i<arr.size()[0]; ++i)
        for(uint j=0; j<arr.size()[1]; ++j)
            std::cout<<arr2.get(i,j)<<"=="<<i*5+j<<std::endl;//retrival

    const auto arr3=std::move(arr);
    for(auto i=arr3.const_begin();i!=arr3.const_end();++i) { //arr is invalid here
        auto& idx(i.index());
        std::cout<<'('<<idx.at(0)<<','<<idx.at(1)<<")="<<arr3(idx)<<std::endl;
    }

    auto arr4=make_array<double>(arr3.size()); //create array with same dimensions
    std::cout<<arr4.size()[0]<<", "<<arr4.size()[1]<<std::endl;

    arr4=std::move(arr3);
    for(auto &i : arr4)
        std::cout<<i<<std::endl;
}

