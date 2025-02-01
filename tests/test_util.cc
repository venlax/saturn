#include "util.h"

#include <cassert>
#include <cstring>
#include <ctime>
#include <iostream>
#include <string>
#include <string_view>


using namespace std;

int main() {
    // assert(saturn::timestampToString(0) == "1970-01-01 08:00:00");
    // // cout << saturn::timestampToString(timeStamp) << endl;
    // assert(saturn::timestampToString(365 * 24 * 60 * 60) == "1971-01-01 08:00:00");

    string str = "125";

    int result = saturn::lexical_cast<int, string>(str);
    assert(result == 125);

    result = saturn::lexical_cast<int , string_view>(string_view(str));

    assert(result == 125);

    string str2 = "1000000000000";

    long long result2 = saturn::lexical_cast<long long, string>(str2);
    assert(result2 == 1000000000000);

    string str3 = "1.125";
    float f = saturn::lexical_cast<float, string>(str3);

    assert(f == 1.125);

    str3 = "16777217";

    // overflow
    f = saturn::lexical_cast<float,  string>(str3);

    assert(f != 16777217.0);
    assert(f == 16777216.0);

    double d = saturn::lexical_cast<double, string>(str3);

    assert(d == 16777217.0);

    string dStr = saturn::lexical_cast<string, double>(d);
    
    assert(dStr == "16777217.000000");
}