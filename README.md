# OL

一个包含 `ollib`、`oldblib`（含 MySQL 和 Oracle 数据库交互）及 FTP 功能的 C++ 工具库，提供基础功能、多数据库交互及 FTP 客户端支持。


## 📜 许可证信息
![License: CC BY 4.0](https://img.shields.io/badge/License-CC%20BY%204.0-blue.svg)  
本项目所有代码（含基础功能、数据库交互模块、FTP 客户端模块及内置依赖）均以 **Creative Commons Attribution 4.0 International (CC BY 4.0)** 许可证发布，完整法律文本见 [CC BY 4.0 官方协议](https://creativecommons.org/licenses/by/4.0/legalcode)。


### CC BY 4.0 核心条款（通俗解读）
你可以**自由地**：
- **共享**：以任何媒介/格式复制、分发本项目代码或文档（包括商业用途）；
- **改编**：对代码进行修改、扩展、集成到其他项目（包括商业项目），无需额外申请授权。

需遵守的**唯一核心义务**：
- **署名（Attribution）**：使用或分发本项目代码/衍生作品时，需注明本项目来源（格式建议：“基于 ol木子李lo 开发的 OL 库”），并提供 [CC BY 4.0 许可证链接](https://creativecommons.org/licenses/by/4.0/)。


## 🎯 核心功能
- **`ollib`**：基础工具库，覆盖网络通信、文件IO、时间处理、字符串操作等高频基础功能。
- **`oldblib`**：多数据库交互模块，统一接口风格，支持两种主流数据库：
  - `oldblib/mysql`：MySQL 数据库交互模块，基于 MySQL C API 封装，支持连接管理、SQL 执行、参数绑定、结果集处理及 BLOB/TEXT 大字段操作。
  - `oldblib/oracle`：Oracle 数据库交互模块，基于 OCI 接口封装，支持连接管理、SQL 执行、结果解析等核心数据库操作。
- **`ol_ftp`**：FTP 客户端模块，基于第三方库 [ftplib](https://github.com/codebrainz/ftplib)（已内置，路径：`third_party/ftplib`），支持文件上传/下载、目录创建/删除、文件列表获取等标准操作。


## 📚 代码文档规范
本项目所有头文件（`.h`）均遵循 **Doxygen 注释规范**，便于开发者快速理解接口功能和使用方式：
- 函数/类/结构体均包含完整注释，覆盖功能描述（`@brief`）、参数说明（`@param`）、返回值（`@return`）及可选注意事项（`@note`）；
- 支持通过 Doxygen 工具自动生成 HTML/PDF 格式 API 文档，降低二次开发学习成本；
- 主流 IDE（VS Code、CLion 等）可识别注释并提供智能提示（鼠标悬停显示详情），提升编码效率。

### 示例注释风格
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


## ⚙️ 环境依赖
### 基础依赖
- 构建工具：CMake 3.10+
- 编译器：C++17 兼容编译器（GCC 8+/Clang 7+/MSVC 2019+/MinGW 8+）

### 模块特定依赖
| 模块                | 依赖项说明                                                                 |
|---------------------|--------------------------------------------------------------------------|
| `oldblib/mysql`     | MySQL 客户端（版本 5.7 及以上）；需包含 MySQL C API 头文件（`mysql.h`），并链接 `mysqlclient` 库（Windows 为 `libmysql.lib`，Linux 为 `libmysqlclient.so`） |
| `oldblib/oracle`    | Oracle 客户端（版本 11g 及以上）；需配置 `ORACLE_HOME` 环境变量，确保 OCI 库（`oci.dll`/`libclntsh.so`）可链接 |
| `ol_ftp`            | 内置依赖：已集成 [ftplib](https://github.com/codebrainz/ftplib)（路径：`third_party/ftplib`），无需额外安装 |


## 📝 编码与格式规范
为保证跨平台兼容性，本项目严格遵循以下规范：
1. **字符集**：所有源代码文件（`.h`/`.cpp`/`.cmake` 等）均采用 **UTF-8（无 BOM）** 编码，避免多语言字符（含中文）在不同系统下乱码；
2. **换行符**：统一使用 **LF（\n）** 作为换行符，防止跨平台协作时因 CRLF（Windows）/LF（Linux/macOS）混用导致的版本控制冲突。

> 💡 提示：建议在编辑器中固化配置（如 VS Code 可通过 `.vscode/settings.json` 设置），提交代码前确保符合规范，示例配置如下：
> ```json
> {
>   "files.encoding": "utf8",
>   "files.eol": "\n",
>   "files.trimTrailingWhitespace": true
> }
> ```


## 🔨 编译步骤
### Linux 平台
```bash
# 1. 克隆仓库
git clone https://github.com/1613661434/OL.git
cd OL

# 2. 创建并进入构建目录（推荐 out-of-source 构建）
mkdir build && cd build

# 3. 生成 Makefile（自动识别 MySQL/Oracle 依赖，内置 ftplib 无需额外配置）
cmake ..

# 4. 并行编译（-j 后接核心数，如 -j4 表示4线程）
make -j4

# 5. 输出路径
# - 静态库：lib/linux/static/
# - 动态库：lib/linux/shared/
# - 测试程序：各自 test/bin/ 目录下
```

### Windows 平台（MSVC）
```powershell
# 1. 克隆仓库
git clone https://github.com/1613661434/OL.git
cd OL

# 2. 创建并进入构建目录
mkdir build && cd build

# 3. 生成 Visual Studio 项目（以 VS2022 x64 为例）
cmake .. -G "Visual Studio 17 2022" -A x64

# 4. 编译（或直接在 VS 中打开 OL.sln 编译）
msbuild OL.sln /p:Configuration=Release /p:Platform=x64 /m

# 5. 输出路径
# - 静态库：lib/win32/static/
# - 动态库：lib/win32/shared/
# - 测试程序：各自 test/bin/ 目录下
```

### Windows 平台（MinGW）
#### 前置条件
- 安装 MinGW-w64（支持 64 位），确保 `g++`、`mingw32-make` 工具可用；
- 将 MinGW 的 `bin` 目录（如 `D:\MinGW\bin`）添加到系统 `PATH`，通过 `g++ --version` 验证安装。

#### 编译命令
```powershell
# 1. 克隆仓库
git clone https://github.com/1613661434/OL.git
cd OL

# 2. 创建并进入构建目录
mkdir build && cd build

# 3. 生成 MinGW Makefile
cmake .. -G "MinGW Makefiles"

# 4. 并行编译
mingw32-make -j4

# 5. 输出路径与扩展名说明
# - 静态库：lib/win32/static/（.a 格式，如 libol.a）
# - 动态库：lib/win32/shared/（.dll 格式，如 ol.dll）
# - 测试程序：各自 test/bin/win32/ 目录下（过滤 Linux 特有测试用例）
```

#### 终端乱码解决
若 Windows PowerShell 出现中文乱码，可临时设置 UTF-8 字符集：
```powershell
# 设置代码页为 UTF-8 并统一输入输出编码
chcp 65001 | Out-Null
$OutputEncoding = [Console]::InputEncoding = [Console]::OutputEncoding = New-Object System.Text.UTF8Encoding

# 验证编码（需显示 BodyName: utf-8, CodePage: 65001）
$OutputEncoding
```


## ⚠️ 注意事项
1. **内置依赖保护**：`third_party/ftplib` 为项目核心依赖，请勿随意修改目录结构；如需更新 ftplib 版本，建议直接替换该目录文件并保持路径一致；
2. **数据库平台适配**：跨平台编译时，需确保数据库客户端版本与目标平台匹配（如 Windows x64 对应 MySQL/Oracle 客户端 x64 版本，Linux x86_64 对应同架构版本）；
3. **许可证合规**：使用或分发本项目/衍生作品时，需按 CC BY 4.0 要求注明“基于 ol木子李lo 开发的 OL 库”及 [CC BY 4.0 许可证链接](https://creativecommons.org/licenses/by/4.0/)。


## 📋 目录结构说明
```
OL
├───oldblib               # 数据库交互模块（多数据库统一接口）
│   ├───mysql             # MySQL 交互子模块
│   │   ├───include       # 头文件（如 ol_mysql.h）
│   │   ├───lib           # 编译输出库（分平台）
│   │   ├───src           # 实现代码
│   │   └───test          # 测试程序及数据
│   └───oracle            # Oracle 交互子模块
│       ├───include       # 头文件（如 ol_oci.h）
│       ├───lib           # 编译输出库（分平台）
│       ├───src           # 实现代码
│       └───test          # 测试程序及数据
└───ollib                 # 基础工具库
    ├───include           # 基础功能头文件
    ├───lib               # 编译输出库（分平台）
    ├───src               # 基础功能实现
    ├───test              # 基础功能测试
    └───third_party       # 第三方依赖
        └───ftplib        # FTP 客户端依赖库
```


## 📋 详细功能介绍
...请等待作者更新