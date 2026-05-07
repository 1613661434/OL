# OL/oldblib/oracle/test/CMakeLists.txt
project(oldblib_oracle_tests)

# 链接主库
link_libraries(oldb_oracle)

# 测试源文件
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/. OLDB_TEST_SRC)

# 输出路径（区分平台）
if(DEFINED CMAKE_CONFIGURATION_TYPES)
    # 多配置：
    set(EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/bin/${OS_NAME}/${ARCHITECTURE}")
else()
    # 单配置：CMAKE_BUILD_TYPE
    set(EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/bin/${OS_NAME}/${ARCHITECTURE}/${CMAKE_BUILD_TYPE}")
endif()

# 生成测试程序
foreach(TEST_FILE ${OLDB_TEST_SRC})
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
    add_executable(${TEST_NAME} ${TEST_FILE})
endforeach()