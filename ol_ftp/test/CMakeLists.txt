# OL/ol_ftp/test/CMakeLists.txt
project(ol_ftp_tests)

# 测试源文件
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/. OL_FTP_TEST_SRC)

# 输出路径
if(DEFINED CMAKE_CONFIGURATION_TYPES)
    # 多配置：
    set(EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/bin/${OS_NAME}_${ARCHITECTURE}")
else()
    # 单配置：CMAKE_BUILD_TYPE
    set(EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/bin/${OS_NAME}_${ARCHITECTURE}-${CMAKE_BUILD_TYPE}")
endif()

# 生成测试程序
foreach(TEST_FILE ${OL_FTP_TEST_SRC})
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
    add_executable(${TEST_NAME} ${TEST_FILE})
    target_link_libraries(${TEST_NAME} PRIVATE ol_ftp)
endforeach()