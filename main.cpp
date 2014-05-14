#include "test.h"
#include <iostream>

using namespace std;

template<typename ... Types>
void testint(Types... args) {
    Test<int,Types...> test(args...);
    test.run();
}

template<typename ... Types>
void testfloat(Types... args) {
    Test<float,Types...> test(args...);
    test.run();
}

template<typename ... Types>
void testdouble(Types... args) {
    Test<double,Types...> test(args...);
    test.run();
}

template<unsigned int ... args>
void test() {
    testint(args...);
    testdouble(args...);
    testfloat(args...);
    std::cerr<<__PRETTY_FUNCTION__<<" test: Success!"<<std::endl;
}

int main()
{
    test<100>();
    test<30,15>();
    test<10,20,30>();
    test<5,9,7,4>();

    //example
    MultiArray<double> arr(4,5);//create an array of double 4x5 size

    for(int i=0; i<4; ++i)
        for(int j=0; j<5; ++j)
            arr(i,j)=i*5+j;//assignment

    for(int i=0; i<4; ++i)
        for(int j=0; j<5; ++j)
            std::cout<<arr(i,j)<<"=="<<i*5+j<<std::endl;//retrival
}

