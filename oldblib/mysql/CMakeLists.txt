# OL/oldblib/mysql/CMakeLists.txt
project(oldblib_mysql)

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

# MySQL/oldb头文件和库路径
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${MYSQL_INCLUDE}
)
link_directories(${MYSQL_LIB})

# ===================== <源文件> =====================
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src OLDB_MYSQL_SRC)
# ===================== </源文件> =====================

# ===================== <输出路径> =====================
set(LIBRARY_OUTPUT_BASE_DIR "${CMAKE_CURRENT_BINARY_DIR}/lib")
# ===================== </输出路径> =====================

# ===================== <OBJECT 库> =====================
## 生成 OBJECT 库（仅编译源文件为 .o/.obj，不生成库文件）
## 包含所有库源文件 + ftplib 源文件
set(OLDB_MYSQL_OBJECT_SOURCES ${OLDB_MYSQL_SRC})
add_library(oldb_mysql_object OBJECT ${OLDB_MYSQL_OBJECT_SOURCES})

## OBJECT 库添加动态库所需的 fPIC 选项（跨平台兼容）
set_target_properties(oldb_mysql_object PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    WINDOWS_EXPORT_ALL_SYMBOLS ON  # 继承动态库的符号导出配置
)

if(WIN32)
    target_link_libraries(oldb_mysql_object PRIVATE libmysql)  # Windows链接libmysql.lib
else()
    target_link_libraries(oldb_mysql_object PRIVATE mysqlclient)  # Linux链接mysqlclient
endif()
# ===================== </OBJECT 库> =====================

# ===================== 静态库配置 =====================
if(OLDB_MYSQL_BUILD_STATIC_LIBS)
    set(OLDB_MYSQL_STATIC_LIB_DIR "${LIBRARY_OUTPUT_BASE_DIR}/${OS_NAME}/${ARCHITECTURE}/$<CONFIG>/static")
    add_library(oldb_mysql_static STATIC  $<TARGET_OBJECTS:oldb_mysql_object>)
    set_target_properties(oldb_mysql_static PROPERTIES
        OUTPUT_NAME "oldb_mysql"
        ARCHIVE_OUTPUT_DIRECTORY ${OLDB_MYSQL_STATIC_LIB_DIR}
    )
    if(WIN32)
        target_link_libraries(oldb_mysql_static PRIVATE libmysql ws2_32)  # Windows链接libmysql.lib + Windows系统库ws2_32
    else()
        target_link_libraries(oldb_mysql_static PRIVATE mysqlclient)  # Linux链接mysqlclient
    endif()
endif()

# ===================== 动态库配置 =====================
if(OLDB_MYSQL_BUILD_SHARED_LIBS)
    set(OLDB_MYSQL_SHARED_LIB_DIR "${LIBRARY_OUTPUT_BASE_DIR}/${OS_NAME}/${ARCHITECTURE}/$<CONFIG>/shared")
    add_library(oldb_mysql_shared SHARED  $<TARGET_OBJECTS:oldb_mysql_object>)
    set_target_properties(oldb_mysql_shared PROPERTIES
        OUTPUT_NAME "oldb_mysql"
        LIBRARY_OUTPUT_DIRECTORY ${OLDB_MYSQL_SHARED_LIB_DIR} # Linux.so / macOS.dylib / Windows 动态库导入库 .lib 输出路径
        RUNTIME_OUTPUT_DIRECTORY ${OLDB_MYSQL_SHARED_LIB_DIR} # Windows.dll / 所有平台可执行文件 输出路径（当前用于动态库 .dll）
        POSITION_INDEPENDENT_CODE ON  # -fPIC（跨平台兼容）
        WINDOWS_EXPORT_ALL_SYMBOLS ON  # Windows自动导出符号
    )
    if(WIN32)
        target_link_libraries(oldb_mysql_shared PRIVATE libmysql ws2_32)  # Windows链接libmysql.lib + Windows系统库ws2_32
    else()
        target_link_libraries(oldb_mysql_shared PRIVATE mysqlclient)  # Linux链接mysqlclient
    endif()
endif()

# 静态库别名
add_library(oldb_mysql ALIAS oldb_mysql_object)

# 引入测试目录
if(OLDB_MYSQL_WITH_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()