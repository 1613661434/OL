cmake_minimum_required(VERSION 3.10)
project(ollib)

# 指定C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 构建类型默认值
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Debug模式(默认注释，不启用)
# set(CMAKE_BUILD_TYPE Debug)

# 平台区分：编译选项
if(MSVC)
    # Visual Studio 编译器选项
    set(RELEASE_FLAGS "/O2 /W3")
    set(DEBUG_FLAGS "/O0 /Zi /W3")  # /Zi 生成调试信息（MSVC）
else()
    # MinGW/GCC 编译器选项
    set(RELEASE_FLAGS "-O2 -Wall")  # -Wall 开启警告（GCC风格）
    set(DEBUG_FLAGS "-O0 -g -Wall")  # -g 生成调试信息（GCC风格）
endif()

# 应用编译选项
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${RELEASE_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${DEBUG_FLAGS}")

# 头文件路径（通用）
set(OLLIB_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${OLLIB_INCLUDE})

# 源文件
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src OLLIB_SRC)

# 第三方库ftplib路径（区分平台）
if(WIN32)
else()
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_party/ftplib/include)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_party/ftplib/lib)
endif()

# 输出路径：区分平台（linux/win32）
if(WIN32)
    set(OLLIB_STATIC_LIB ${CMAKE_CURRENT_SOURCE_DIR}/lib/win32/static)
    set(OLLIB_SHARED_LIB ${CMAKE_CURRENT_SOURCE_DIR}/lib/win32/shared)
else()
    set(OLLIB_STATIC_LIB ${CMAKE_CURRENT_SOURCE_DIR}/lib/linux/static)
    set(OLLIB_SHARED_LIB ${CMAKE_CURRENT_SOURCE_DIR}/lib/linux/shared)
endif()
file(MAKE_DIRECTORY ${OLLIB_STATIC_LIB} ${OLLIB_SHARED_LIB})

# ===================== 静态库配置 =====================
add_library(ol_static STATIC ${OLLIB_SRC})
set_target_properties(ol_static PROPERTIES
    OUTPUT_NAME "ol"
    ARCHIVE_OUTPUT_DIRECTORY ${OLLIB_STATIC_LIB}  # 静态库路径（.a/.lib）
)

# 非Windows平台添加ftplib源码
if(NOT WIN32)
target_link_libraries(ol_static PRIVATE ftp)  # 链接ftplib
endif()

# ===================== 动态库配置 =====================
# 基础源文件列表（不含ftplib）
set(OL_SHARED_SOURCES ${OLLIB_SRC})

# 非Windows平台添加ftplib源码
if(NOT WIN32)
    list(APPEND OL_SHARED_SOURCES 
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/ftplib/src/ftplib.c
    )
endif()

# 创建动态库（根据平台条件包含不同源文件）
add_library(ol_shared SHARED 
    ${OL_SHARED_SOURCES}
)

set_target_properties(ol_shared PROPERTIES
    OUTPUT_NAME "ol"
    LIBRARY_OUTPUT_DIRECTORY ${OLLIB_SHARED_LIB}  # Linux .so路径
    RUNTIME_OUTPUT_DIRECTORY ${OLLIB_SHARED_LIB}  # Windows .dll路径
    POSITION_INDEPENDENT_CODE ON  # 强制-fPIC（跨平台）
    WINDOWS_EXPORT_ALL_SYMBOLS ON  # Windows自动导出符号
)

# 非Windows平台可能需要链接的额外库（如系统ftp库）
if(NOT WIN32)
    target_link_libraries(ol_shared PRIVATE ftp)
endif()

# 静态库别名
add_library(ol ALIAS ol_static)

# 引入测试目录
add_subdirectory(test)