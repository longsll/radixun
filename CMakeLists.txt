cmake_minimum_required (VERSION 3.22.1) #cmake版本

project (demo) #项目名称

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -fPIC -ggdb -std=c++20 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")
set(CMAKE_C_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -fPIC -ggdb -std=c11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")


set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin) #输出文件路径


aux_source_directory (./radixun/common RADIXUN_COMMON_LIST) #添加./src编译路径
aux_source_directory (./radixun/http RADIXUN_HTTP_LIST) #添加./src编译路径
aux_source_directory (./radixun/net RADIXUN_NET_LIST) #添加./src编译路径


#find yaml package
find_package(yaml-cpp REQUIRED)
include_directories(${YAML_CPP_INCLUDE_DIR})
add_executable (main ./radixun/main.cpp ${RADIXUN_COMMON_LIST} ${RADIXUN_HTTP_LIST} ${RADIXUN_NET_LIST} ) #连接编译编译路径
#动态连接yaml库
target_link_libraries(main yaml-cpp::yaml-cpp)
