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
    MultiArray<double> arr(4u,5u);//create an array of double 4x5 size

    for(int i=0; i<4; ++i)
        for(int j=0; j<5; ++j)
            arr(i,j)=i*5+j;//assignment

    for(int i=0; i<4; ++i)
        for(int j=0; j<5; ++j)
            std::cout<<arr(i,j)<<"=="<<i*5+j<<std::endl;//retrival
}

