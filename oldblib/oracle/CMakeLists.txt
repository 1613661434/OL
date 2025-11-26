# OL/oldblib/oracle/CMakeLists.txt
project(oldblib_oracle)

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
    # Linux：（默认或环境变量）
    if(DEFINED ENV{ORACLE_HOME})
        set(ORACLE_BASE $ENV{ORACLE_HOME})
    else()
        set(ORACLE_BASE "/opt/oracle/instantclient_19_18") # 默认路径
    endif()
    set(OCI_INCLUDE "$ENV{ORACLE_HOME}/rdbms/public")
    set(OCI_LIB "$ENV{ORACLE_HOME}/lib")
endif()

# 验证Oracle路径
if(NOT EXISTS ${OCI_INCLUDE})
    message(FATAL_ERROR "Oracle include directory not found: ${OCI_INCLUDE}")
endif()

# Oracle/oldb头文件和库路径
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OCI_INCLUDE}
)
link_directories(${OCI_LIB})

# ===================== <源文件> =====================
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src OLDB_ORACLE_SRC)
# ===================== </源文件> =====================

# ===================== <输出路径> =====================
set(LIBRARY_OUTPUT_BASE_DIR "${CMAKE_CURRENT_BINARY_DIR}/lib/${OS_NAME}/${ARCHITECTURE}")
# ===================== </输出路径> =====================

# ===================== <OBJECT 库> =====================
## 生成 OBJECT 库（仅编译源文件为 .o/.obj，不生成库文件）
## 包含所有库源文件 + ftplib 源文件
set(OLDB_ORACLE_OBJECT_SOURCES ${OLDB_ORACLE_SRC})
add_library(oldb_oracle_object OBJECT ${OLDB_ORACLE_OBJECT_SOURCES})

## OBJECT 库添加动态库所需的 fPIC 选项（跨平台兼容）
set_target_properties(oldb_oracle_object PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    WINDOWS_EXPORT_ALL_SYMBOLS ON  # 继承动态库的符号导出配置
)

if(WIN32)
    target_link_libraries(oldb_oracle_object PRIVATE oci)  # Windows链接oci.lib
else()
    target_link_libraries(oldb_oracle_object PRIVATE clntsh)  # Linux链接clntsh
endif()
# ===================== </OBJECT 库> =====================

# ===================== 静态库配置 =====================
if(OLDB_ORACLE_BUILD_STATIC_LIBS)
    set(OLDB_ORACLE_STATIC_LIB_DIR "${LIBRARY_OUTPUT_BASE_DIR}/static")
    # 单配置：CMAKE_BUILD_TYPE
    if(NOT DEFINED CMAKE_CONFIGURATION_TYPES)
        set(OLDB_ORACLE_STATIC_LIB_DIR "${OLDB_ORACLE_STATIC_LIB_DIR}/${CMAKE_BUILD_TYPE}")
    endif()
    add_library(oldb_oracle_static STATIC  $<TARGET_OBJECTS:oldb_oracle_object>)
    set_target_properties(oldb_oracle_static PROPERTIES
        OUTPUT_NAME "oldb_oracle"
        ARCHIVE_OUTPUT_DIRECTORY ${OLDB_ORACLE_STATIC_LIB_DIR}
    )
    if(WIN32)
        target_link_libraries(oldb_oracle_static PRIVATE oci ws2_32)  # Windows链接oci.lib + Windows系统库ws2_32
    else()
        target_link_libraries(oldb_oracle_static PRIVATE clntsh)  # Linux链接clntsh
    endif()
endif()

# ===================== 动态库配置 =====================
if(OLDB_ORACLE_BUILD_SHARED_LIBS)
    set(OLDB_ORACLE_SHARED_LIB_DIR "${LIBRARY_OUTPUT_BASE_DIR}/shared")
    # 单配置：CMAKE_BUILD_TYPE
    if(NOT DEFINED CMAKE_CONFIGURATION_TYPES)
        set(OLDB_ORACLE_SHARED_LIB_DIR "${OLDB_ORACLE_SHARED_LIB_DIR}/${CMAKE_BUILD_TYPE}")
    endif()
    add_library(oldb_oracle_shared SHARED  $<TARGET_OBJECTS:oldb_oracle_object>)
    set_target_properties(oldb_oracle_shared PROPERTIES
        OUTPUT_NAME "oldb_oracle"
        LIBRARY_OUTPUT_DIRECTORY ${OLDB_ORACLE_SHARED_LIB_DIR} # Linux.so / macOS.dylib / Windows 动态库导入库 .lib 输出路径
        RUNTIME_OUTPUT_DIRECTORY ${OLDB_ORACLE_SHARED_LIB_DIR} # Windows.dll / 所有平台可执行文件 输出路径（当前用于动态库 .dll）
        POSITION_INDEPENDENT_CODE ON  # -fPIC（跨平台兼容）
        WINDOWS_EXPORT_ALL_SYMBOLS ON  # Windows自动导出符号
    )
    if(WIN32)
        target_link_libraries(oldb_oracle_shared PRIVATE oci ws2_32)  # Windows链接oci.lib + Windows系统库ws2_32
    else()
        target_link_libraries(oldb_oracle_shared PRIVATE clntsh)  # Linux链接clntsh
    endif()
endif()

# 静态库别名
add_library(oldb_oracle ALIAS oldb_oracle_object)

# 引入测试目录
if(OLDB_ORACLE_WITH_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()