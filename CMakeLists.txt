# 设置最低版本要求
cmake_minimum_required(VERSION 3.0)

# 设置项目名称
project(simple_rpc)

set(CMAKE_CXX_STANDARD 11)



# file(GLOB SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/rocket/common/*.cpp)
file(GLOB SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/rocket/common/*.cc)
message(${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR}/rocket/common)
message(${PROJECT_SOURCE_DIR})

message(${SRC_LIST})

# 添加可执行文件生成
add_executable(app testcases/test_log.cc ${SRC_LIST})

# 添加线程库
target_link_libraries(app pthread)
