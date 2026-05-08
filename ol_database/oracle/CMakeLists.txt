# OL/oldblib/oracle/CMakeLists.txt
project(ol_database_oracle)

# Oracle 模块配置
set(ORACLE_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src ORACLE_SRC)

# 聚合到顶层库
list(APPEND OL_DATABASE_INCLUDES ${ORACLE_INCLUDE})
list(APPEND OL_DATABASE_SOURCES ${ORACLE_SRC})

# ===================== Oracle 依赖 =====================
if(DEFINED ENV{ORACLE_HOME})
    set(ORACLE_HOME $ENV{ORACLE_HOME} CACHE PATH "Oracle installation directory")
    if(WIN32)
        list(APPEND OL_DATABASE_INCLUDES ${ORACLE_HOME}/sdk/include)
        link_directories(${ORACLE_HOME}/sdk/lib/msvc)
        list(APPEND OL_DATABASE_LIBS oci ws2_32)
    else()
        list(APPEND OL_DATABASE_INCLUDES ${ORACLE_HOME}/rdbms/public)
        link_directories(${ORACLE_HOME}/lib)
        list(APPEND OL_DATABASE_LIBS clntsh)
    endif()
else()
    message(FATAL_ERROR "ORACLE_HOME environment variable not found!")
endif()

# 传递父作用域
set(OL_DATABASE_INCLUDES ${OL_DATABASE_INCLUDES} PARENT_SCOPE)
set(OL_DATABASE_SOURCES ${OL_DATABASE_SOURCES} PARENT_SCOPE)
set(OL_DATABASE_LIBS ${OL_DATABASE_LIBS} PARENT_SCOPE)

# 独立测试
if(OL_ORACLE_WITH_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()