# OL/ol_ftp/CMakeLists.txt
project(ol_ftp)

# 头文件目录
set(OL_FTP_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include)
# ftplib头文件
set(FTPLIB_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/third_party/ftplib/include)

# 源文件
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src OL_FTP_SRC)
# ftplib源文件
set(FTPLIB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/ftplib/src/ftplib.c)

# ===================== 输出路径 =====================
set(LIBRARY_OUTPUT_BASE_DIR "${CMAKE_CURRENT_BINARY_DIR}/lib/${OS_NAME}_${ARCHITECTURE}")

# ===================== 静态库 =====================
if(OL_BUILD_STATIC_LIBS)
    set(OL_FTP_STATIC_DIR "${LIBRARY_OUTPUT_BASE_DIR}_static")

	# 单配置：CMAKE_BUILD_TYPE
    if(NOT CMAKE_CONFIGURATION_TYPES)
        set(OL_FTP_STATIC_DIR "${OL_FTP_STATIC_DIR}-${CMAKE_BUILD_TYPE}")
    endif()

    add_library(ol_ftp_static STATIC ${OL_FTP_SRC} ${FTPLIB_SRC})
    set_target_properties(ol_ftp_static PROPERTIES
        OUTPUT_NAME "ol_ftp"
        ARCHIVE_OUTPUT_DIRECTORY ${OL_FTP_STATIC_DIR}
    )

    target_compile_definitions(ol_ftp_static PRIVATE FTPLIB_STATIC)

    target_include_directories(ol_ftp_static
        PUBLIC $<BUILD_INTERFACE:${OL_FTP_INCLUDE}>
        PUBLIC $<BUILD_INTERFACE:${FTPLIB_INCLUDE}>
    )

    target_link_libraries(ol_ftp_static PUBLIC ol_core)
	if(WIN32)
		target_link_libraries(ol_ftp_static PUBLIC ws2_32)
	endif()
endif()

# ===================== 动态库 =====================
if(OL_BUILD_SHARED_LIBS)
    set(OL_FTP_SHARED_DIR "${LIBRARY_OUTPUT_BASE_DIR}_shared")

	# 单配置：CMAKE_BUILD_TYPE
    if(NOT CMAKE_CONFIGURATION_TYPES)
        set(OL_FTP_SHARED_DIR "${OL_FTP_SHARED_DIR}-${CMAKE_BUILD_TYPE}")
    endif()

    add_library(ol_ftp_shared SHARED ${OL_FTP_SRC} ${FTPLIB_SRC})
    set_target_properties(ol_ftp_shared PROPERTIES
        OUTPUT_NAME "ol_ftp"
        LIBRARY_OUTPUT_DIRECTORY ${OL_FTP_SHARED_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${OL_FTP_SHARED_DIR}
        POSITION_INDEPENDENT_CODE ON
        WINDOWS_EXPORT_ALL_SYMBOLS ON
    )

    target_compile_definitions(ol_ftp_shared PRIVATE BUILDING_LIBRARY)

    target_include_directories(ol_ftp_shared
        PUBLIC $<BUILD_INTERFACE:${OL_FTP_INCLUDE}>
        PUBLIC $<BUILD_INTERFACE:${FTPLIB_INCLUDE}>
    )

    target_link_libraries(ol_ftp_shared PUBLIC ol_core)
	if(WIN32)
		target_link_libraries(ol_ftp_shared PUBLIC ws2_32)
	endif()
endif()

# 别名：指向OBJECT库
add_library(ol_ftp ALIAS ol_ftp_static)

# 引入测试目录
if(OL_FTP_WITH_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()