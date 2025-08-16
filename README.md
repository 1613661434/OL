# OL

一个包含 `ollib`、`oldblib/oracle` 及 FTP 功能的 C++ 工具库，提供基础功能、Oracle 数据库交互及 FTP 客户端支持。

## 🍀 项目衍生说明  
OL 库 **fork 自 freecplus 库**，感谢原作者贡献！在其基础上扩展功能并实现跨平台适配，核心协议遵循：  
- [freecplus 开源许可协议](https://blog.csdn.net/wucz122140729/article/details/105167157)（需保留版权声明与许可协议）  
- 原文遵循 [CC 4.0 BY-SA 协议](https://creativecommons.org/licenses/by-sa/4.0/)，转载/修改需保留原文链接与声明  

## 功能

- `ollib`：基础工具库（如网络、文件操作、时间处理等）
- `oldblib/oracle`：Oracle 数据库交互模块（基于 OCI，提供连接管理、SQL 执行等功能）
- `ol_ftp`：FTP 客户端模块（基于第三方库`(已内置)`的 [ftplib](https://github.com/codebrainz/ftplib)，支持文件上传、下载、目录操作等）

## 代码文档规范

本项目所有头文件（`.h`）均遵循 **Doxygen 注释规范**，便于开发者快速理解接口功能和使用方式，主要特点包括：

- 函数/类/结构体均包含完整注释，包括功能描述（`@brief`）、参数说明（`@param`）、返回值（`@return`）及注意事项（`@note` 可选）
- 支持通过 Doxygen 工具自动生成 HTML/PDF 格式的 API 文档
- 主流 IDE（VS Code、CLion 等）可识别注释并提供智能提示（鼠标悬停显示详细说明）

示例注释风格：
```cpp
/**
 * @brief 删除字符串左右两边指定字符
 *        支持 C 字符串和 std::string 两种类型
 * @param str 待处理的字符串（C字符串直接修改，std::string为引用）
 * @param c 要删除的字符（默认空格' '）
 * @return 修改后的字符串（C字符串返回指针，std::string返回引用）
 */
char* deleteLRchr(char* str, const char c = ' ');
std::string& deleteLRchr(std::string& str, const char c = ' ');
```

## 环境依赖

### 基础依赖
- CMake 3.10+
- C++17 兼容编译器（GCC 8+/Clang 7+/MSVC 2019+/MinGW 8+）

### 模块特定依赖
1. **`oldblib/oracle` 依赖**  
   - Oracle 客户端（版本 11g 及以上）  
   - 环境变量配置：需设置 `ORACLE_HOME` 指向 Oracle 客户端安装目录，确保 OCI 库（`oci.dll`/`libclntsh.so`）可被链接。

2. **`ol_ftp` 依赖**  
   - 内置 ftplib：已集成到项目中（路径：`third_party/ftplib`），无需额外安装，编译时会自动引用。

## 编码与格式规范

为保证跨平台兼容性，本项目严格遵循以下规范：

1. **字符集**：所有源代码文件（`.h`/`.cpp`/`.cmake` 等）均采用 **UTF-8（无 BOM）** 编码，确保多语言字符（包括中文）在 Windows、Linux、macOS 下无乱码。

2. **换行符**：统一使用 **LF（\n）** 作为换行符，避免跨平台协作时因 CRLF（Windows）和 LF（Linux/macOS）混用导致的版本控制冲突。

> 提示：建议在编辑器中开启自动检测配置（如 VS Code 可通过 `.vscode/settings.json` 固化编码和换行符设置），提交代码前确保符合上述规范。

## 编译步骤

### Linux

```bash
# 克隆仓库
git clone https://github.com/1613661434/OL.git
cd OL

# 创建构建目录并进入
mkdir build
cd build

# 生成 Makefile（Oracle 路径通过 ORACLE_HOME 自动识别，ftplib 已内置）
cmake ..

# 编译（-j 指定并行任务数）
make -j4

# 输出路径
# - 静态库：lib/linux/static/
# - 动态库：lib/linux/shared/
# - 测试程序：各自 test/bin/ 目录下
```

### Windows（MSVC）

```powershell
# 克隆仓库
git clone https://github.com/1613661434/OL.git
cd OL

# 创建构建目录并进入
mkdir build
cd build

# 生成 Visual Studio 项目（以 VS2022 为例，Oracle 依赖通过 ORACLE_HOME 识别）
cmake .. -G "Visual Studio 17 2022" -A x64

# 编译（使用 MSBuild 或在 VS 中打开项目）
msbuild OL.sln /p:Configuration=Release /p:Platform=x64 /m

# 输出路径
# - 静态库：lib/win32/static/
# - 动态库：lib/win32/shared/
# - 测试程序：各自 test/bin/ 目录下
```

### Windows（MinGW）

#### 前置条件
- 安装 MinGW 编译器（推荐 MinGW-w64，支持 64 位系统，需包含 `g++`、`mingw32-make` 工具）
- 确保 MinGW 的 `bin` 目录（如 `D:\MinGW\bin`）已添加到系统环境变量 `PATH`，可通过 `g++ --version` 验证是否安装成功

#### 编译步骤
```powershell
# 克隆仓库
git clone https://github.com/1613661434/OL.git
cd OL

# 创建构建目录并进入
mkdir build
cd build

# 生成 MinGW Makefile（指定生成器为 MinGW Makefiles）
cmake .. -G "MinGW Makefiles"

# 编译（-j 指定并行任务数，加速编译）
mingw32-make -j4

# 输出路径
# - 静态库：lib/win32/static/（扩展名为 .a，如 libol.a）
# - 动态库：lib/win32/shared/（扩展名为 .dll，如 ol.dll）
# - 测试程序：各自 test/bin/win32/ 目录下（仅包含 Windows 兼容的测试程序）
```

#### 注意事项
- MinGW 编译的静态库默认以 `.a` 为扩展名（与 Linux 一致），动态库以 `.dll` 为扩展名（Windows 原生格式）
- 部分 Linux 特有测试程序（如基于命名管道的 `test_fifo_*`）会被自动过滤，不参与 Windows 平台编译
- 如果 Windows 终端（PowerShell）出现乱码，可以临时设置UTF8字符集
```powershell
# 1. 设置代码页为 UTF-8（65001，Windows 终端通用标识）
chcp 65001 | Out-Null

# 2. 强制将输入/输出/管道编码全部设为 UTF-8
$OutputEncoding = [Console]::InputEncoding = [Console]::OutputEncoding = New-Object System.Text.UTF8Encoding

# 3. 验证结果（此时应显示 BodyName 为 utf-8，CodePage 为 65001）
$OutputEncoding
[Console]::InputEncoding
[Console]::OutputEncoding
```

## 注意事项
- `third_party/ftplib` 为项目内置依赖，请勿随意修改目录结构，如需更新 ftplib 版本，建议直接替换该目录下的文件并保持路径一致。
- 跨平台编译时，确保 Oracle 客户端的平台版本与目标平台一致（如 Windows 用 x64 库，Linux 用 x86_64 库）。

## 详细功能介绍
...请等待作者更新