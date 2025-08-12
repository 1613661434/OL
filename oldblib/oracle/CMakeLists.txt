cmake_minimum_required(VERSION 3.10)
project(oldblib_oracle)

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
if(WIN32)
    set(COMMON_FLAGS "/W3 /D_CRT_SECURE_NO_WARNINGS")  # MSVC选项
else()
    set(COMMON_FLAGS "-Wno-write-strings -D_GLIBCXX_USE_CXX11_ABI=1")  # GCC选项
endif()

# 应用优化/调试选项
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 ${COMMON_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g ${COMMON_FLAGS}")

# Oracle环境检查（区分平台）
if(WIN32)
    # Windows：Oracle Instant Client路径（默认或环境变量）
    if(DEFINED ENV{ORACLE_HOME})
        set(ORACLE_BASE $ENV{ORACLE_HOME})
    else()
        set(ORACLE_BASE "C:/oracle/instantclient_19_18")  # 默认路径
    endif()
    set(OCI_INCLUDE "${ORACLE_BASE}/sdk/include")
    set(OCI_LIB "${ORACLE_BASE}/sdk/lib/msvc")  # MSVC库路径
else()
    # Linux：原配置
    if(NOT DEFINED ENV{ORACLE_HOME})
        message(FATAL_ERROR "请设置ORACLE_HOME环境变量")
    endif()
    set(OCI_INCLUDE "$ENV{ORACLE_HOME}/rdbms/public")
    set(OCI_LIB "$ENV{ORACLE_HOME}/lib")
endif()

# Oracle头文件和库路径
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OCI_INCLUDE}
)
link_directories(${OCI_LIB})

# 源文件
set(OLDBA_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/ol_oci.cpp)

# 输出路径（区分平台）
if(WIN32)
    set(OLDBA_STATIC_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/win32/static)
    set(OLDBA_SHARED_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/win32/shared)
else()
    set(OLDBA_STATIC_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/linux/static)
    set(OLDBA_SHARED_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/linux/shared)
endif()
file(MAKE_DIRECTORY ${OLDBA_STATIC_LIB_DIR} ${OLDBA_SHARED_LIB_DIR})

# ===================== 静态库配置 =====================
add_library(oldba_static STATIC ${OLDBA_SRC})
set_target_properties(oldba_static PROPERTIES
    OUTPUT_NAME "oldb"
    ARCHIVE_OUTPUT_DIRECTORY ${OLDBA_STATIC_LIB_DIR}
)
if(WIN32)
    target_link_libraries(oldba_static PRIVATE oci)  # Windows链接oci.lib
else()
    target_link_libraries(oldba_static PRIVATE clntsh)  # Linux链接clntsh
endif()

# ===================== 动态库配置 =====================
add_library(oldba_shared SHARED ${OLDBA_SRC})
set_target_properties(oldba_shared PROPERTIES
    OUTPUT_NAME "oldb"
    LIBRARY_OUTPUT_DIRECTORY ${OLDBA_SHARED_LIB_DIR}  # Linux .so
    RUNTIME_OUTPUT_DIRECTORY ${OLDBA_SHARED_LIB_DIR}  # Windows .dll
    POSITION_INDEPENDENT_CODE ON
    WINDOWS_EXPORT_ALL_SYMBOLS ON  # Windows自动导出符号
)
if(WIN32)
    target_link_libraries(oldba_shared PRIVATE oci)  # Windows链接oci.lib
else()
    target_link_libraries(oldba_shared PRIVATE clntsh)  # Linux链接clntsh
endif()

# 静态库别名
add_library(oldba ALIAS oldba_static)

# 引入测试目录
add_subdirectory(test)