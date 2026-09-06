# OL

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](https://opensource.org/licenses/MIT)
[![C++](https://img.shields.io/badge/C%2B%2B-17%2B-%2300599C?logo=c%2B%2B)](https://isocpp.org/)
[![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20Windows-%23808080?logo=linux)](https://github.com/1613661434/OL)
[![CMake](https://img.shields.io/badge/build-CMake%203.10%2B-%23064F8C?logo=cmake)](https://cmake.org/)
[![MySQL](https://img.shields.io/badge/database-MySQL%205.7%2B-%234479A1?logo=mysql)](https://www.mysql.com/)
[![Oracle](https://img.shields.io/badge/database-Oracle%2011g%2B-%23F80000?logo=oracle)](https://www.oracle.com/database/)

A C++17 utility library featuring `ol_core` (core utilities), `ol_network` (**Linux master/worker Reactor network library**), `ol_database` (MySQL/Oracle integration), and `ol_ftp` (FTP client), with modular compilation, cross-platform support, and Linux-native high-performance networking.

> **Author: ol木子李lo（aka: ol）**
>
> **GitHub**: [https://github.com/1613661434/OL](https://github.com/1613661434/OL)

## 📜 License

This project is licensed under the **MIT License**. See [MIT License](https://opensource.org/licenses/MIT) for the full legal text.

### MIT License Summary

You are **free to**:

- **Use**: Run, copy, and use the code or documentation for any purpose, including commercial use;
- **Modify**: Modify, extend, and integrate the code into other projects (including commercial projects) without additional authorization;
- **Distribute**: Copy and distribute the code or derivative works (including commercial distribution).

The **only obligation**:

- **Attribution**: Include the original copyright notice and MIT license permission notice in all copies or substantial portions of the software.

## 🎯 Features

Modular compilation design — enable or disable modules on demand:

- `ol_core`: **Core library (required)**
  High-frequency utilities: file I/O, timestamps, string operations, dynamic thread pool (fixed/auto-scaling modes), data structures (hash, trie, etc.).

- `ol_network`: **High-performance network library (Linux only)**
  Master/worker Reactor pattern based on epoll, non-blocking I/O with edge-triggered (ET) mode. Architecture: 1 master Reactor for accepting connections + N worker Reactors for I/O processing, suitable for high-concurrency TCP server development.

- `ol_database`: **Multi-database module**
  Unified interface with connection pool support, independently toggleable:
    - MySQL: MySQL C API wrapper — connection management, SQL execution, BLOB/TEXT support;
    - Oracle: OCI wrapper — connection management, SQL execution, BLOB/CLOB support, `ol::DBPool` compatible.

- `ol_ftp`: **FTP client module**
  Based on built-in third-party library `ftplib`: file upload/download, directory operations, file listings.

## 📚 Documentation Conventions

All headers (`.h`) follow the **Doxygen** comment style:

- Functions/classes/structs include `@brief`, `@param`, `@return`, and `@note` tags;
- Descriptions in Chinese, developer-facing messages (exceptions, assertions, errors) in English;
- Auto-generate HTML/PDF API docs;
- Modern IDEs (VS Code, CLion) provide native IntelliSense support.

### Example

```cpp
/**
 * @brief Create directories recursively by absolute path
 * @param pathorfilename Absolute path to a file or directory
 * @param bisfilename Whether pathorfilename is a file (true) or directory (false), default true
 * @return true on success, false on failure
 */
bool newdir(const std::string& pathorfilename, bool bisfilename = true);
```

## ⚙️ Requirements

### Base Requirements

- Build tool: CMake 3.10+
- Compiler: C++17 compliant (GCC 8+/Clang 7+/MSVC 2019+/MinGW 8+)
- Operating System:
    - `ol_network`: **Linux only (kernel 2.6+, epoll support)**
    - Other modules: Windows 10+/Linux CentOS 7+/Ubuntu 18.04+

### Per-Module Dependencies

|Module|Dependency|
|---|---|
|ol_network|Linux kernel epoll, no extra dependencies|
|ol_database(MySQL)|MySQL client 5.7+, set `MYSQL_HOME`|
|ol_database(Oracle)|Oracle client 11g+, set `ORACLE_HOME`|
|ol_ftp|Built-in ftplib, no extra installation needed|

## 🔧 CMake Configuration

All variables are set via `cmake -D<var>=<value>`.

### 1. General Settings

|Variable|Default|Options|Description|
|---|---|---|---|
|CMAKE_BUILD_TYPE|Release|Debug/Release|Build type|
|ENABLE_WARNINGS|OFF|ON/OFF|Enable compiler warnings|
|OL_ENABLE_DEBUG_LOGS|OFF|ON/OFF|Define `OL_DEBUG` for all modules and enable debug logs|
|OL_BUILD_STATIC_LIBS|ON|ON/OFF|Build static libraries (forced ON when tests enabled)|
|OL_BUILD_SHARED_LIBS|ON|ON/OFF|Build shared libraries|

### 2. Module Switches

|Variable|Default|Description|
|---|---|---|
|OL_BUILD_CORE|ON|Core library (**always required**)|
|OL_BUILD_FTP|OFF|Build FTP client module|
|OL_BUILD_NETWORK|OFF|Build Linux network library module|
|OL_BUILD_DATABASE|OFF|Build database module|

### 3. Database Sub-Module Switches

|Variable|Default|Description|
|---|---|---|
|OL_BUILD_MYSQL|ON|Build MySQL support|
|OL_BUILD_ORACLE|OFF|Build Oracle support|

### 4. Test Switches

|Variable|Default|Description|
|---|---|---|
|OL_CORE_WITH_TESTS|OFF|Build core library tests|
|OL_FTP_WITH_TESTS|OFF|Build FTP tests|
|OL_NETWORK_WITH_TESTS|OFF|Build network library tests|
|OL_MYSQL_WITH_TESTS|OFF|Build MySQL tests|
|OL_ORACLE_WITH_TESTS|OFF|Build Oracle tests|

These switches only **build** test programs; they do not run them automatically. Once tests are enabled, use CTest as follows:

```bash
# List registered automatic tests
ctest --test-dir build -N

# Run one test
ctest --test-dir build -R "^test_ol_cqueue$" --output-on-failure

# Run all automatic tests
ctest --test-dir build --output-on-failure -j 4
```

Tests that require terminal interaction, an external FTP/database service, IPC, or a separately started server and client are not registered as automatic tests. Continue to run those executables manually.

### Configuration Examples

```bash
# Example 1: Linux Debug + network library + network tests
cmake .. -DCMAKE_BUILD_TYPE=Debug -DOL_BUILD_NETWORK=ON -DOL_NETWORK_WITH_TESTS=ON

# Example 2: Windows core + MySQL only, no shared libs
cmake .. -DOL_BUILD_DATABASE=ON -DOL_BUILD_MYSQL=ON -DOL_BUILD_ORACLE=OFF -DOL_BUILD_SHARED_LIBS=OFF

# Example 3: Linux Oracle + tests (disable MySQL)
cmake .. -DOL_BUILD_DATABASE=ON -DOL_BUILD_MYSQL=OFF -DOL_BUILD_ORACLE=ON -DOL_ORACLE_WITH_TESTS=ON

# Example 4: Full build (core + network + database + FTP + all test programs)
cmake .. -DOL_BUILD_FTP=ON -DOL_BUILD_NETWORK=ON -DOL_BUILD_DATABASE=ON -DOL_BUILD_MYSQL=ON -DOL_BUILD_ORACLE=ON -DOL_CORE_WITH_TESTS=ON -DOL_FTP_WITH_TESTS=ON -DOL_NETWORK_WITH_TESTS=ON -DOL_MYSQL_WITH_TESTS=ON -DOL_ORACLE_WITH_TESTS=ON
```

## 📝 Coding Conventions

1. **Charset**: UTF-8 (no BOM), consistent across all platforms;
2. **Line endings**: LF (\n), avoid Git conflicts;
3. **Out-of-source build**: Always create a separate build directory;
4. **Debug macro**: Use `OL_DEBUG` for debug output. Enable it globally with `-DOL_ENABLE_DEBUG_LOGS=ON`, or define it in an individual source file.

## 🔨 Build Instructions

### Linux (network library supported)

```bash
# 1. Clone
git clone https://github.com/1613661434/OL.git
cd OL

# 2. Create build directory
mkdir build && cd build

# 3. Configure CMake (add module switches as needed)
cmake ..  # Basic: core + database
# cmake .. -DOL_BUILD_NETWORK=ON  # Enable network library
# cmake .. -DOL_BUILD_FTP=ON      # Enable FTP

# 4. Build in parallel
make -j4

# 5. Output
# Libraries: <module>/lib
# Test binaries: <module>/test/bin
```

### Windows (network library not supported)

#### MSVC

```bash
git clone https://github.com/1613661434/OL.git
cd OL
mkdir build && cd build
cmake .. -G"Visual Studio 17 2022"
# Open the solution in Visual Studio to build
```

#### MinGW

```bash
git clone https://github.com/1613661434/OL.git
cd OL
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
mingw32-make -j4
```

## ⚠️ Notes

1. **Platform**: `ol_network` is Linux-only; automatically skipped on Windows;
2. **Tests**: Enabling any test forces static library build;
3. **Third-party**: `third_party/ftplib` is a built-in dependency; do not modify its structure;
4. **Database**: Set `MYSQL_HOME`/`ORACLE_HOME` before building the corresponding database module;
5. **Windows terminal**: Use the PowerShell command below to fix Chinese text garbling.

### Windows Terminal Encoding Fix

```powershell
$OutputEncoding = [Console::InputEncoding] = [Console::OutputEncoding] = [System.Text.UTF8Encoding]::UTF8
```

## 📋 Project Structure

```
OL
├── CMakeLists.txt            # Top-level build configuration
├── README.md                 # Project documentation (Chinese)
├── README_EN.md              # Project documentation (English)
├── LICENSE                   # MIT License
├── clang-format.txt          # Code formatting configuration
├── OL_Doxygen                # Doxygen documentation configuration
│
├── ol_core/                  # Core utility library (required)
│   ├── include/              #   Headers (ThreadPool, containers, strings, etc.)
│   ├── src/                  #   Sources
│   └── test/                 #   Tests
│
├── ol_database/              # Database module
│   ├── include/              #   IDBConn abstract interface + DBPool connection pool
│   ├── mysql/                #   MySQL sub-module
│   │   ├── include/          #     Headers
│   │   ├── src/              #     Sources
│   │   └── test/             #     Tests + test data
│   └── oracle/               #   Oracle sub-module
│       ├── include/          #     Headers
│       ├── src/              #     Sources
│       └── test/             #     Tests + test data
│
├── ol_network/               # Linux network library
│   ├── include/ol_net/       #   Headers
│   ├── src/                  #   Sources
│   └── test/                 #   Tests (Echo/Bank examples)
│
├── ol_ftp/                   # FTP client module
│   ├── include/              #   Headers
│   ├── src/                  #   Sources
│   ├── test/                 #   Tests
│   └── third_party/ftplib/   #   FTP core library (built-in dependency)
│
└── docs/                     # Documentation
```

## 📋 ol_core Library Highlights

### Dynamic Thread Pool `ol::ThreadPool`

- **Fixed mode**: `ol::ThreadPool<false>` — fixed number of threads, simple and reliable;
- **Dynamic mode**: `ol::ThreadPool<true>` — auto-scaling based on task load, managed by a supervisor thread;
- **Queue policies**: Reject (`kReject`), block-wait (`kBlock`), or timeout-wait (`kTimeout`) when the task queue is full;
- **Dual task interfaces**: `addTask()` for fire-and-forget, `submitTask()` returns `std::future`;
- **Thread-safe**: atomics + mutexes + condition variables, suitable for use alongside `ol::DBPool`.

## 📋 ol_network Library (Linux Only)

### Architecture

**Master/Worker Reactor Pattern**:

- Master Reactor: single-threaded, accepts new connections;
- Worker Reactors: multi-threaded (default = CPU cores), handle I/O events;
- I/O model: epoll + edge-triggered (ET) + non-blocking I/O, high concurrency with low latency.

### Core Components

`EventLoop`, `Acceptor`, `Connection`, `Buffer`, `TcpServer`.

### Use Cases

High-concurrency TCP servers, gateways, game backends, custom protocol communication services.
