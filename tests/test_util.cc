#include "util.h"

#include <cassert>
#include <cstring>
#include <ctime>
#include <iostream>
#include <string>
#include <string_view>


using namespace std;

#define test_cast(val) { \
    std::string temp = saturn::cast<decltype(val), std::string>()(val); \
    std::cout << temp << std::endl; \
}



int main() {
    int i = 5;
    // int* ptr = &i;
    test_cast(i);
    
    float f = 0.5f;
    test_cast(f);

    bool b = true;
    test_cast(b);

    std::vector<int> vec = {1,2,3};
    test_cast(vec);

    std::list<float> list = {1.0,2.0,3.5};
    test_cast(list);

    std::set<double> set = {7.5, 8.4, 9.3};
    test_cast(set);

    std::map<int, double> map = {{5,5.5}, {6,6.5}, {7,7.5}};
    test_cast(map);


}