# OL/ollib/CMakeLists.txt
project(ollib)

# 头文件路径（通用）
set(OLLIB_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${OLLIB_INCLUDE})

# ===================== <源文件> =====================
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src OLLIB_SRC)

## Linux平台
if(UNIX AND NOT APPLE)
    ### 加入网络库源文件
    aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/ol_net OLLIB_NET_SRC)
    list(APPEND OLLIB_SRC ${OLLIB_NET_SRC})

    ### 第三方库ftplib配置,仅添加头文件路径（源文件后续直接加入OBJECT库）
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_party/ftplib/include)

    ### 链接pthread库
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()
# ===================== </源文件> =====================

# ===================== <输出路径> =====================
set(LIBRARY_OUTPUT_BASE_DIR "${CMAKE_CURRENT_BINARY_DIR}/lib")
# ===================== </输出路径> =====================

# ===================== <OBJECT 库> =====================
## 生成 OBJECT 库（仅编译源文件为 .o/.obj，不生成库文件）
## 包含所有库源文件 + ftplib 源文件
set(OL_OBJECT_SOURCES ${OLLIB_SRC})
if(UNIX AND NOT APPLE) # 仅Linux平台需要ftplib
    ### 直接加入ftplib源文件
    list(APPEND OL_OBJECT_SOURCES 
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/ftplib/src/ftplib.c
    )
endif()
add_library(ol_object OBJECT ${OL_OBJECT_SOURCES})

## OBJECT 库添加动态库所需的 fPIC 选项（跨平台兼容）
set_target_properties(ol_object PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    WINDOWS_EXPORT_ALL_SYMBOLS ON  # 继承动态库的符号导出配置
)
# ===================== </OBJECT 库> =====================

# ===================== 静态库配置 =====================
if(OL_BUILD_STATIC_LIBS)
    set(OLLIB_STATIC_LIB_DIR "${LIBRARY_OUTPUT_BASE_DIR}/${OS_NAME}/${ARCHITECTURE}/$<CONFIG>/static")
    add_library(ol_static STATIC $<TARGET_OBJECTS:ol_object>)
    set_target_properties(ol_static PROPERTIES
        OUTPUT_NAME "ol"
        ARCHIVE_OUTPUT_DIRECTORY ${OLLIB_STATIC_LIB_DIR}  # 静态库路径（.a/.lib）
    )
endif()

# ===================== 动态库配置 =====================
if(OL_BUILD_SHARED_LIBS)
    set(OLLIB_SHARED_LIB_DIR "${LIBRARY_OUTPUT_BASE_DIR}/${OS_NAME}/${ARCHITECTURE}/$<CONFIG>/shared")
    add_library(ol_shared SHARED $<TARGET_OBJECTS:ol_object>)
    set_target_properties(ol_shared PROPERTIES
        OUTPUT_NAME "ol"
        LIBRARY_OUTPUT_DIRECTORY ${OLLIB_SHARED_LIB_DIR} # Linux.so / macOS.dylib / Windows 动态库导入库 .lib 输出路径
        RUNTIME_OUTPUT_DIRECTORY ${OLLIB_SHARED_LIB_DIR} # Windows.dll / 所有平台可执行文件 输出路径（当前用于动态库 .dll）
        POSITION_INDEPENDENT_CODE ON  # -fPIC（跨平台兼容）
        WINDOWS_EXPORT_ALL_SYMBOLS ON  # Windows自动导出符号
    )
endif()

# 别名：指向 OBJECT 库
add_library(ol ALIAS ol_object)

# 引入测试目录
if(OL_WITH_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()