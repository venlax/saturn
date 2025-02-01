#include "util.h"

#include <cstdlib>
#include <string>
#include <type_traits>

namespace saturn {

    std::string timestampToString(uint64_t timestamp) {
        // 将时间戳转换为time_t类型
        std::time_t time = static_cast<std::time_t>(timestamp);

        // 转换为本地时间（也可以用gmtime转换为UTC时间）
        std::tm *tm = std::localtime(&time);

        // 格式化输出
        std::ostringstream oss;
        oss << std::put_time(tm, "%Y-%m-%d %H:%M:%S");
        return std::move(oss.str());
    }

}