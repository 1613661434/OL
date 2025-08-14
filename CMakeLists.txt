cmake_minimum_required(VERSION 3.10)
project(ol)

# 指定C++标准（全局生效）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 跨平台字符集配置：统一使用UTF-8
if(MSVC)
    # Windows (MSVC)：强制UTF-8编码，禁用BOM
    add_compile_options(/utf-8)
    # 禁用安全函数警告、使用Unicode字符集
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS UNICODE _UNICODE)
else()
    # Linux/macOS (GCC/Clang)：默认支持UTF-8，添加通用警告选项
    add_compile_options(-Wno-write-strings -D_GLIBCXX_USE_CXX11_ABI=1)
    # 添加额外的跨平台编译优化
    add_compile_options(-finput-charset=UTF-8)  # 显式指定输入编码为UTF-8
endif()

# 添加子目录
add_subdirectory(ollib)
# add_subdirectory(oldblib/oracle)