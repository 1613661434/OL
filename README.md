# OL

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](https://opensource.org/licenses/MIT)
[![C++](https://img.shields.io/badge/C%2B%2B-17%2B-%2300599C?logo=c%2B%2B)](https://isocpp.org/)
[![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20Windows-%23808080?logo=linux)](https://github.com/1613661434/OL)
[![CMake](https://img.shields.io/badge/build-CMake%203.10%2B-%23064F8C?logo=cmake)](https://cmake.org/)
[![MySQL](https://img.shields.io/badge/database-MySQL%205.7%2B-%234479A1?logo=mysql)](https://www.mysql.com/)
[![Oracle](https://img.shields.io/badge/database-Oracle%2011g%2B-%23F80000?logo=oracle)](https://www.oracle.com/database/)

一个包含 `ol_core`（基础工具库）、`ol_network`（**Linux 主从 Reactor 多线程网络库**）、`ol_database`（MySQL/Oracle 数据库交互）及 `ol_ftp`（FTP 客户端）的 C++17 工具库，提供模块化编译、跨平台支持及 Linux 专属高性能网络通信能力。

> **作者：ol木子李lo（简称:ol）**
>
> **GitHub项目地址**：[https://github.com/1613661434/OL](https://github.com/1613661434/OL)

## 📜 许可证信息

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](https://opensource.org/licenses/MIT)

本项目所有代码均以 **MIT License** 许可证发布，完整法律文本见 [MIT 官方协议](https://opensource.org/licenses/MIT)。

### MIT License 核心条款

你可以**自由地**：

- **使用**：以任何目的（包括商业用途）运行、复制、使用本项目代码或文档；
- **修改**：对代码进行修改、扩展、集成到其他项目（包括商业项目），无需额外申请授权；
- **分发**：复制、分发本项目代码或衍生作品（包括商用分发）。

需遵守的**唯一核心义务**：

- **保留声明**：使用、修改或分发本项目代码 / 衍生作品时，需在所有副本或实质性部分中保留原始的版权声明和 MIT 许可证权限通知。

## 🎯 核心功能

项目采用**模块化编译设计**，可按需开启 / 关闭任意功能模块，轻量化部署：

- `ol_core`：**核心基础库（必选）**
  提供高频通用能力：文件 IO、时间处理、字符串操作、动态线程池（支持固定/动态扩缩容双模式）、通用数据结构（哈希、前缀树等）。

- `ol_network`：**高性能网络库（仅限 Linux）**
  基于 epoll 实现的主从 Reactor 多线程网络库，支持非阻塞 IO、边缘触发（ET）；
  架构：1 主 Reactor 监听连接 + N 从 Reactor 处理 IO，适用于高并发 TCP 服务端开发。

- `ol_database`：**多数据库交互模块**
  统一接口封装，支持双数据库，已适配连接池，可独立开关：

    - MySQL：基于 MySQL C API 封装，支持连接管理、SQL 执行、BLOB/TEXT 大字段；

    - Oracle：基于 OCI 接口封装，支持连接管理、SQL 执行、BLOB/CLOB 大字段。

- `ol_ftp`：**FTP 客户端模块**
  基于内置第三方库 `ftplib` 实现，支持文件上传 / 下载、目录操作、文件列表获取。

## 📚 代码文档规范

本项目所有头文件（`.h`）均遵循 **Doxygen 注释规范**：

- 函数 / 类 / 结构体包含 `@brief`（功能）、`@param`（参数）、`@return`（返回值）、`@note`（注意事项）；
- 描述文字使用中文，面向开发者的信息（异常、断言、错误消息）使用英文；
- 支持自动生成 HTML/PDF API 文档；
- 主流 IDE（VS Code、CLion）原生支持智能提示。

### 示例注释

```cpp
/**
 * @brief 根据绝对路径逐级创建目录
 * @param pathorfilename 绝对路径的文件名或目录名
 * @param bisfilename 指定pathorfilename类型（true-文件名，false-目录名，默认true）
 * @return true-成功，false-失败
 */
bool newdir(const std::string& pathorfilename, bool bisfilename = true);
```

## ⚙️ 环境依赖

### 基础依赖

- 构建工具：CMake 3.10+
- 编译器：C++17 兼容（GCC 8+/Clang 7+/MSVC 2019+/MinGW 8+）
- 操作系统：
    - `ol_network`：**仅限 Linux（内核 2.6+，支持 epoll）**
    - 其他模块：Windows 10+/Linux CentOS 7+/Ubuntu 18.04+

### 模块特定依赖

|模块|依赖说明|
|---|---|
|ol_network|Linux 内核原生 epoll，无额外依赖|
|ol_database(MySQL)|MySQL 客户端 5.7+，需配置 `MYSQL_HOME`|
|ol_database(Oracle)|Oracle 客户端 11g+，需配置 `ORACLE_HOME`|
|ol_ftp|内置 ftplib，无需额外安装|

## 🔧 CMake 配置变量说明

所有变量通过 `cmake -D<变量名>=<值>` 配置。

### 1. 通用配置

|变量名|默认值|可选值|说明|
|---|---|---|---|
|CMAKE_BUILD_TYPE|Release|Debug/Release|编译类型|
|ENABLE_WARNINGS|OFF|ON/OFF|开启编译器警告|
|OL_BUILD_STATIC_LIBS|ON|ON/OFF|编译所有模块静态库（开启测试时强制启用）|
|OL_BUILD_SHARED_LIBS|ON|ON/OFF|编译所有模块动态库|

### 2. 功能模块总开关

|变量名|默认值|说明|
|---|---|---|
|OL_BUILD_CORE|ON|核心基础库（**强制开启**）|
|OL_BUILD_FTP|OFF|编译 FTP 客户端模块|
|OL_BUILD_NETWORK|OFF|编译 Linux 网络库模块|
|OL_BUILD_DATABASE|OFF|编译数据库模块|

### 3. 数据库子模块开关

|变量名|默认值|说明|
|---|---|---|
|OL_BUILD_MYSQL|ON|编译 MySQL 支持|
|OL_BUILD_ORACLE|OFF|编译 Oracle 支持|

### 4. 测试程序开关

|变量名|默认值|说明|
|---|---|---|
|OL_CORE_WITH_TESTS|OFF|编译核心库测试|
|OL_FTP_WITH_TESTS|OFF|编译 FTP 测试|
|OL_NETWORK_WITH_TESTS|OFF|编译网络库测试|
|OL_MYSQL_WITH_TESTS|OFF|编译 MySQL 测试|
|OL_ORACLE_WITH_TESTS|OFF|编译 Oracle 测试|

### 配置示例

```bash
# 示例1：Linux 编译 Debug 版 + 开启网络库 + 启用网络测试
cmake .. -DCMAKE_BUILD_TYPE=Debug -DOL_BUILD_NETWORK=ON -DOL_NETWORK_WITH_TESTS=ON

# 示例2：Windows 仅编译核心+MySQL，关闭动态库
cmake .. -DOL_BUILD_DATABASE=ON -DOL_BUILD_MYSQL=ON -DOL_BUILD_ORACLE=OFF -DOL_BUILD_SHARED_LIBS=OFF

# 示例3：Linux 开启 Oracle + 测试（关闭 MySQL）
cmake .. -DOL_BUILD_DATABASE=ON -DOL_BUILD_MYSQL=OFF -DOL_BUILD_ORACLE=ON -DOL_ORACLE_WITH_TESTS=ON

# 示例4：全功能开启（核心+网络+数据库+FTP+所有测试）
cmake .. -DOL_BUILD_FTP=ON -DOL_BUILD_NETWORK=ON -DOL_BUILD_MYSQL=ON -DOL_BUILD_ORACLE=ON -D*_WITH_TESTS=ON
```

## 📝 编码规范

1. **字符集**：所有文件采用 **UTF-8（无 BOM）**，跨平台无乱码；
2. **换行符**：统一使用 **LF（\n）**，避免 Git 冲突；
3. **编译约束**：禁止源码内编译（out-of-source build），必须创建独立 build 目录；
4. **调试宏**：项目使用 `OL_DEBUG` 宏控制调试输出。

## 🔨 编译步骤

### Linux 平台（支持网络库）

```bash
# 1. 拉取代码
git clone https://github.com/1613661434/OL.git
cd OL

# 2. 创建编译目录
mkdir build && cd build

# 3. 配置 CMake（按需添加模块开关）
cmake ..  # 基础版：核心+数据库
# cmake .. -DOL_BUILD_NETWORK=ON  # 开启网络库
# cmake .. -DOL_BUILD_FTP=ON      # 开启FTP

# 4. 并行编译
make -j4

# 5. 产物输出
# 库文件：各模块 lib 目录
# 测试程序：各模块 test/bin 目录
```

### Windows 平台（不支持网络库）

#### MSVC 编译

```bash
git clone https://github.com/1613661434/OL.git
cd OL
mkdir build && cd build
cmake .. -G"Visual Studio 17 2022"
# 用 VS 打开解决方案编译
```

#### MinGW 编译

```bash
git clone https://github.com/1613661434/OL.git
cd OL
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
mingw32-make -j4
```

## ⚠️ 注意事项

1. **平台限制**：`ol_network` 仅支持 Linux，Windows 下自动跳过编译；
2. **测试依赖**：开启任意测试后，CMake **强制启用静态库编译**；
3. **第三方库**：`third_party/ftplib` 为内置依赖，禁止修改目录结构；
4. **数据库适配**：编译前必须配置对应数据库的环境变量（`MYSQL_HOME`/`ORACLE_HOME`）；
5. **Windows 乱码**：PowerShell 执行编码命令可修复中文乱码。

### Windows 终端乱码修复

```powershell
$OutputEncoding = [Console::InputEncoding] = [Console::OutputEncoding] = [System.Text.UTF8Encoding]::UTF8
```

## 📋 项目目录结构

```
OL
├── CMakeLists.txt            # 顶层构建配置
├── README.md                 # 项目说明文档
├── LICENSE                   # 许可证
├── clang-format.txt          # 代码格式化配置
│
├── ol_core/                  # 核心基础工具库（必选）
│   ├── include/              ## 头文件（ThreadPool、容器、字符串等）
│   ├── src/                  ## 源文件
│   └── test/                 ## 测试程序
│
├── ol_database/              # 数据库模块
│   ├── include/              ## IDBConn 抽象接口 + DBPool 连接池
│   ├── mysql/                ## MySQL 子模块
│   │   ├── include/          ### 头文件
│   │   ├── src/              ### 源文件
│   │   └── test/             ### 测试程序 + 测试数据
│   └── oracle/               ## Oracle 子模块
│       ├── include/          ### 头文件
│       ├── src/              ### 源文件
│       └── test/             ### 测试程序 + 测试数据
│
├── ol_network/               # Linux 网络库
│   ├── include/ol_net/       ## 头文件
│   ├── src/                  ## 源文件
│   └── test/                 ## 测试程序（Echo/Bank 示例）
│
├── ol_ftp/                   # FTP 客户端模块
│
└── docs/                     # 项目文档
```

## 📋 ol_core 核心库详解

### 动态线程池 `ol::ThreadPool`

- **固定模式**：`ol::ThreadPool<false>` — 固定线程数，简单可靠；
- **动态模式**：`ol::ThreadPool<true>` — 根据任务负载自动扩缩容，管理者线程定期检查；
- **队列策略**：满队列时支持拒绝（kReject）、阻塞等待（kBlock）、超时等待（kTimeout）三种策略；
- **双任务接口**：`addTask()` 无返回值，`submitTask()` 返回 `std::future`；
- **线程安全**：全原子变量 + 互斥锁 + 条件变量，适配 `ol::DBPool` 等高性能场景。

## 📋 ol_network 网络库详解（Linux 专属）

### 架构设计

**主从 Reactor 多线程模型**：

- 主 Reactor：单线程，负责监听新连接；
- 从 Reactor：多线程（默认 = CPU 核心数），负责 IO 事件处理；
- IO 模型：epoll + 边缘触发（ET）+ 非阻塞 IO，高并发低延迟。

### 核心组件

`EventLoop`（事件循环）、`Acceptor`（连接接收器）、`Connection`（连接管理）、`Buffer`（网络缓冲区）、`TcpServer`（服务端入口）。

### 适用场景

高并发 TCP 服务器、网关服务、游戏后端、自定义协议通信服务。