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

# add_executable(Saturn
#     saturn/log.cc
# )
# 启用测试
enable_testing()

# add_executable(test_log
#     tests/test_log.cc
#     saturn/config.cc
#     saturn/util.cc
#     saturn/log.cc
# )

# add_executable(test_util
#     tests/test_util.cc
#     saturn/util.cc
#     saturn/log.cc
#     saturn/config.cc
#     saturn/thread.cc
#     saturn/sem.cc
# ) 
# add_executable(test_fiber
#     tests/test_fiber.cc
#     saturn/util.cc
#     saturn/log.cc
#     saturn/config.cc
#     saturn/thread.cc
#     saturn/sem.cc
#     saturn/fiber.cc
# ) 
add_executable(test_scheduler
    tests/test_scheduler.cc
    saturn/scheduler.cc
    saturn/util.cc
    saturn/log.cc
    saturn/config.cc
    saturn/thread.cc
    saturn/sem.cc
    saturn/fiber.cc
) 
# add_executable(test_config
#     tests/test_config.cc
#     saturn/config.cc
#     saturn/log.cc
#     saturn/util.cc
# )

# add_executable(test_thread
#     tests/test_thread.cc
#     saturn/config.cc
#     saturn/log.cc
#     saturn/util.cc
#     saturn/thread.cc
#     saturn/sem.cc
# )
# target_link_libraries(test_log /usr/local/lib/libyaml-cpp.so)
target_link_libraries(test_scheduler /usr/local/lib/libyaml-cpp.so)
# target_link_libraries(test_config /usr/local/lib/libyaml-cpp.so)
# target_link_libraries(test_thread /usr/local/lib/libyaml-cpp.so)

# 注册测试
# add_test(NAME TestLog COMMAND test_log)
# add_test(NAME TestUtil COMMAND test_util)
add_test(NAME TestConfig COMMAND test_config)
