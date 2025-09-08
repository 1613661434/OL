cmake_minimum_required(VERSION 3.10)
project(oldblib_mysql_tests)

# C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 调试模式（默认启用，可注释切换为Release）
set(CMAKE_BUILD_TYPE Debug)

# 平台编译选项
if(MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /O0 /g /W3 /D_CRT_SECURE_NO_WARNINGS")
else()
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g -D_GLIBCXX_USE_CXX11_ABI=1")
endif()

# 头文件路径（MySQL和自身）
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
if(WIN32)
    # Windows下MySQL头文件路径，可通过环境变量指定
    if(DEFINED ENV{MYSQL_INCLUDE})
        include_directories($ENV{MYSQL_INCLUDE})
    else()
        # 默认安装路径
        include_directories("C:/Program Files/MySQL/MySQL Server 8.0/include")
    endif()
else()
    # Linux下MySQL头文件通常在系统路径或通过环境变量指定
    if(DEFINED ENV{MYSQL_INCLUDE})
        include_directories($ENV{MYSQL_INCLUDE})
    else()
        include_directories("/usr/include/mysql")
    endif()
endif()

# 库路径（区分平台）
if(WIN32)
    link_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/../lib/win32/static
    )
    # Windows下MySQL库路径
    if(DEFINED ENV{MYSQL_LIB})
        link_directories($ENV{MYSQL_LIB})
    else()
        # 默认安装路径
        link_directories("C:/Program Files/MySQL/MySQL Server 8.0/lib")
    endif()
else()
    link_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/../lib/linux/static
    )
    # Linux下MySQL库通常在系统路径或通过环境变量指定
    if(DEFINED ENV{MYSQL_LIB})
        link_directories($ENV{MYSQL_LIB})
    else()
        link_directories("/usr/lib/x86_64-linux-gnu")
    endif()
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
    
    # 链接库（区分MySQL库）
    if(WIN32)
        # Windows下链接MySQL客户端库
        target_link_libraries(${TEST_NAME} PRIVATE oldb libmysql)
    else()
        # Linux下链接MySQL客户端库
        target_link_libraries(${TEST_NAME} PRIVATE oldb mysqlclient pthread dl m)
    endif()
endforeach()