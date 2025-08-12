/*
 *  程序名：test_ol_ooci_updatetable.cpp，此程序演示开发框架操作Oracle数据库（修改表中的数据）。
 *  作者：ol
 */
#include "ol_oci.h" // 开发框架操作Oracle的头文件。

using namespace std;
using namespace ol;

int main(int argc, char* argv[])
{
    connection conn; // 创建数据库连接类的对象。

    // 登录数据库，返回值：0-成功，其它-失败。
    // 失败代码在conn.m_cda.rc中，失败描述在conn.m_cda.message中。
    if (conn.connecttodb("scott/000888@snorcl11g_5", "Simplified Chinese_China.AL32UTF8") != 0)
    {
        printf("connect database failed.\n%s\n", conn.message());
        return -1;
    }

    printf("connect database ok.\n");

    sqlstatement stmt(&conn);

    // 静态SQL语句。
    /*
    stmt.prepare("\
          update girls set name='冰冰',weight=45.2,btime=to_date('2008-01-02 12:30:22','yyyy-mm-dd hh24:mi:ss') where id=10");
    */

    // 动态SQL语句。
    struct st_girl
    {
        long id;        // 超女编号，用long数据类型对应Oracle无小数的number(10)。
        char name[31];  // 超女姓名，用char[31]对应Oracle的varchar2(30)。
        double weight;  // 超女体重，用double数据类型对应Oracle有小数的number(8,2)。
        char btime[20]; // 报名时间，用char对应Oracle的date，格式：'yyyy-mm-dd hh24:mi:ssi'。
        char memo[301]; // 备注，用char[301]对应Oracle的varchar2(300)。
    } stgirl;

    stmt.prepare("\
            update girls set name=:1,weight=:2,btime=to_date(:3,'yyyy-mm-dd hh24:mi:ss') where id=:4"); // :1,:2,...,:n可以理解为输入参数。
    stmt.bindin(1, stgirl.name, 30);
    stmt.bindin(2, stgirl.weight);
    stmt.bindin(3, stgirl.btime, 19);
    stmt.bindin(4, stgirl.id);

    // 初始化结构体，为变量赋值。
    memset(&stgirl, 0, sizeof(struct st_girl));
    stgirl.id = 11;               // 超女编号。
    sprintf(stgirl.name, "幂幂"); // 超女姓名。
    stgirl.weight = 43.85;        // 超女体重。
    strcpy(stgirl.btime, "2021-08-25 10:33:35");

    // 执行SQL语句，一定要判断返回值，0-成功，其它-失败。
    // 失败代码在stmt.m_cda.rc中，失败描述在stmt.m_cda.message中。
    if (stmt.execute() != 0)
    {
        printf("stmt.execute() failed.\n%s\n%s\n", stmt.sql(), stmt.message());
        return -1;
    }

    printf("成功修改了%ld条记录。\n", stmt.rpc()); // stmt.m_cda.rpc是本次执行SQL影响的记录数。

    conn.commit(); // 提交事务。

    return 0;
}