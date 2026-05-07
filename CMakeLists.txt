# OL/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(ol VERSION 0.1.0 LANGUAGES CXX)

# 避免在源树创建
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "Error: In-source builds are not allowed.
    Please create a separate directory for build files, such as a 'build' directory.")
endif()

# 指定C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 确保CMAKE_BUILD_TYPE为单配置生成器指定默认值
if((NOT DEFINED CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "") AND NOT DEFINED CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING
        "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel Coverage."
        FORCE) # FORCE 强制覆盖已定义但为空的情况
endif()

# ===================== <Options> =====================
# 全局警告开关
option(ENABLE_WARNINGS "Enable compiler warnings" OFF)

# 全局库类型开关（控制所有模块：静态库 / 动态库）
option(OL_BUILD_STATIC_LIBS "Build all modules as static libraries (GLOBAL)" ON)
option(OL_BUILD_SHARED_LIBS "Build all modules as shared libraries (GLOBAL)" ON)

# 功能模块总开关
option(OL_BUILD_CORE "Build core module" ON)
option(OL_BUILD_FTP "Build FTP module" OFF)
option(OL_BUILD_NETWORK "Build network module" OFF)
option(OL_BUILD_DATABASE "Build database module" OFF)

## 数据库子模块开关
option(OL_BUILD_MYSQL "Build MySQL support" ON)
option(OL_BUILD_ORACLE "Build Oracle support" OFF)

## 模块独立测试开关
option(OL_CORE_WITH_TESTS "Build and run core tests" ON)
option(OL_FTP_WITH_TESTS "Build and run FTP tests" OFF)
option(OL_NETWORK_WITH_TESTS "Build and run network tests" OFF)
option(OL_MYSQL_WITH_TESTS "Build and run MySQL tests" OFF)
option(OL_ORACLE_WITH_TESTS "Build and run Oracle tests" OFF)
# ===================== </Options> =====================

# 跨平台字符集配置：统一使用UTF-8
if(MSVC)
    # Windows (MSVC)：强制UTF-8编码，禁用BOM
    add_compile_options(/utf-8)

    # 禁用安全函数警告、使用Unicode字符集
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS UNICODE _UNICODE)
else()
    # Linux/macOS (GCC/Clang)：默认支持UTF-8，添加通用警告选项
    add_compile_options(-Wno-write-strings -D_GLIBCXX_USE_CXX11_ABI=1)

    # 添加额外的跨平台编译优化
    add_compile_options(-finput-charset=UTF-8) # 显式指定输入编码为UTF-8
endif()

# 是否启用警告
if(ENABLE_WARNINGS)
    if(MSVC)
        add_compile_options("/W3")
    else()
        add_compile_options("-Wall")
    endif()
endif()

# 确定操作系统名称 (例如: windows, linux)
if(WIN32)
    set(OS_NAME "windows")
elseif(UNIX AND NOT APPLE)
    set(OS_NAME "linux")
elseif(APPLE)
    set(OS_NAME "macos")
endif()

# 确定架构 (例如: x64, x86)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(ARCHITECTURE "x64")
else()
    set(ARCHITECTURE "x86")
endif()

# 添加子目录
if(OL_BUILD_CORE)
    add_subdirectory(ol_core)
endif()

if(OL_BUILD_FTP)
    add_subdirectory(ol_ftp)
endif()

if(OL_BUILD_NETWORK)
    add_subdirectory(ol_network)
endif()

if(OL_BUILD_DATABASE)
    add_subdirectory(ol_database)
endif()