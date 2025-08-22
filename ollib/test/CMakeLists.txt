cmake_minimum_required(VERSION 3.10)
project(ollib_tests)

# C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 调试模式（默认启用，可注释切换为Release）
set(CMAKE_BUILD_TYPE Debug)

# 平台编译选项
if(MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /O0 /g /W3")
else()
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g -D_GLIBCXX_USE_CXX11_ABI=1")
endif()

# 头文件和库路径（复用主库变量）
include_directories(${OLLIB_INCLUDE})
link_directories(${OLLIB_STATIC_LIB})

# 链接主库
link_libraries(ol)

# 测试源文件 - 先收集所有测试文件
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/. ALL_TEST_SRC)

# 过滤非Linux平台的测试文件
if(NOT LINUX)
    # 从测试列表中移除相关测试文件
    list(FILTER ALL_TEST_SRC EXCLUDE REGEX 
        "test_(fifo_|ol_ftp_|ol_ipc|ol_tcp_|ol_signal|pipe|shm).*\\.cpp$"
    )
endif()

# 输出路径（区分平台）
if(WIN32)
    set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin/win32)
else()
    set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin/linux)
endif()
file(MAKE_DIRECTORY ${EXECUTABLE_OUTPUT_PATH})

# 生成测试程序（只使用过滤后的测试文件列表）
foreach(TEST_FILE ${ALL_TEST_SRC})
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
    add_executable(${TEST_NAME} ${TEST_FILE})
    
    # 对于Linux平台的FIFO测试程序，无需额外操作
    # 对于Windows平台，已通过上面的过滤排除了不兼容的测试文件
endforeach()