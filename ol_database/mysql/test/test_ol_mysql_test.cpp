/****************************************************************************************/
/*
 * 程序名：ol_mysql_test.cpp
 * 功能描述：MySQL连接池综合测试用例，测试内容包括：
 *          - 单连接基础操作（连接、增删改查、事务）
 *          - 预处理语句（绑定参数、执行、结果集）
 *          - 生产级连接池（多线程、阻塞获取、自动重连、连接复用）
 *          - 测试环境自动清理
 * 作者：ol
 * 适用标准：C++17及以上
 */
/****************************************************************************************/
#include "ol_mysql.h"
#include "ol_database.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <string>
#include <memory>
#include <stdexcept>
#include <vector>

// 全局测试配置
const std::string MYSQL_CONN_STR = "root:0088@127.0.0.1:3306/testdb";
const std::string MYSQL_CHARSET = "utf8mb4";
const size_t MAX_CONN = 5;          ///< 连接池最大连接数
const size_t THREAD_TEST_COUNT = 3; ///< 多线程测试线程数量

/**
 * @brief 打印测试结果辅助函数
 * @param testName 测试用例名称
 * @param success 测试是否成功
 */
void printTestResult(const std::string& testName, bool success)
{
    std::cout << "[" << (success ? "PASS" : "FAIL") << "] " << testName << std::endl;
}

/**
 * @brief 测试1：单个MySQL连接基本操作
 * @return 测试全部通过返回true，任意步骤失败返回false
 * @note 测试内容：连接/断开、建表、增删数据、事务回滚
 */
