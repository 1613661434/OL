/*
 *  程序名：test_ol_oci_inserttable.cpp，此程序演示开发框架操作Oracle数据库（向表中插入数据）。
 *  作者：ol
 */
#include "ol_oci.h" // 开发框架操作Oracle的头文件。

using namespace std;
using namespace ol;

int main(int argc, char* argv[])
{
    DBConn conn; // 创建数据库连接类的对象。

    // 登录数据库，返回值：0-成功，其它-失败。
    // 失败代码在conn.m_cda.rc中，失败描述在conn.m_cda.message中。
    if (conn.connecttodb("scott/000888@snorcl11g_5", "Simplified Chinese_China.AL32UTF8") != 0)
    {
        printf("connect database failed.\n%s\n", conn.message());
        return -1;
    }

    printf("connect database ok.\n");

    // DBStmt stmt;       // 操作SQL语句的对象。
    // stmt.connect(&conn);  // 指定stmt对象使用的数据库连接。
    DBStmt stmt(&conn);

    // 准备插入表的SQL语句。
    /*
    // 静态SQL语句，适用于一次性执行的SQL。1）效率不如动态SQL语句高；2）特殊字符不方便处理；3）安全性（SQL注入）。
    for (int ii=10;ii<15;ii++)
    {
        stmt.prepare("\
                insert into girls(id,name,weight,btime,memo) \
                    values(%d,'西施%05dgirl',%.1f,to_date('2000-01-01 12:30:%02d','yyyy-mm-dd hh24:mi:ss'),\
                               '这是''第%05d个超级女生的备注。')",ii,ii,45.35+ii,ii,ii);

        // 执行SQL语句，一定要判断返回值，0-成功，其它-失败。
        // 失败代码在stmt.m_cda.rc中，失败描述在stmt.m_cda.message中。
        if (stmt.execute() != 0)
        {
            printf("stmt.execute() failed.\n%s\n%s\n",stmt.sql(),stmt.message()); return -1;
        }

        printf("成功插入了%ld条记录。\n",stmt.rpc()); // stmt.m_cda.rpc是本次执行SQL影响的记录数。
    }
    */

    // 1）如果字段是字符串型，绑定的变量可以用char[]，也可以用string，推荐用char[]。
    // 2）如果字段是字符串型，bindin()的第三个参数填字段的长度，太小可能会有问题，不推荐缺省值2000。
    // 3）动态SQL语句的字段也可以填静态的值。
    // 4）绑定的变量，一般用结构体。
    struct st_girl
    {
        long id;        // 超女编号，用long数据类型对应Oracle无小数的number(10)。
        char name[31];  // 超女姓名，用char[31]对应Oracle的varchar2(30)。
        double weight;  // 超女体重，用double数据类型对应Oracle有小数的number(8,2)。
        char btime[20]; // 报名时间，用char对应Oracle的date，格式：'yyyy-mm-dd hh24:mi:ssi'。
        char memo[301]; // 备注，用char[301]对应Oracle的varchar2(300)。
    } stgirl;

    // 动态SQL语句，适用于多次执行的SQL。
    stmt.prepare("insert into girls(id,name,weight,btime,memo) \
                                         values(:1,:2,:3,to_date(:4,'yyyy-mm-dd hh24:mi:ss'),:5)"); // :1,:2,...,:n可以理解为输入参数。
    stmt.bindin(1, stgirl.id);
    stmt.bindin(2, stgirl.name, 30);
    stmt.bindin(3, stgirl.weight);
    stmt.bindin(4, stgirl.btime, 19);
    stmt.bindin(5, stgirl.memo, 300); // 字符串的长度可以不指定，缺省是2000，这种用法不严瑾，不建议。

    // 对变量赋值，执行SQL语句。
    for (int ii = 10; ii < 15; ii++)
    {
        // 初始化变量。
        memset(&stgirl, 0, sizeof(struct st_girl));

        // 为变量赋值。
        stgirl.id = ii;                                            // 超女编号。
        sprintf(stgirl.name, "西施%05dgirl", ii);                  // 超女姓名。
        stgirl.weight = 45.35 + ii;                                // 超女体重。
        sprintf(stgirl.btime, "2021-08-25 10:33:%02d", ii);        // 报名时间。
        sprintf(stgirl.memo, "这是'第%05d个超级女生的备注。", ii); // 备注。

        // 执行SQL语句，一定要判断返回值，0-成功，其它-失败。
        // 失败代码在stmt.m_cda.rc中，失败描述在stmt.m_cda.message中。
        if (stmt.execute() != 0)
        {
            printf("stmt.execute() failed.\n%s\n%s\n", stmt.sql(), stmt.message());
            return -1;
        }

        printf("成功插入了%ld条记录。\n", stmt.rpc()); // stmt.m_cda.rpc是本次执行SQL语句影响的记录数。
    }

    conn.commit(); // 提交事务。

    return 0;
}