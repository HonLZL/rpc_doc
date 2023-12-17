# 设置最低版本要求
cmake_minimum_required(VERSION 3.0)

# 设置项目名称
project(simple_rpc)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")


message("PROJECT_SOURCE_DIR}: " ${PROJECT_SOURCE_DIR} "&&&")

file(GLOB SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/rocket/common/*.cc)
message(${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR}/rocket/common)

include_directories(/usr/include/tinyxml)

message(${PROJECT_SOURCE_DIR})

message(${SRC_LIST})

# 添加可执行文件生成
add_executable(app testcases/test_log.cc ${SRC_LIST})

# 添加线程库
target_link_libraries(app pthread)

# target_link_libraries(app libtinyxml.so)
target_link_libraries(app tinyxml)

# sudo code --user-data-dir ="~/.vscode-root" --no-sandbox
