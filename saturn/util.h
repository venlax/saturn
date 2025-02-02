#ifndef __SATURN_UTIL_H__
#define __SATURN_UTIL_H__

#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>

#include <stdio.h>
#include <sys/syscall.h>
#include <unistd.h>



/**
 * This is the saturn's util lib
 * providing some useful functions in saturn 
 * if the function is a template function, the impl will just be in the header file (aka util.h)
 * otherwise, the function's impl will be found at util.cc 😊
 */

namespace saturn {

    std::string timestampToString(uint64_t timestamp, std::string_view fmt = "%Y-%m-%dT%H:%M:%SZ"); 
    uint64_t get_current_time();
    pid_t get_thread_id();
    uint32_t get_fiber_id();


    template <typename To, typename From>
    To lexical_cast(const From& _) {
        if constexpr (std::is_same<std::string, To>::value) {
            return std::move(std::to_string(_));
        }
        if constexpr (std::is_same<std::string, From>::value 
            || std::is_same<std::string_view, From>::value) {
            if constexpr (std::is_floating_point<To>::value) {
                return static_cast<To>(std::atof(_.data()));
            } else if constexpr (std::is_integral<To>::value) {
                return static_cast<To>(std::atoll(_.data()));
            }
        }
    }

}

#endif // !__SATURN_UTIL_H__