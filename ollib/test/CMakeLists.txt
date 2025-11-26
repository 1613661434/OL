# OL/ollib/test/CMakeLists.txt
project(ollib_tests)

# 链接主库
link_libraries(ol)

# 测试源文件 - 先收集所有测试文件
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/. OL_TEST_SRC)

# 过滤非Linux平台的测试文件
if(NOT UNIX)
    ## 从测试列表中移除相关测试文件
    list(FILTER OL_TEST_SRC EXCLUDE REGEX 
    "test_(fifo_|ol_ftp_|ol_ipc|ol_tcp_|ol_signal|ol_InetAddr|pipe|shm).*\\.cpp$"
    # "test_(fifo_|ol_ftp_|ol_ipc|ol_tcp_|ol_signal|ol_InetAddr|pipe|shm).*\\.cpp$|test_ol_sort\\.cpp$|test_ol_sort2\\.cpp$"
    )
endif()

# 输出路径（区分平台）
if(DEFINED CMAKE_CONFIGURATION_TYPES)
    # 多配置：
    set(EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/bin/${OS_NAME}/${ARCHITECTURE}")
else()
    # 单配置：CMAKE_BUILD_TYPE
    set(EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/bin/${OS_NAME}/${ARCHITECTURE}/${CMAKE_BUILD_TYPE}")
endif()

# 生成测试程序（过滤后）
foreach(TEST_FILE ${OL_TEST_SRC})
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
    add_executable(${TEST_NAME} ${TEST_FILE})
endforeach()

# ===================== ol_net测试配置（仅Linux） =====================
if(UNIX AND NOT APPLE)
    ## 1. ol_net测试的路径定义
    set(OL_NET_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ol_net)  # test/ol_net目录
    set(OL_NET_INCLUDE ${OL_NET_TEST_DIR}/include)          # test/ol_net/include（测试专用头文件）
    set(OL_NET_SRC_DIR ${OL_NET_TEST_DIR}/src)              # test/ol_net/src（测试专用源文件）
    set(OL_NET_OUTPUT_DIR ${EXECUTABLE_OUTPUT_PATH}/ol_net) # 输出目录：bin/linux/ol_net

    ## 2. 添加ol_net测试的头文件路径（测试代码需引用ol_BankServer.h等）
    include_directories(${OL_NET_INCLUDE})

    ## 3. 收集ol_net测试的源文件：
    ###    - 测试入口文件（test_ol_echoserver.cpp等）
    ###    - 测试专用实现文件（ol_EchoServer.cpp等）
    aux_source_directory(${OL_NET_TEST_DIR} OL_NET_TEST_FILES)  # 测试入口cpp
    aux_source_directory(${OL_NET_SRC_DIR} OL_NET_IMPL_FILES)   # 测试实现cpp
    set(OL_NET_ALL_SRC ${OL_NET_TEST_FILES} ${OL_NET_IMPL_FILES})

    ## 4. 生成ol_net测试程序
    foreach(TEST_FILE ${OL_NET_TEST_FILES})
        get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)  # 提取文件名（不含扩展名）
        add_executable(${TEST_NAME} ${TEST_FILE} ${OL_NET_IMPL_FILES})  # 链接测试实现文件

        ### 设置输出路径到bin/linux/ol_net
        set_target_properties(${TEST_NAME} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${OL_NET_OUTPUT_DIR}
        )
    endforeach()
endif()