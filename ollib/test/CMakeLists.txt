cmake_minimum_required(VERSION 3.10)
project(ollib_tests)

# C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 调试模式（默认启用，可注释切换为Release）
set(CMAKE_BUILD_TYPE Debug)

# 平台编译选项
if(WIN32)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /O0 /g /W3")
else()
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g -D_GLIBCXX_USE_CXX11_ABI=1")
endif()

# 头文件和库路径（复用主库变量）
include_directories(${OLLIB_INCLUDE})
link_directories(${OLLIB_STATIC_LIB})

# 链接主库
link_libraries(ol)

# 测试源文件
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/. TEST_SRC)

# 输出路径（区分平台）
if(WIN32)
    set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin/win32)
else()
    set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin/linux)
endif()
file(MAKE_DIRECTORY ${EXECUTABLE_OUTPUT_PATH})

# 生成测试程序
foreach(TEST_FILE ${TEST_SRC})
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
    add_executable(${TEST_NAME} ${TEST_FILE})
endforeach()