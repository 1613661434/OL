# OL/ol_network/CMakeLists.txt
project(ol_network)

# ===================== 平台判断：仅支持 Linux =====================
if(NOT (UNIX AND NOT APPLE))
    message(STATUS "[INFO] ol_network module is only supported on Linux, skipping build...")
    return()
endif()

# 头文件目录
set(OL_NETWORK_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# 源文件
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src OL_NETWORK_SRC)

# 输出路径
set(LIBRARY_OUTPUT_BASE_DIR "${CMAKE_CURRENT_BINARY_DIR}/lib/${OS_NAME}_${ARCHITECTURE}")

# ===================== 静态库 =====================
if(OL_BUILD_STATIC_LIBS)
    set(OL_NETWORK_STATIC_DIR "${LIBRARY_OUTPUT_BASE_DIR}_static")
    if(NOT CMAKE_CONFIGURATION_TYPES)
        set(OL_NETWORK_STATIC_DIR "${OL_NETWORK_STATIC_DIR}-${CMAKE_BUILD_TYPE}")
    endif()

    add_library(ol_network_static STATIC ${OL_NETWORK_SRC})
    set_target_properties(ol_network_static PROPERTIES
        OUTPUT_NAME "ol_network"
        ARCHIVE_OUTPUT_DIRECTORY ${OL_NETWORK_STATIC_DIR}
    )

    target_include_directories(ol_network_static
        PUBLIC $<BUILD_INTERFACE:${OL_NETWORK_INCLUDE}>
    )

    target_link_libraries(ol_network_static PUBLIC ol_core pthread)
endif()

# ===================== 动态库 =====================
if(OL_BUILD_SHARED_LIBS)
    set(OL_NETWORK_SHARED_DIR "${LIBRARY_OUTPUT_BASE_DIR}_shared")
    if(NOT CMAKE_CONFIGURATION_TYPES)
        set(OL_NETWORK_SHARED_DIR "${OL_NETWORK_SHARED_DIR}-${CMAKE_BUILD_TYPE}")
    endif()

    add_library(ol_network_shared SHARED ${OL_NETWORK_SRC})
    set_target_properties(ol_network_shared PROPERTIES
        OUTPUT_NAME "ol_network"
        LIBRARY_OUTPUT_DIRECTORY ${OL_NETWORK_SHARED_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${OL_NETWORK_SHARED_DIR}
        POSITION_INDEPENDENT_CODE ON
        WINDOWS_EXPORT_ALL_SYMBOLS ON
    )

    target_include_directories(ol_network_shared
        PUBLIC $<BUILD_INTERFACE:${OL_NETWORK_INCLUDE}>
    )

    target_link_libraries(ol_network_shared PUBLIC ol_core pthread)
endif()

# 库别名（默认指向静态库）
add_library(ol_network ALIAS ol_network_static)

# 测试模块
if(OL_NETWORK_WITH_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()