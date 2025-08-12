cmake_minimum_required(VERSION 3.10)
project(oldblib_oracle_tests)

# C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 调试模式（默认启用，可注释切换为Release）
set(CMAKE_BUILD_TYPE Debug)

# 平台编译选项
if(WIN32)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /O0 /g /W3 /D_CRT_SECURE_NO_WARNINGS")
else()
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g -D_GLIBCXX_USE_CXX11_ABI=1")
endif()

# 头文件路径（Oracle和自身）
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
if(WIN32)
    include_directories(${OCI_INCLUDE})  # Windows头文件
else()
    include_directories($ENV{ORACLE_HOME}/rdbms/public)  # Linux头文件
endif()

# 库路径（区分平台）
if(WIN32)
    link_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/../lib/win32/static
        ${OCI_LIB}
    )
else()
    link_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/../lib/linux/static
        $ENV{ORACLE_HOME}/lib
    )
endif()

# 测试源文件
file(GLOB TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

# 输出路径（区分平台）
if(WIN32)
    set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin/win32)
else()
    set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin/linux)
endif()
file(MAKE_DIRECTORY ${EXECUTABLE_OUTPUT_PATH})

# 生成测试程序
foreach(TEST_FILE ${TEST_FILES})
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
    add_executable(${TEST_NAME} ${TEST_FILE})
# 链接库（区分Oracle库）
if(WIN32)
    target_link_libraries(${TEST_NAME} PRIVATE oldba oci)
else()
    target_link_libraries(${TEST_NAME} PRIVATE oldba clntsh)
endif()
endforeach()