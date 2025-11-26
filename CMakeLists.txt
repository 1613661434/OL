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
        FORCE)  # FORCE 强制覆盖已定义但为空的情况
endif()

# ===================== <option> =====================
# 统一：控制是否开启警告（默认开启）
option(ENABLE_WARNINGS "Enable compiler warnings (recommended)" ON)

# OL库相关
option(OL_WITH_TESTS "Compile and (for ol_check) run OL test executables" OFF)
option(OL_BUILD_STATIC_LIBS "Build ol_lib as a static library." ON)
option(OL_BUILD_SHARED_LIBS "Build ol_lib as a shared library." ON)

# OLDB库相关
## MYSQL
option(OLDB_MYSQL_WITH_TESTS "Compile and (for oldb_check) run OLDB test executables" OFF)
option(OLDB_MYSQL_BUILD_STATIC_LIBS "Build oldb_lib as a static library." OFF)
option(OLDB_MYSQL_BUILD_SHARED_LIBS "Build oldb_lib as a shared library." OFF)

## ORACLE
option(OLDB_ORACLE_WITH_TESTS "Compile and (for oldb_check) run OLDB test executables" OFF)
option(OLDB_ORACLE_BUILD_STATIC_LIBS "Build oldb_lib as a static library." OFF)
option(OLDB_ORACLE_BUILD_SHARED_LIBS "Build oldb_lib as a shared library." OFF)
# ===================== </option> =====================

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
    add_compile_options(-finput-charset=UTF-8)  # 显式指定输入编码为UTF-8
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
if(OL_WITH_TESTS OR OL_BUILD_STATIC_LIBS OR OL_BUILD_SHARED_LIBS)
    add_subdirectory(ollib)
endif()

if(OLDB_MYSQL_WITH_TESTS OR OLDB_MYSQL_BUILD_STATIC_LIBS OR OLDB_MYSQL_BUILD_SHARED_LIBS)
    add_subdirectory(oldblib/mysql)
endif()

if(OLDB_ORACLE_WITH_TESTS OR OLDB_ORACLE_BUILD_STATIC_LIBS OR OLDB_ORACLE_BUILD_SHARED_LIBS)
    add_subdirectory(oldblib/oracle)
endif()