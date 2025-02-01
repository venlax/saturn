# 最低版本要求
cmake_minimum_required(VERSION 3.22)

# 项目名称和版本号
project(Saturn VERSION 1.0)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 20)  # 使用 C++20
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 指定源文件所在目录
include_directories(${PROJECT_SOURCE_DIR}/saturn)

# add_executable(Saturn
#     saturn/log.cc
# )
# 启用测试
enable_testing()

add_executable(test_log
    tests/test_log.cc
    saturn/util.cc
    saturn/log.cc
)

add_executable(test_util
    tests/test_util.cc
    saturn/util.cc
) 

# 注册测试
add_test(NAME TestLog COMMAND test_log)
add_test(NAME TestUtil COMMAND test_util)
