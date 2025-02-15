#include "util.h"

#include <chrono>
#include <cstdlib>
#include <execinfo.h>
#include <sstream>
#include <string>
#include <type_traits>

#include "fiber.h"
#include "log.h"
#include "macro.h"

namespace saturn {
    static saturn::Logger::ptr g_logger = LOGGER();


    std::string timestampToString(uint64_t timestamp, std::string_view fmt) {
        // 将时间戳转换为 std::chrono::system_clock::time_point
        std::chrono::system_clock::time_point tp = std::chrono::system_clock::from_time_t(timestamp);

        // 转换为 std::tm 结构
        std::time_t time_t_val = std::chrono::system_clock::to_time_t(tp);
        std::tm tm_val = *std::gmtime(&time_t_val); // 转换为 UTC 时间

        std::ostringstream oss;
        oss << std::put_time(&tm_val, fmt.data());
        return oss.str();
    }


 
    pid_t getThreadId() {
        return syscall(SYS_gettid);
    }
    uint32_t getFiberId() {
        return Fiber::GetFiberId();
    }

    void backtrace(std::vector<std::string>& vec, int size, int skip) {
        int nptrs;
        void** buffer = static_cast<void**>(malloc(size * sizeof(void*)));
        char** strings;
        nptrs = ::backtrace(buffer, size);
        strings = ::backtrace_symbols(buffer, nptrs);
        if (strings == nullptr) {
            SATURN_LOG_ERROR(g_logger) << "backtrace error.";
            free(buffer);
            free(strings);
            return;
        }
        for (size_t i = skip; i < nptrs; ++i) {
            vec.emplace_back(strings[i]);
        }
        free(buffer);
        free(strings);
    }

    std::string backtraceStr(int size , int skip , std::string_view prefix) {
        std::vector<std::string> strings;
        backtrace(strings, size, skip);
        std::stringstream ss;

        for (size_t i = 0; i < strings.size(); ++i) {
            ss << prefix << strings[i] << std::endl;
        }
        return ss.str();
    }


}