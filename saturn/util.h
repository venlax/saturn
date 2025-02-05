#ifndef __SATURN_UTIL_H__
#define __SATURN_UTIL_H__

#include "log.h"
#include <boost/lexical_cast.hpp>
#include <concepts>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
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


// #include "config.h"


/**
 * This is the saturn's util lib
 * providing some useful functions in saturn 
 * if the function is a template function, the impl will just be in the header file (aka util.h)
 * otherwise, the function's impl will be found at util.cc 😊
 */

namespace saturn {
    class LogConfig;
    class LogAppenderConfig;
    template <typename T>
    concept is_vector = requires { typename T::value_type; } && std::same_as<T, std::vector<typename T::value_type>>;
    template <typename T>
    concept is_list = requires { typename T::value_type; } && std::same_as<T, std::list<typename T::value_type>>;
    template <typename T>
    concept is_set = requires { typename T::value_type; } && std::same_as<T, std::set<typename T::value_type>>;
    template <typename T>
    concept is_unordered_set = requires { typename T::value_type; } && std::same_as<T, std::unordered_set<typename T::value_type>>;
    template <typename T>
    concept is_map = requires { typename T::key_type; typename T::mapped_type;} && std::same_as<T, std::map<typename T::key_type, typename T::mapped_type>>;;
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
    uint64_t getCurrentTime();
    pid_t getThreadId();
    uint32_t getFiberId();

    template<class From, class To>
    class cast {
        public:
            To operator()(const From& var) {
                static_assert(!std::is_pointer_v<From>, "can't cast the pointer");
            
                if constexpr (std::is_same_v<std::string, From> &&
                            std::is_same_v<LogLevel, To>) {
                    if (var == "DEBUG") return LogLevel::DEBUG;
                    else if (var == "INFO") return LogLevel::INFO;
                    else if (var == "WARN") return LogLevel::WARN;
                    else if (var == "ERROR") return LogLevel::ERROR;
                    else if (var == "FATAL") return LogLevel::FATAL;
                    return LogLevel::UNKNOWN;
                } else if constexpr (std::is_same_v<std::string, From> && 
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
                }  else if constexpr (std::is_same_v<std::string, From> && 
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
                }  else if constexpr (std::is_same_v<std::string, From> &&
                            std::is_same_v<LogConfig, To>) {
                    YAML::Node node = YAML::Load(var);
                    LogConfig config;

                    if (!node["name"].IsDefined()) {
                        std::cout << "logger name must define." << std::endl;
                        throw std::logic_error("name undefined");
                    }
                    config.name = node["name"].as<std::string>();
                    config.level = node["level"].IsDefined() ? cast<std::string, LogLevel>()(node["level"].as<std::string>()) : LogLevel::UNKNOWN;
                    if (node["formatter"].IsDefined())  {
                        config.formatter = node["formatter"].as<std::string>();
                    }
                    if (node["appenders"].IsDefined()) {
                        for (size_t i = 0; i < node["appenders"].size(); ++i) {
                            auto ap = node["appenders"][i];
                            LogAppenderConfig la_config;
                            if (!ap["type"].IsDefined()) {
                                std::cout << "appender type must define." << std::endl;
                                throw std::logic_error("appender type undefined");
                            }
                            std::string type = ap["type"].as<std::string>();
                            if (type == "StdoutLogAppender") {
                                la_config.type = 1;
                            } else if (type == "FileLogAppender") {
                                la_config.type = 2;
                                if (!ap["file"].IsDefined()) {
                                    std::cout << "FileLogAppender must define the \"file\"" << std::endl;
                                    throw std::logic_error("file path undefined");
                                }
                                la_config.file = ap["file"].as<std::string>();
                            } else {
                                std::cout << "LogAppender unknown type: " << type << " " << std::endl;
                                throw std::logic_error("unknown log appender type");
                            }

                            la_config.level = ap["level"].IsDefined() ? cast<std::string, LogLevel>()(ap["level"].as<std::string>()) : LogLevel::UNKNOWN;
                            
                            if (ap["formatter"].IsDefined()) {
                                la_config.formatter = ap["formatter"].as<std::string>();
                            }
                            config.appenders.push_back(std::move(la_config));
                        }
                    }
                    return config;
                } else if constexpr (std::is_same_v<LogLevel, From> 
                            && std::is_same_v<To, std::string>) {
                        return level_str[static_cast<size_t>(var)].data();
                } else if constexpr ( is_std_container<From> 
                            && std::is_same_v<std::string, To>) {
                    YAML::Node node;
                    for (auto& i : var) {
                        if constexpr (is_value_container<From>) node.push_back(YAML::Load(cast<typename From::value_type, std::string>()(i)));
                        else if constexpr (is_key_value_container<From>) node[i.first] = YAML::Load(cast<typename From::mapped_type, std::string>()(i.second));
                    }
                    std::stringstream ss;
                    ss << node;
                    return ss.str();
                } else if constexpr (std::is_same_v<LogConfig, From>
                            && std::is_same_v<std::string, To>) {
                    YAML::Node node;
                    node["name"] = var.name;
                    node["level"] = cast<LogLevel, std::string>()(var.level);
                    node["formatter"] = var.formatter;
                    for (auto& ap : var.appenders) {
                        YAML::Node na;
                        if(ap.type == 1) {
                            na["type"] = "StdoutLogAppender";
                        } else if(ap.type == 2) {
                            na["type"] = "FileLogAppender";
                            na["file"] = ap.file;
                        }
                        na["level"] = cast<LogLevel, std::string>()(ap.level);
                        na["formatter"] = ap.formatter;
                        node["appenders"].push_back(na);
                    }
                    std::stringstream ss;
                    ss << node;
                    return ss.str();
                } else {
                    return boost::lexical_cast<To>(var);
                }
                
        }
    };

}

#endif // !__SATURN_UTIL_H__