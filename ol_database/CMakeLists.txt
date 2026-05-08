# OL/ol_database/CMakeLists.txt
project(ol_database)

# ===================== 公共核心代码（数据库连接池、多态基类） =====================
# 头文件目录
set(OL_DB_PUBLIC_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include)
# 源文件
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src OL_DB_PUBLIC_SRC)

# 初始化聚合变量（收集所有子模块）
set(OL_DATABASE_INCLUDES ${OL_DB_PUBLIC_INCLUDE})
set(OL_DATABASE_SOURCES ${OL_DB_PUBLIC_SRC})
set(OL_DATABASE_LIBS ol_core)
set(OL_DATABASE_LIB_PATHS "")

# ===================== 加载子模块 =====================
if(OL_BUILD_MYSQL)
    add_subdirectory(mysql)
endif()

if(OL_BUILD_ORACLE)
    add_subdirectory(oracle)
endif()

# 既没有公共代码，也没有启用任何数据库 → 直接跳过构建
if(NOT OL_DATABASE_SOURCES)
    message(STATUS "[INFO] ol_database: No public code and no database enabled, skipping build.")
    return()
endif()

if(OL_DATABASE_LIB_PATHS)
    link_directories(${OL_DATABASE_LIB_PATHS})
endif()

# ===================== 输出路径 =====================
set(LIBRARY_OUTPUT_BASE_DIR "${CMAKE_CURRENT_BINARY_DIR}/lib/${OS_NAME}_${ARCHITECTURE}")

# ===================== 静态库 =====================
if(OL_BUILD_STATIC_LIBS)
    set(OL_DB_STATIC_DIR "${LIBRARY_OUTPUT_BASE_DIR}_static")
    if(NOT CMAKE_CONFIGURATION_TYPES)
        set(OL_DB_STATIC_DIR "${OL_DB_STATIC_DIR}-${CMAKE_BUILD_TYPE}")
    endif()

    add_library(ol_database_static STATIC ${OL_DATABASE_SOURCES})
    set_target_properties(ol_database_static PROPERTIES
        OUTPUT_NAME "ol_database"
        ARCHIVE_OUTPUT_DIRECTORY ${OL_DB_STATIC_DIR}
    )
    target_include_directories(ol_database_static PUBLIC ${OL_DATABASE_INCLUDES})
    target_link_libraries(ol_database_static PUBLIC ${OL_DATABASE_LIBS})
endif()

# ===================== 动态库 =====================
if(OL_BUILD_SHARED_LIBS)
    set(OL_DB_SHARED_DIR "${LIBRARY_OUTPUT_BASE_DIR}_shared")
    if(NOT CMAKE_CONFIGURATION_TYPES)
        set(OL_DB_SHARED_DIR "${OL_DB_SHARED_DIR}-${CMAKE_BUILD_TYPE}")
    endif()

    add_library(ol_database_shared SHARED ${OL_DATABASE_SOURCES})
    set_target_properties(ol_database_shared PROPERTIES
        OUTPUT_NAME "ol_database"
        LIBRARY_OUTPUT_DIRECTORY ${OL_DB_SHARED_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${OL_DB_SHARED_DIR}
        POSITION_INDEPENDENT_CODE ON
        WINDOWS_EXPORT_ALL_SYMBOLS ON
    )
    target_include_directories(ol_database_shared PUBLIC ${OL_DATABASE_INCLUDES})
    target_link_libraries(ol_database_shared PUBLIC ${OL_DATABASE_LIBS})
endif()

# 库别名
add_library(ol_database ALIAS ol_database_static)