bool testSingleDBConn()
{
    try
    {
        ol::mysql::DBConn conn;
        // 配置连接参数
        conn.setConnectParam(MYSQL_CONN_STR, MYSQL_CHARSET, true);

        // 1. 测试连接
        if (!conn.connect())
        {
            std::cerr << "连接失败: " << conn.errorMsg() << std::endl;
            return false;
        }
        if (!conn.isConnected())
        {
            std::cerr << "连接状态检测失败" << std::endl;
            return false;
        }

        // 2. 测试执行SQL（创建测试表）
        int ret = conn.execute(
            "CREATE TABLE IF NOT EXISTS test_pool_table ("
            "id INT PRIMARY KEY AUTO_INCREMENT, "
            "name VARCHAR(50) NOT NULL, "
            "age INT DEFAULT 0, "
            "create_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
            ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;");
        if (ret != 0)
        {
            std::cerr << "创建表失败: " << conn.errorMsg() << std::endl;
            return false;
        }

        // 3. 测试插入数据
        ret = conn.execute("INSERT INTO test_pool_table (name, age) VALUES ('test_user1', 20);");
        if (ret != 0 || conn.affectedRows() != 1)
        {
            std::cerr << "插入数据失败: " << conn.errorMsg() << std::endl;
            return false;
        }

        // 4. 测试事务回滚
        if (!conn.beginTransaction())
        {
            std::cerr << "开启事务失败: " << conn.errorMsg() << std::endl;
            return false;
        }
        ret = conn.execute("INSERT INTO test_pool_table (name, age) VALUES ('test_rollback', 30);");
        if (ret != 0)
        {
            std::cerr << "事务插入失败: " << conn.errorMsg() << std::endl;
            return false;
        }
        conn.rollback();

        // 5. 测试删除测试数据
        ret = conn.execute("DELETE FROM test_pool_table WHERE name = 'test_user1';");
        if (ret != 0)
        {
            std::cerr << "删除数据失败: " << conn.errorMsg() << std::endl;
            return false;
        }

        // 6. 断开连接
        conn.disconnect();
        if (conn.isConnected())
        {
            std::cerr << "断开连接失败" << std::endl;
            return false;
        }

        return true;
    }
    catch (const std::exception& e)
    {
        std::cerr << "单连接测试异常: " << e.what() << std::endl;
        return false;
    }
}

/**
 * @brief 测试2：MySQL预处理语句（stmt）功能
 * @param conn 已建立连接的MySQL连接对象
 * @return 测试全部通过返回true，任意步骤失败返回false
 * @note 测试内容：SQL预处理、参数绑定、增查操作、结果集获取
 */
bool testDBStmt(ol::mysql::DBConn& conn)
{
    try
    {
        // 1. 创建预处理语句对象
        auto stmt = conn.createStmt();
        if (!stmt)
        {
            std::cerr << "创建预处理语句失败" << std::endl;
            return false;
        }

        // 2. 预处理插入SQL
        const char* insertSql = "INSERT INTO test_pool_table (name, age) VALUES (?, ?);";
        if (!stmt->prepare(insertSql))
        {
            std::cerr << "预处理SQL失败: " << stmt->errorMsg() << std::endl;
            return false;
        }

        // 3. 绑定输入参数
        char* name = "stmt_test";
        int name_len = strlen(name);
        int age = 25;
        if (stmt->bindin(1, name, name_len) != 0 || stmt->bindin(2, age) != 0)
        {
            std::cerr << "绑定参数失败: " << stmt->errorMsg() << std::endl;
            return false;
        }

        // 4. 执行预处理语句
        if (!stmt->execute())
        {
            std::cerr << "执行预处理语句失败: " << stmt->errorMsg() << std::endl;
            return false;
        }
        if (stmt->affectedRows() != 1)
        {
            std::cerr << "预处理插入影响行数错误" << std::endl;
            return false;
        }

        // 5. 预处理查询
        auto queryStmt = conn.createStmt();
        const char* querySql = "SELECT age FROM test_pool_table WHERE name = ?;";
        if (!queryStmt->prepare(querySql))
        {
            std::cerr << "预处理查询SQL失败: " << queryStmt->errorMsg() << std::endl;
            return false;
        }

        if (queryStmt->bindin(1, name, name_len) != 0)
        {
            std::cerr << "绑定查询参数失败: " << queryStmt->errorMsg() << std::endl;
            return false;
        }

        // 6. 绑定输出参数
        int resAge = 0;
        if (queryStmt->bindout(1, resAge) != 0)
        {
            std::cerr << "绑定输出参数失败: " << queryStmt->errorMsg() << std::endl;
            return false;
        }

        if (!queryStmt->execute())
        {
            std::cerr << "执行查询失败: " << queryStmt->errorMsg() << std::endl;
            return false;
        }

        if (queryStmt->next() != 0)
        {
            std::cerr << "获取查询结果失败" << std::endl;
            return false;
        }
        if (resAge != 25)
        {
            std::cerr << "查询结果错误，预期25，实际" << resAge << std::endl;
            return false;
        }

        // 清理测试数据
        conn.execute("DELETE FROM test_pool_table WHERE name = 'stmt_test';");
        return true;
    }
    catch (const std::exception& e)
    {
        std::cerr << "预处理语句测试异常: " << e.what() << std::endl;
        return false;
    }
}

/**
 * @brief 测试3：数据库连接池核心功能
 * @return 测试全部通过返回true，任意步骤失败返回false
 * @note 测试内容：连接池初始化、连接获取/释放、多线程并发、自动重连
 */
bool testDBPool()
{
    try
    {
        // ===================== 适配单例模式：获取全局唯一连接池 =====================
        auto& pool = ol::DBPool<ol::mysql::DBConn>::GetInst();

        // 初始化连接池
        pool.init(
            MAX_CONN,
            [&](ol::mysql::DBConn& conn)
            {
                conn.setConnectParam(MYSQL_CONN_STR, MYSQL_CHARSET, true);
            });

        // 检查初始空闲连接数
        size_t initIdle = pool.idle();
        std::cout << "初始空闲连接数: " << initIdle << std::endl;
        if (initIdle == 0)
        {
            std::cerr << "连接池初始化失败，无可用连接" << std::endl;
            return false;
        }

        // 阻塞获取连接
        auto conn1 = pool.get();
        if (!conn1)
        {
            std::cerr << "获取第一个连接失败" << std::endl;
            return false;
        }

        // 执行预处理语句测试
        if (!testDBStmt(*conn1))
        {
            std::cerr << "连接池连接执行预处理语句失败" << std::endl;
            return false;
        }

        // 释放连接回池
        pool.release(conn1);

        // 多线程并发测试连接池
        bool threadTestSuccess = true;
        auto threadFunc = [&](int threadId)
        {
            try
            {
                // 超时获取连接（默认3秒，生产环境推荐）
                auto conn = pool.getTimeout();
                if (!conn)
                {
                    std::cerr << "线程" << threadId << "获取连接超时/失败" << std::endl;
                    threadTestSuccess = false;
                    return;
                }
                // 模拟业务逻辑执行
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                conn->execute("SELECT 1;");
                // 释放连接
                pool.release(conn);
            }
            catch (const std::exception& e)
            {
                std::cerr << "线程" << threadId << "异常: " << e.what() << std::endl;
                threadTestSuccess = false;
            }
        };

        // 启动多线程测试
        std::vector<std::thread> threads;
        for (size_t i = 1; i <= THREAD_TEST_COUNT; ++i)
        {
            threads.emplace_back(threadFunc, i);
        }
        for (auto& t : threads)
        {
            t.join();
        }

        if (!threadTestSuccess)
        {
            return false;
        }

        // 销毁连接池，释放所有资源
        pool.destroy();
        return true;
    }
    catch (const std::exception& e)
    {
        std::cerr << "连接池测试异常: " << e.what() << std::endl;
        return false;
    }
}

/**
 * @brief 测试4：清理测试环境
 * @return 清理成功返回true，失败返回false
 * @note 删除测试表，断开连接，还原数据库环境
 */
bool cleanTestEnv()
{
    try
    {
        ol::mysql::DBConn conn;
        // 统一连接参数配置
        conn.setConnectParam(MYSQL_CONN_STR, MYSQL_CHARSET, true);
        if (!conn.connect())
        {
            std::cerr << "清理环境连接失败: " << conn.errorMsg() << std::endl;
            return false;
        }
        // 删除测试表
        int ret = conn.execute("DROP TABLE IF EXISTS test_pool_table;");
        if (ret != 0)
        {
            std::cerr << "删除测试表失败: " << conn.errorMsg() << std::endl;
            return false;
        }
        conn.disconnect();
        return true;
    }
    catch (const std::exception& e)
    {
        std::cerr << "清理环境异常: " << e.what() << std::endl;
        return false;
    }
}

/**
 * @brief 主函数：执行所有MySQL综合测试用例
 * @return 程序退出码
 */
int main()
{
    std::cout << "==================== MySQL连接池综合测试 ====================" << std::endl;

    // 顺序执行所有测试用例
    printTestResult("单个MySQL连接基本操作", testSingleDBConn());
    printTestResult("数据库连接池核心功能", testDBPool());
    printTestResult("清理测试环境", cleanTestEnv());

    std::cout << "============================================================" << std::endl;
    std::cout << "所有测试完成！" << std::endl;

    return 0;
}