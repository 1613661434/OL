# OL/ol_network/test/CMakeLists.txt
project(ol_network_tests)

# 测试头文件目录 (test/include)
set(TEST_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include)
# 测试实现源码目录 (test/src)
set(TEST_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# 收集测试实现文件
aux_source_directory(${TEST_SRC_DIR} TEST_IMPL_FILES)
# 收集测试入口文件
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} TEST_ENTRY_FILES)

# ===================== 输出路径 =====================
if(DEFINED CMAKE_CONFIGURATION_TYPES)
    # 多配置：
    set(EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/bin/${OS_NAME}_${ARCHITECTURE}")
else()
    # 单配置：CMAKE_BUILD_TYPE
    set(EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/bin/${OS_NAME}_${ARCHITECTURE}-${CMAKE_BUILD_TYPE}")
endif()

# ===================== 生成测试程序 =====================
foreach(TEST_FILE ${TEST_ENTRY_FILES})
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
    # 测试程序 = 入口文件 + 测试专用实现文件
    add_executable(${TEST_NAME} ${TEST_FILE} ${TEST_IMPL_FILES})
    
    # 链接库
    target_link_libraries(${TEST_NAME} PRIVATE ol_network)
    # 添加测试专用头文件
    target_include_directories(${TEST_NAME} PRIVATE ${TEST_INCLUDE})
endforeach()