#include "util.h"

#include <chrono>
#include <cstdlib>
#include <string>
#include <type_traits>

namespace saturn {

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

    uint64_t getCurrentTime() {
        // 获取当前时间点
        auto now = std::chrono::system_clock::now();
    
        // 获取当前时间自纪元以来的毫秒数
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    
        // 返回64位时间戳
        return duration.count();
    }

 
    pid_t getThreadId() {
        return syscall(SYS_gettid);
    }
    uint32_t getFiberId() {
        // TODO
        return 0;
    }



}