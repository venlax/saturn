# 最低版本要求
cmake_minimum_required(VERSION 3.22)

# 项目名称和版本号
project(Saturn VERSION 1.0)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 20)  # 使用 C++20 , partial 😀
set(CMAKE_CXX_STANDARD_REQUIRED True)
find_package(yaml-cpp REQUIRED)

add_compile_options(-rdynamic)

# 启用调试信息
set(CMAKE_BUILD_TYPE Debug)
# 指定源文件所在目录
include_directories(${PROJECT_SOURCE_DIR}/saturn)

set(SOURCES
    saturn/hook.cc # hook.cc must be the first
    saturn/address.cc
    saturn/iomanager.cc
    saturn/timer.cc
    saturn/scheduler.cc
    saturn/util.cc
    saturn/log.cc
    saturn/config.cc
    saturn/thread.cc
    saturn/sem.cc
    saturn/fiber.cc
    saturn/fdmanager.cc
)

# 启用测试
enable_testing()

add_executable(test_hook
    tests/test_hook.cc
    ${SOURCES}
) 

target_link_libraries(test_hook /usr/local/lib/libyaml-cpp.so dl)

# 注册测试
# add_test(NAME TestConfig COMMAND test_config)
