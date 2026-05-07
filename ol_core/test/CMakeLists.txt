# OL/ol_core/test/CMakeLists.txt
project(ol_core_tests)

# 测试源文件
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/. OL_CORE_TEST_SRC)

# 过滤非Linux平台的测试文件
if(NOT UNIX)
    ## 从测试列表中移除相关测试文件
    list(FILTER OL_CORE_TEST_SRC EXCLUDE REGEX 
    "test_(fifo_|ol_ipc|ol_tcp_|ol_signal|pipe|shm).*\\.cpp$"
    # "test_(fifo_|ol_ipc|ol_tcp_|ol_signal|pipe|shm).*\\.cpp$|test_ol_sort\\.cpp$|test_ol_sort2\\.cpp$"
    )
endif()

# 输出路径
if(DEFINED CMAKE_CONFIGURATION_TYPES)
    # 多配置：
    set(EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/bin/${OS_NAME}_${ARCHITECTURE}")
else()
    # 单配置：CMAKE_BUILD_TYPE
    set(EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/bin/${OS_NAME}_${ARCHITECTURE}-${CMAKE_BUILD_TYPE}")
endif()

# 生成测试程序
foreach(TEST_FILE ${OL_CORE_TEST_SRC})
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
    add_executable(${TEST_NAME} ${TEST_FILE})
    target_link_libraries(${TEST_NAME} PRIVATE ol_core)
endforeach()