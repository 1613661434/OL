cmake_minimum_required(VERSION 3.10)
project(oldblib_mysql)

# C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 构建类型默认值
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Debug模式(默认注释，不启用)
# set(CMAKE_BUILD_TYPE Debug)

# 平台编译选项
if(MSVC)
    # Visual Studio 选项
    set(COMMON_FLAGS "/W3 /D_CRT_SECURE_NO_WARNINGS")
else()
    # MinGW/GCC 选项
    set(COMMON_FLAGS "-Wall -D_GLIBCXX_USE_CXX11_ABI=1")
endif()

# 应用优化/调试选项
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 ${COMMON_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g ${COMMON_FLAGS}")

# MySQL环境检查（区分平台）
if(WIN32)
    # Windows：MySQL路径（默认或环境变量）
    if(DEFINED ENV{MYSQL_HOME})
        set(MYSQL_BASE $ENV{MYSQL_HOME})
    else()
        set(MYSQL_BASE "C:/Program Files/MySQL/MySQL Server 8.0")  # 默认路径
    endif()
    set(MYSQL_INCLUDE "${MYSQL_BASE}/include")
    set(MYSQL_LIB "${MYSQL_BASE}/lib")
else()
    # Linux：检查MySQL环境变量或默认路径
    if(DEFINED ENV{MYSQL_HOME})
        set(MYSQL_BASE $ENV{MYSQL_HOME})
    else()
        set(MYSQL_BASE "/usr/local/mysql")  # 常见的默认安装路径
    endif()
    set(MYSQL_INCLUDE "${MYSQL_BASE}/include")
    set(MYSQL_LIB "${MYSQL_BASE}/lib")
    
    # 检查是否存在mysql_config（Linux特定）
    find_program(MYSQL_CONFIG mysql_config)
    if(MYSQL_CONFIG)
        execute_process(COMMAND ${MYSQL_CONFIG} --cflags OUTPUT_VARIABLE MYSQL_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
        execute_process(COMMAND ${MYSQL_CONFIG} --libs OUTPUT_VARIABLE MYSQL_LDFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MYSQL_CFLAGS}")
    else()
        message(WARNING "mysql_config not found, using default paths")
    endif()
endif()

# 验证MySQL路径
if(NOT EXISTS ${MYSQL_INCLUDE})
    message(FATAL_ERROR "MySQL include directory not found: ${MYSQL_INCLUDE}")
endif()

# MySQL头文件和库路径
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${MYSQL_INCLUDE}
)
link_directories(${MYSQL_LIB})

# 源文件（假设已修改为使用MySQL API）
set(OLDB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/ol_mysql.cpp)

# 输出路径（区分平台）
if(WIN32)
    set(OLDB_STATIC_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/win32/static)
    set(OLDB_SHARED_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/win32/shared)
else()
    set(OLDB_STATIC_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/linux/static)
    set(OLDB_SHARED_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/linux/shared)
endif()
file(MAKE_DIRECTORY ${OLDB_STATIC_LIB_DIR} ${OLDB_SHARED_LIB_DIR})

# ===================== 静态库配置 =====================
add_library(oldb_static STATIC ${OLDB_SRC})
set_target_properties(oldb_static PROPERTIES
    OUTPUT_NAME "oldb"
    ARCHIVE_OUTPUT_DIRECTORY ${OLDB_STATIC_LIB_DIR}
)
if(WIN32)
    target_link_libraries(oldb_static PRIVATE libmysql)  # Windows链接libmysql.lib
else()
    target_link_libraries(oldb_static PRIVATE mysqlclient)  # Linux链接mysqlclient
endif()

# ===================== 动态库配置 =====================
add_library(oldb_shared SHARED ${OLDB_SRC})
set_target_properties(oldb_shared PROPERTIES
    OUTPUT_NAME "oldb"
    LIBRARY_OUTPUT_DIRECTORY ${OLDB_SHARED_LIB_DIR}  # Linux .so
    RUNTIME_OUTPUT_DIRECTORY ${OLDB_SHARED_LIB_DIR}  # Windows .dll
    POSITION_INDEPENDENT_CODE ON
    WINDOWS_EXPORT_ALL_SYMBOLS ON  # Windows自动导出符号
)
if(WIN32)
    target_link_libraries(oldb_shared PRIVATE libmysql)  # Windows链接libmysql.lib
else()
    target_link_libraries(oldb_shared PRIVATE mysqlclient)  # Linux链接mysqlclient
endif()

# 静态库别名
add_library(oldb ALIAS oldb_static)

# 引入测试目录
add_subdirectory(test)