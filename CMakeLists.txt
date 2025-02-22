# 最低版本要求
cmake_minimum_required(VERSION 3.22)

# 项目名称和版本号
project(Saturn VERSION 1.0)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 20)  # 使用 C++20 , partial 😀
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")

find_package(yaml-cpp REQUIRED)

add_compile_options(-rdynamic)

# 启用调试信息
set(CMAKE_BUILD_TYPE Debug)
# 指定源文件所在目录
include_directories(${PROJECT_SOURCE_DIR}/saturn)

set(SOURCES
    saturn/hook.cc # hook.cc must be the first
    saturn/address.cc
    saturn/socket.cc
    saturn/iomanager.cc
    saturn/http/external/mongrel2/bstr/bstraux.cc
    saturn/http/external/mongrel2/bstr/bstrlib.cc
    saturn/http/external/mongrel2/dbg.cc
    saturn/http/external/mongrel2/http/http11_parser.cc
    saturn/http/external/mongrel2/http/httpclient_parser.cc
    saturn/http/http_parser.cc
    saturn/http/http_session.cc
    saturn/http/http.cc
    saturn/tcpserver.cc
    saturn/stream.cc
    saturn/stream/socketstream.cc
    saturn/timer.cc
    saturn/scheduler.cc
    saturn/util.cc
    saturn/log.cc
    saturn/config.cc
    saturn/thread.cc
    saturn/sem.cc
    saturn/fiber.cc
    saturn/fdmanager.cc
    saturn/bytearray.cc
)

# 启用测试
enable_testing()

# add_executable(test_socket
#     tests/test_socket.cc
#     ${SOURCES}
# ) 
# add_executable(test_address
#     tests/test_address.cc
#     ${SOURCES}
# ) 
# add_executable(test_bytearray
#     tests/test_bytearray.cc
#     ${SOURCES}
# ) 

# add_executable(test_httpparser
#     tests/test_httpparser.cc
#     ${SOURCES}
# ) 
# add_executable(test_http
#     tests/test_http.cc
#     ${SOURCES}
# ) 
# add_executable(test_tcpserver
#     tests/test_tcpserver.cc
#     ${SOURCES}
# ) 

add_executable(echoserver
    tests/examples/echoserver.cc
    ${SOURCES}
)


# target_link_libraries(test_socket /usr/local/lib/libyaml-cpp.so dl)
# target_link_libraries(test_address /usr/local/lib/libyaml-cpp.so dl)
# target_link_libraries(test_bytearray /usr/local/lib/libyaml-cpp.so dl)
# target_link_libraries(test_httpparser /usr/local/lib/libyaml-cpp.so dl)
# target_link_libraries(test_http /usr/local/lib/libyaml-cpp.so dl)
# target_link_libraries(test_tcpserver /usr/local/lib/libyaml-cpp.so dl)
target_link_libraries(echoserver /usr/local/lib/libyaml-cpp.so dl)



# 注册测试
# add_test(NAME TestConfig COMMAND test_config)
