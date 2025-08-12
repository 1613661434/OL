# OL

一个包含 `ollib` 和 `oldblib/oracle` 的C++工具库，提供基础功能和Oracle数据库交互支持。

## 功能

- `ollib`：基础工具库（如网络、队列等）
- `oldblib/oracle`：Oracle数据库交互模块（基于OCI）

## 环境依赖

- CMake 3.10+
- C++17 兼容编译器（GCC 8+/Clang 7+/MSVC 2019+）
- Oracle客户端（仅 `oldblib/oracle` 需要，设置 `ORACLE_HOME` 环境变量）

## 编译步骤

### Linux

```bash
# 克隆仓库
git clone https://github.com/你的用户名/OL.git
cd OL

# 创建构建目录
mkdir build && cd build

# 生成Makefile
cmake ..

# 编译（-j指定并行任务数）
make -j4

# 输出路径
# - 静态库：lib/linux/static/
# - 动态库：lib/linux/shared/
# - 测试程序：各自test/bin/目录下
