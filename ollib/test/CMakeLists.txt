cmake_minimum_required(VERSION 3.10)
project(ollib_tests)

# C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 调试模式（默认启用，可注释切换为Release）
set(CMAKE_BUILD_TYPE Debug)

# 平台编译选项
if(MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od /Zi /W3")
else()
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g -D_GLIBCXX_USE_CXX11_ABI=1")
endif()

# 头文件和库路径（复用主库变量）
include_directories(${OLLIB_INCLUDE})
link_directories(${OLLIB_STATIC_LIB})

# 链接主库
link_libraries(ol)

# 测试源文件 - 先收集所有测试文件
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/. ALL_TEST_SRC)

# 过滤非Linux平台的测试文件
if(NOT LINUX)
    # 从测试列表中移除相关测试文件
    list(FILTER ALL_TEST_SRC EXCLUDE REGEX 
    "test_(fifo_|ol_ftp_|ol_ipc|ol_tcp_|ol_signal|ol_InetAddr|pipe|shm).*\\.cpp$"
    # "test_(fifo_|ol_ftp_|ol_ipc|ol_tcp_|ol_signal|ol_InetAddr|pipe|shm).*\\.cpp$|test_ol_sort\\.cpp$|test_ol_sort2\\.cpp$"
    )
endif()

# 输出路径（区分平台）
if(WIN32)
    set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin/win32)
else()
    set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin/linux)
endif()
file(MAKE_DIRECTORY ${EXECUTABLE_OUTPUT_PATH})

# 生成测试程序（只使用过滤后的测试文件列表）
foreach(TEST_FILE ${ALL_TEST_SRC})
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
    add_executable(${TEST_NAME} ${TEST_FILE})
    
    # 对于Linux平台的FIFO测试程序，无需额外操作
    # 对于Windows平台，已通过上面的过滤排除了不兼容的测试文件
endforeach()

# ===================== ol_net测试配置（仅Linux） =====================
if(LINUX)
    # 1. ol_net测试的路径定义
    set(OL_NET_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ol_net)  # test/ol_net目录
    set(OL_NET_INCLUDE ${OL_NET_TEST_DIR}/include)          # test/ol_net/include（测试专用头文件）
    set(OL_NET_SRC_DIR ${OL_NET_TEST_DIR}/src)              # test/ol_net/src（测试专用源文件）
    set(OL_NET_OUTPUT_DIR ${EXECUTABLE_OUTPUT_PATH}/ol_net) # 输出目录：bin/linux/ol_net

    # 2. 创建输出目录
    file(MAKE_DIRECTORY ${OL_NET_OUTPUT_DIR})

    # 3. 添加ol_net测试的头文件路径（测试代码需引用ol_BankServer.h等）
    include_directories(${OL_NET_INCLUDE})

    # 4. 收集ol_net测试的源文件：
    #    - 测试入口文件（test_ol_echoserver.cpp等）
    #    - 测试专用实现文件（ol_EchoServer.cpp等）
    aux_source_directory(${OL_NET_TEST_DIR} OL_NET_TEST_FILES)  # 测试入口cpp
    aux_source_directory(${OL_NET_SRC_DIR} OL_NET_IMPL_FILES)   # 测试实现cpp
    set(OL_NET_ALL_SRC ${OL_NET_TEST_FILES} ${OL_NET_IMPL_FILES})

    # 5. 生成ol_net测试程序
    foreach(TEST_FILE ${OL_NET_TEST_FILES})
        get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)  # 提取文件名（不含扩展名）
        add_executable(${TEST_NAME} ${TEST_FILE} ${OL_NET_IMPL_FILES})  # 链接测试实现文件

        # 设置输出路径到bin/linux/ol_net
        set_target_properties(${TEST_NAME} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${OL_NET_OUTPUT_DIR}
        )

        # 链接主库（ol）
        target_link_libraries(${TEST_NAME} ol)
    endforeach()
endif()