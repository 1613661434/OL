cmake_minimum_required(VERSION 3.10)
project(ol)

# 指定C++标准（全局生效）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 区分平台设置通用选项
if(WIN32)
    # Windows：禁用安全函数警告、多字节字符集
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS UNICODE _UNICODE)
else()
    # Linux：保持原选项
    add_compile_options(-Wno-write-strings -D_GLIBCXX_USE_CXX11_ABI=1)
endif()

# 添加子目录
add_subdirectory(ollib)
add_subdirectory(oldblib/oracle)