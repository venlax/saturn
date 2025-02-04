#ifndef __SATURN_UTIL_H__
#define __SATURN_UTIL_H__

#include <boost/lexical_cast.hpp>
#include <concepts>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <yaml-cpp/node/parse.h>
#include <yaml-cpp/yaml.h>

#include <sys/syscall.h>
#include <unistd.h>



/**
 * This is the saturn's util lib
 * providing some useful functions in saturn 
 * if the function is a template function, the impl will just be in the header file (aka util.h)
 * otherwise, the function's impl will be found at util.cc 😊
 */

namespace saturn {

    template <typename T>
    concept is_vector = requires { typename T::value_type; } && std::same_as<T, std::vector<typename T::value_type>>;
    template <typename T>
    concept is_list = requires { typename T::value_type; } && std::same_as<T, std::list<typename T::value_type>>;
    template <typename T>
    concept is_set = requires { typename T::value_type; } && std::same_as<T, std::set<typename T::value_type>>;
    template <typename T>
    concept is_unordered_set = requires { typename T::value_type; } && std::same_as<T, std::unordered_set<typename T::value_type>>;
    template <typename T>
    concept is_map = requires {typename T::key_type; typename T::mapped_type;} && std::same_as<T, std::map<typename T::key_type, typename T::mapped_type>>;;
    template <typename T>
    concept is_unordered_map = requires {typename T::key_type; typename T::mapped_type;} && std::same_as<T, std::unordered_map<typename T::key_type, typename T::mapped_type>>;;

    template <typename T>
    concept is_sequence_container = is_vector<T> || is_list<T>;

    template <typename T>
    concept is_associative_container_set = is_set<T> || is_unordered_set<T>;

    template <typename T>
    concept is_associative_container_map = is_map<T> || is_unordered_map<T>;

    template <typename T>
    concept is_associative_container = is_associative_container_set<T> || is_associative_container_map<T>;

    template <typename T>
    concept is_value_container = is_sequence_container<T> || is_associative_container_set<T>;

    template <typename T>
    concept is_key_value_container = is_associative_container_map<T>;

    template <typename T>
    concept is_std_container = is_sequence_container<T> || is_associative_container<T>;

    std::string timestampToString(uint64_t timestamp, std::string_view fmt = "%Y-%m-%dT%H:%M:%SZ"); 
    uint64_t get_current_time();
    pid_t get_thread_id();
    uint32_t get_fiber_id();

    template<class From, class To>
    class cast {
        public:
            To operator()(const From& var) {
                static_assert(!std::is_pointer_v<From>, "can't cast the pointer");

                if constexpr (std::is_scalar_v<From>) {
                    return boost::lexical_cast<To>(var);    
                }
            
                if constexpr (std::is_same_v<std::string, From> && 
                            (is_value_container<To>)) {
                    YAML::Node node = YAML::Load(var);
                    To container;
                    for (size_t i = 0; i < node.size(); ++i) {
                        std::stringstream ss;
                        ss << node[i];
                        if constexpr (is_sequence_container<To>) container.emplace_back(cast<std::string, typename To::value_type>()(ss.str()));
                        else if constexpr (is_associative_container_set<To>) container.emplace(cast<std::string, typename To::value_type>()(ss.str()));
                    }
                    return container;
                }   
                if constexpr (std::is_same_v<std::string, From> && 
                            (is_key_value_container<To>)) {
                    YAML::Node node = YAML::Load(var);
                    To container;
                    for (auto iter = node.begin();
                        iter != node.end();
                        ++iter) {
                        std::stringstream ss;
                        ss << iter->second;
                        container.emplace(iter->first.Scalar(), cast<std::string, typename To::mapped_type>()(ss.str()));
                    }   
                    return container;
                }   


                if constexpr ( is_std_container<From> 
                            && std::is_same_v<std::string, To>) {
                    YAML::Node node;
                    for (auto& i : var) {
                        if constexpr (is_value_container<From>) node.push_back(YAML::Load(cast<typename From::value_type, std::string>()(i)));
                        else if constexpr (is_key_value_container<From>) node[i.first] = YAML::Load(cast<typename From::mapped_type, std::string>()(i.second));
                    }
                    std::stringstream ss;
                    ss << node;
                    return ss.str();
                }
        }
    };

}

#endif // !__SATURN_UTIL_H__