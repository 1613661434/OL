# OL/ol_database/mysql/CMakeLists.txt
project(ol_database_mysql)

# MySQL 模块配置
set(MYSQL_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src MYSQL_SRC)

# 聚合到顶层库
list(APPEND OL_DATABASE_INCLUDES ${MYSQL_INCLUDE})
list(APPEND OL_DATABASE_SOURCES ${MYSQL_SRC})

# ===================== MySQL 依赖 =====================
if(DEFINED ENV{MYSQL_HOME})
    # 优先使用环境变量 MYSQL_HOME
    set(MYSQL_HOME $ENV{MYSQL_HOME} CACHE PATH "MySQL installation directory")
    list(APPEND OL_DATABASE_INCLUDES ${MYSQL_HOME}/include)
    list(APPEND OL_DATABASE_LIB_PATHS ${MYSQL_HOME}/lib)

    link_directories(${OL_DATABASE_LIB_PATHS})

    if(WIN32)
        list(APPEND OL_DATABASE_LIBS libmysql ws2_32)
    else()
        list(APPEND OL_DATABASE_LIBS mysqlclient)
    endif()
else()
    # 无环境变量 → 自动查找 MySQL
    find_package(MySQL REQUIRED)
    list(APPEND OL_DATABASE_INCLUDES ${MYSQL_INCLUDE_DIRS})
    list(APPEND OL_DATABASE_LIBS ${MYSQL_LIBRARIES})

    # Windows 补充系统库
    if(WIN32)
        list(APPEND OL_DATABASE_LIBS ws2_32)
    endif()
endif()

# 传递父作用域
set(OL_DATABASE_INCLUDES ${OL_DATABASE_INCLUDES} PARENT_SCOPE)
set(OL_DATABASE_SOURCES ${OL_DATABASE_SOURCES} PARENT_SCOPE)
set(OL_DATABASE_LIB_PATHS ${OL_DATABASE_LIB_PATHS} PARENT_SCOPE)
set(OL_DATABASE_LIBS ${OL_DATABASE_LIBS} PARENT_SCOPE)

# 独立测试
if(OL_MYSQL_WITH_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()