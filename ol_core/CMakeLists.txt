# OL/ol_core/CMakeLists.txt
project(ol_core)

# 头文件目录
set(OL_CORE_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include)
# 源文件
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src OL_CORE_SRC)

# Linux平台
if(UNIX AND NOT APPLE)
    ## 链接pthread库
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()

# ===================== <输出路径> =====================
set(LIBRARY_OUTPUT_BASE_DIR "${CMAKE_CURRENT_BINARY_DIR}/lib/${OS_NAME}/${ARCHITECTURE}")
# ===================== </输出路径> =====================

# ===================== <OBJECT 库> =====================
## 生成 OBJECT 库（仅编译源文件为 .o/.obj，不生成库文件）
set(OL_OBJECT_SOURCES ${OL_CORE_SRC})

add_library(ol_core_object OBJECT ${OL_OBJECT_SOURCES})

## OBJECT 库添加动态库所需的 fPIC 选项（跨平台兼容）
set_target_properties(ol_core_object PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    WINDOWS_EXPORT_ALL_SYMBOLS ON # 继承动态库的符号导出配置
)

target_include_directories(ol_core_object
    PUBLIC $<BUILD_INTERFACE:${OL_CORE_INCLUDE}>
)
# ===================== </OBJECT 库> =====================

# ===================== 静态库配置 =====================
if(OL_BUILD_STATIC_LIBS)
    set(OL_CORE_STATIC_LIB_DIR "${LIBRARY_OUTPUT_BASE_DIR}/static")

    # 单配置：CMAKE_BUILD_TYPE
    if(NOT DEFINED CMAKE_CONFIGURATION_TYPES)
        set(OL_CORE_STATIC_LIB_DIR "${OL_CORE_STATIC_LIB_DIR}/${CMAKE_BUILD_TYPE}")
    endif()

    add_library(ol_static STATIC $<TARGET_OBJECTS:ol_core_object>)
    set_target_properties(ol_static PROPERTIES
        OUTPUT_NAME "ol_core"
        ARCHIVE_OUTPUT_DIRECTORY ${OL_CORE_STATIC_LIB_DIR} # 静态库路径（.a/.lib）
    )
endif()

# ===================== 动态库配置 =====================
if(OL_BUILD_SHARED_LIBS)
    set(OL_CORE_SHARED_LIB_DIR "${LIBRARY_OUTPUT_BASE_DIR}/shared")

    # 单配置：CMAKE_BUILD_TYPE
    if(NOT DEFINED CMAKE_CONFIGURATION_TYPES)
        set(OL_CORE_SHARED_LIB_DIR "${OL_CORE_SHARED_LIB_DIR}/${CMAKE_BUILD_TYPE}")
    endif()

    add_library(ol_shared SHARED $<TARGET_OBJECTS:ol_core_object>)
    set_target_properties(ol_shared PROPERTIES
        OUTPUT_NAME "ol_core"
        LIBRARY_OUTPUT_DIRECTORY ${OL_CORE_SHARED_LIB_DIR} # Linux.so / macOS.dylib / Windows 动态库导入库 .lib 输出路径
        RUNTIME_OUTPUT_DIRECTORY ${OL_CORE_SHARED_LIB_DIR} # Windows.dll / 所有平台可执行文件 输出路径（当前用于动态库 .dll）
        POSITION_INDEPENDENT_CODE ON # -fPIC（跨平台兼容）
        WINDOWS_EXPORT_ALL_SYMBOLS ON # Windows自动导出符号
    )
endif()

# 别名：指向 OBJECT 库
add_library(ol_core ALIAS ol_core_object)

# 引入测试目录
if(OL_CORE_WITH_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()