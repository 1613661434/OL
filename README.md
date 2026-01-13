# OL

一个包含 `ollib`（含**主从 Reactor 多线程网络库**）、`oldblib`（MySQL/Oracle 数据库交互）及 FTP 功能的 C++ 工具库，提供基础功能、多数据库交互、FTP 客户端支持及 Linux 专属的高性能网络通信能力。

## 📜 许可证信息

![License: CC BY 4.0](https://img.shields.io/badge/License-CC%20BY%204.0-blue.svg)

本项目所有代码（含基础功能、网络库、数据库交互模块、FTP 客户端模块及内置依赖）均以 **Creative Commons Attribution 4.0 International (CC BY 4.0)** 许可证发布，完整法律文本见 [CC BY 4.0 官方协议](https://creativecommons.org/licenses/by/4.0/legalcode)。

### CC BY 4.0 核心条款（通俗解读）

你可以**自由地**：

* **共享**：以任何媒介 / 格式复制、分发本项目代码或文档（包括商业用途）；

* **改编**：对代码进行修改、扩展、集成到其他项目（包括商业项目），无需额外申请授权。

需遵守的**唯一核心义务**：

* **署名（Attribution）**：使用或分发本项目代码 / 衍生作品时，需注明本项目来源（格式建议：“基于 ol 木子李 lo 开发的 OL 库”），并提供 [CC BY 4.0 许可证链接](https://creativecommons.org/licenses/by/4.0/)。

## 🎯 核心功能

* `ollib`：基础工具库，覆盖高频基础功能及 Linux 专属网络库：


  * 基础能力：文件 IO、时间处理、字符串操作、线程池、数据结构（哈希、前缀树等）；

  * **主从 Reactor 多线程网络库（仅限 Linux）**：路径`ollib/include/ol_net`/`ollib/src/ol_net`，基于 epoll 实现，支持非阻塞 IO、边缘触发（ET），核心架构为 “1 个主 Reactor 监听连接 + N 个从 Reactor 处理 IO 事件”，支持连接管理、报文缓冲区、事件驱动回调，适用于高并发网络场景（如服务器开发）。

* `oldblib`：多数据库交互模块，统一接口风格，支持两种主流数据库：


  * `oldblib/mysql`：MySQL 交互，基于 MySQL C API 封装，支持连接管理、SQL 执行、BLOB/TEXT 大字段操作；

  * `oldblib/oracle`：Oracle 交互，基于 OCI 接口封装，支持连接管理、SQL 执行、BLOB/CLOB 大字段操作。

* `ol_ftp`：FTP 客户端模块，基于内置[ft](https://github.com/codebrainz/ftplib)[plib](https://github.com/codebrainz/ftplib)（路径`third_party/ftplib`），支持文件上传 / 下载、目录操作、文件列表获取。

## 📚 代码文档规范

本项目所有头文件（`.h`）均遵循 **Doxygen 注释规范**，便于开发者快速理解接口功能：

* 函数 / 类 / 结构体包含完整注释，覆盖功能描述（`@brief`）、参数说明（`@param`）、返回值（`@return`）及注意事项（`@note`）；

* 支持通过 Doxygen 自动生成 HTML/PDF 格式 API 文档；

* 主流 IDE（VS Code、CLion）可识别注释并提供智能提示，提升编码效率。

### 示例注释风格

```cpp
/**
 * @brief 根据绝对路径逐级创建目录
 * @param pathorfilename 绝对路径的文件名或目录名
 * @param bisfilename 指定pathorfilename类型（true-文件名，false-目录名，默认true）
 * @return true-成功，false-失败（权限不足、路径非法、磁盘满等）
 */
bool newdir(const std::string& pathorfilename, bool bisfilename = true);
```

## ⚙️ 环境依赖

### 基础依赖

* 构建工具：CMake 3.10+

* 编译器：C++17 兼容编译器（Linux：GCC 8+/Clang 7+；Windows：MSVC 2019+/MinGW 8+）

* 操作系统：


  * 网络库：**仅限 Linux（内核 2.6+，需支持 epoll）**；

  * 其他模块：Windows 10+/Linux CentOS 7+/Ubuntu 18.04+。

### 模块特定依赖

| 模块               | 依赖项说明                                                            |
| ---------------- | ---------------------------------------------------------------- |
| `ollib/ol_net`   | Linux epoll（内核 2.6+）；无需额外安装，内核自带                                 |
| `oldblib/mysql`  | MySQL 客户端（5.7+），需链接`mysqlclient`库（Linux）/`libmysql.lib`（Windows） |
| `oldblib/oracle` | Oracle 客户端（11g+），需配置`ORACLE_HOME`环境变量                            |
| `ol_ftp`         | 内置 ftplib，无需额外安装                                                 |

## 🔧 CMake 缓存变量说明

所有变量可通过 `cmake -D<变量名>=<值>` 命令行配置（覆盖默认值），用于灵活控制编译行为。变量分类如下：

### 1. 通用配置变量

| 变量名                | 默认值     | 可选值                                     | 作用描述                                        |
| ------------------ | ------- | --------------------------------------- | ------------------------------------------- |
| `CMAKE_BUILD_TYPE` | Release | Debug/Release/RelWithDebInfo/MinSizeRel | 构建类型（单配置生成器如 MinGW/Linux 必需，多配置如 MSVC 无需指定） |
| `ENABLE_WARNINGS`  | OFF      | ON/OFF                                  | 是否启用编译器警告                    |

### 2. `ollib` 库配置变量

| 变量名                    | 默认值 | 可选值    | 作用描述                                  |
| ---------------------- | --- | ------ | ------------------------------------- |
| `OL_WITH_TESTS`        | OFF | ON/OFF | 是否编译 `ollib` 的测试程序（如线程池、网络库测试）        |
| `OL_BUILD_STATIC_LIBS` | ON  | ON/OFF | 是否生成 `ollib` 静态库（`libol.a`/`ol.lib`）  |
| `OL_BUILD_SHARED_LIBS` | ON  | ON/OFF | 是否生成 `ollib` 动态库（`libol.so`/`ol.dll`） |

### 3. `oldblib` 数据库模块配置变量

#### MySQL 模块（需依赖环境变量`MYSQL_HOME`）

| 变量名                            | 默认值 | 可选值    | 作用描述                                                  |
| ------------------------------ | --- | ------ | ----------------------------------------------------- |
| `OLDB_MYSQL_WITH_TESTS`        | OFF | ON/OFF | 是否编译 MySQL 模块的测试程序                                    |
| `OLDB_MYSQL_BUILD_STATIC_LIBS` | OFF | ON/OFF | 是否生成 MySQL 模块静态库（`liboldb_mysql.a`/`oldb_mysql.lib`）  |
| `OLDB_MYSQL_BUILD_SHARED_LIBS` | OFF | ON/OFF | 是否生成 MySQL 模块动态库（`liboldb_mysql.so`/`oldb_mysql.dll`） |

#### Oracle 模块（需依赖环境变量`ORACLE_HOME`）

| 变量名                             | 默认值 | 可选值    | 作用描述                                                     |
| ------------------------------- | --- | ------ | -------------------------------------------------------- |
| `OLDB_ORACLE_WITH_TESTS`        | OFF | ON/OFF | 是否编译 Oracle 模块的测试程序                                      |
| `OLDB_ORACLE_BUILD_STATIC_LIBS` | OFF | ON/OFF | 是否生成 Oracle 模块静态库（`liboldb_oracle.a`/`oldb_oracle.lib`）  |
| `OLDB_ORACLE_BUILD_SHARED_LIBS` | OFF | ON/OFF | 是否生成 Oracle 模块动态库（`liboldb_oracle.so`/`oldb_oracle.dll`） |

### 变量使用示例

```bash
# 示例1：Linux 下编译 Debug 版本 + 启用 ollib 测试 + 仅生成静态库
cmake .. -DCMAKE_BUILD_TYPE=Debug -DOL_WITH_TESTS=ON -DOL_BUILD_SHARED_LIBS=OFF

# 示例2：Windows MinGW 下编译 Release 版本 + 启用 MySQL 模块 + 生成动态库
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DOLDB_MYSQL_BUILD_SHARED_LIBS=ON

# 示例3：禁用所有警告 + 同时生成 ollib 和 MySQL 静态库
cmake .. -DENABLE_WARNINGS=OFF -DOL_BUILD_STATIC_LIBS=ON -DOLDB_MYSQL_BUILD_STATIC_LIBS=ON
```

## 📝 编码与格式规范

1. **字符集**：所有文件（`.h`/`.cpp`/`.cmake`）均采用 **UTF-8（无 BOM）**，避免多语言字符乱码；

2. **换行符**：统一使用 **LF（\n）**，防止跨平台协作时版本控制冲突。

## 🔨 编译步骤

### Linux 平台（含网络库编译）

```bash
# 1. 克隆仓库
git clone https://github.com/1613661434/OL.git
cd OL

# 2. 创建构建目录（推荐out-of-source构建）
mkdir build && cd build

# 3. 生成Makefile（可添加缓存变量自定义配置）
cmake ..  # 示例：启用测试 + 仅静态库 → cmake .. -DOL_WITH_TESTS=ON -DOL_BUILD_SHARED_LIBS=OFF

# 4. 并行编译（-j后接核心数，如-j4）
make -j4

# 5. 输出路径
# - 静态库：ollib/lib/linux/x64/static/${CMAKE_BUILD_TYPE}/（含网络库代码）
# - 动态库：ollib/lib/linux/x64/shared/${CMAKE_BUILD_TYPE}/（含网络库代码）
# - 网络库测试程序：ollib/test/bin/linux/x64/${CMAKE_BUILD_TYPE}/ol_net/（test_ol_echoserver等）
# - 其他测试程序：各自test/bin/linux/x64/${CMAKE_BUILD_TYPE}/目录下
```

### Windows 平台（不含网络库）

#### MSVC 编译

```bash
# 1. 克隆仓库
git clone https://github.com/1613661434/OL.git
cd OL

# 2. 创建构建目录
mkdir build && cd build

# 3. 生成.sln文件（可添加缓存变量）
cmake .. -G"Visual Studio 17 2022"  # 示例：Debug模式 → -DCMAKE_BUILD_TYPE=Debug

# 4. VS2022打开进行编译

# 5. 输出：仅基础模块与数据库模块，无网络库
```

#### MinGW 编译

```bash
# 1. 克隆仓库
git clone https://github.com/1613661434/OL.git
cd OL

# 2. 创建构建目录
mkdir build && cd build

# 3. 生成MinGW Makefile（可添加缓存变量）
cmake .. -G "MinGW Makefiles"  # 示例：Debug模式 → -DCMAKE_BUILD_TYPE=Debug

# 4. 并行编译
mingw32-make -j4

# 5. 输出：仅基础模块与数据库模块，无网络库
```

## ⚠️ 注意事项

1. **网络库平台限制**：`ollib/ol_net` 仅限 Linux，Windows 下不会编译相关代码，避免依赖冲突；

2. **内置依赖保护**：`third_party/ftplib` 为核心依赖，请勿修改目录结构；如需更新，直接替换该目录文件并保持路径一致；

3. **数据库版本适配**：跨平台编译时，需确保数据库客户端版本与目标平台架构匹配（如 x64 客户端对应 x64 编译目标）；

4. **许可证合规**：使用 / 分发本项目时，需按 CC BY 4.0 要求注明来源及许可证链接。

### 终端乱码解决

若 Windows PowerShell 出现中文乱码，可临时设置 UTF-8 字符集：

```powershell
# 设置输入输出编码为 UTF-8
$OutputEncoding = [Console]::InputEncoding = [Console]::OutputEncoding = [System.Text.UTF8Encoding]::UTF8

# 验证编码（需显示 BodyName: utf-8, CodePage: 65001）
[Console]::OutputEncoding
[Console]::InputEncoding
```

## 📋 目录结构说明

```bash
OL
├───oldblib               # 数据库交互模块
│   ├───mysql             # MySQL子模块（include/lib/src/test）
│   └───oracle            # Oracle子模块（include/lib/src/test）
├───ollib                 # 基础工具库（含网络库）
│   ├───include
│   │   ├───ol_base           # 基础组件头文件（ol_sort_base.h等）
│   │   ├───ol_net            # 网络库头文件（Acceptor.h/Connection.h等）
│   │   ├───third_party       # 第三方库头文件（ftplib.h等）
│   │   └───其他基础模块头文件
│   ├───src
│   │   ├───ol_base           # 基础组件实现
│   │   ├───ol_net            # 网络库实现（Acceptor.cpp/Connection.cpp等）
│   │   └───其他基础模块实现
│   └───test
│       ├───ol_net            # 网络库测试（回声服务器/银行示例等）
│       └───其他模块测试
└───third_party
│   └───ftplib            # 内置FTP依赖库
└───docs                  # 说明文档（index.html）
```

## 📋 网络库详细介绍（仅限 Linux）

### 1. 架构设计

采用**主从 Reactor 多线程模型**，核心组件分工：

* **主 Reactor**：1 个线程，负责监听`listenfd`，接收新连接后分发到从 Reactor；

* **从 Reactor**：N 个线程（默认与 CPU 核心数一致），每个线程绑定 1 个`epoll`实例，处理已连接 fd 的 IO 事件（读 / 写）；

* **事件驱动**：基于`epoll ET`（边缘触发）+ 非阻塞 IO，减少不必要的事件触发，提升高并发性能。

### 2. 核心组件

| 组件           | 功能描述                                                      |
| ------------ | --------------------------------------------------------- |
| `EventLoop`  | 事件循环核心，管理`epoll`实例与注册的事件回调（读 / 写 / 关闭）                    |
| `Acceptor`   | 主 Reactor 专属，监听新连接，生成`SocketFd`后传递给从 Reactor 的`EventLoop` |
| `Connection` | 管理单个 TCP 连接，封装`fd`、输入 / 输出缓冲区（`Buffer`）及 IO 回调            |
| `Buffer`     | 网络缓冲区，支持报文分隔符（四字节长度 `\r\n\r\n`）、从 fd 直接读数据                |
| `TcpServer`  | 网络服务端入口，封装主从 Reactor 初始化、连接管理、回调注册（新连接 / 消息 / 关闭）         |

### 3. 典型使用场景

* 高并发 TCP 服务器（如回声服务器、网关、游戏服务器）；

* 需低延迟 IO 处理的场景（依赖 epoll ET 模式减少事件开销）；

* 需统一连接管理与报文解析的网络应用（依赖`Buffer`的分隔符功能）。