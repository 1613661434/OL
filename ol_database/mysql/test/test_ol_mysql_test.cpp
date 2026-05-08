/****************************************************************************************/
/*
 * 程序名：ol_mysql_test.cpp
 * 功能描述：MySQL连接池综合测试用例
 * 作者：ol
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

// 全局测试配置
const std::string MYSQL_CONN_STR = "root:0088@127.0.0.1:3306/testdb";
const std::string MYSQL_CHARSET = "utf8mb4";
const size_t MAX_CONN = 5; // 连接池最大连接数

// 打印测试结果辅助函数
void printTestResult(const std::string& testName, bool success)
{
    std::cout << "[" << (success ? "PASS" : "FAIL") << "] " << testName << std::endl;
}

// 测试1：单个MySQL连接基本操作
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

        // 4. 测试事务
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
            std::cerr << "获取查询结果失败: " << queryStmt->errorMsg() << std::endl;
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

// 测试3：数据库连接池核心功能
bool testDBPool()
{
    try
    {
        // 创建连接池：最大连接数 + 配置回调
        ol::DBPool<ol::mysql::DBConn> pool(
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

        // 获取连接
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

        // 释放连接
        pool.release(conn1);

        // 多线程测试连接池
        bool threadTestSuccess = true;
        auto threadFunc = [&](int threadId)
        {
            try
            {
                auto conn = pool.get();
                if (!conn)
                {
                    std::cerr << "线程" << threadId << "获取连接失败" << std::endl;
                    threadTestSuccess = false;
                    return;
                }
                // 模拟业务
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                conn->execute("SELECT 1;");
                pool.release(conn);
            }
            catch (const std::exception& e)
            {
                std::cerr << "线程" << threadId << "异常: " << e.what() << std::endl;
                threadTestSuccess = false;
            }
        };

        std::thread t1(threadFunc, 1);
        std::thread t2(threadFunc, 2);
        std::thread t3(threadFunc, 3);
        t1.join();
        t2.join();
        t3.join();

        if (!threadTestSuccess) return false;

        // 销毁连接池
        pool.destroy();
        return true;
    }
    catch (const std::exception& e)
    {
        std::cerr << "连接池测试异常: " << e.what() << std::endl;
        return false;
    }
}

// 测试4：清理测试环境
bool cleanTestEnv()
{
    try
    {
        ol::mysql::DBConn conn;
        conn.setConnectParam(MYSQL_CONN_STR, MYSQL_CHARSET);
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

int main()
{
    std::cout << "==================== MySQL连接池综合测试 ====================" << std::endl;

    // 执行测试用例
    printTestResult("单个MySQL连接基本操作", testSingleDBConn());
    printTestResult("数据库连接池核心功能", testDBPool());
    printTestResult("清理测试环境", cleanTestEnv());

    std::cout << "============================================================" << std::endl;
    std::cout << "所有测试完成！" << std::endl;

    return 0;
}