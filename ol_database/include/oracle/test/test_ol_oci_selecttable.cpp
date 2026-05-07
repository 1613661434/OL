/*
 *  程序名：test_ol_oci_selecttable.cpp，此程序演示开发框架操作Oracle数据库（查询表中的数据）。
 *  作者：ol
 */
#include "ol_oci.h" // 开发框架操作Oracle的头文件。

using namespace std;
using namespace ol::oracle;

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

    DBStmt stmt(&conn);

    int minid = 11, maxid = 13;
    struct st_girl
    {
        long id;        // 超女编号，用long数据类型对应Oracle无小数的number(10)。
        char name[31];  // 超女姓名，用char[31]对应Oracle的varchar2(30)。
        double weight;  // 超女体重，用double数据类型对应Oracle有小数的number(8,2)。
        char btime[20]; // 报名时间，用char对应Oracle的date，格式：'yyyy-mm-dd hh24:mi:ssi'。
        char memo[301]; // 备注，用char[301]对应Oracle的varchar2(300)。
    } stgirl;

    // 准备查询表的SQL语句，prepare()方法不需要判断返回值。
    stmt.prepare("select id,name,weight,to_char(btime,'yyyy-mm-dd hh24:mi:ss'),memo from girls where id>=11 and id<=13");
    // 为SQL语句绑定输入变量的地址，bindin()方法不需要判断返回值。
    stmt.bindin(1, minid);
    stmt.bindin(2, maxid);
    // 把查询语句的结果集与变量的地址绑定，bindout()方法不需要判断返回值。
    stmt.bindout(1, stgirl.id);
    stmt.bindout(2, stgirl.name, 30);
    stmt.bindout(3, stgirl.weight);
    stmt.bindout(4, stgirl.btime, 19);
    stmt.bindout(5, stgirl.memo, 300);

    // 执行SQL语句，一定要判断返回值，0-成功，其它-失败。
    // 失败代码在stmt.m_cda.rc中，失败描述在stmt.m_cda.message中。
    if (stmt.execute() != 0)
    {
        printf("stmt.execute() failed.\n%s\n%s\n", stmt.sql(), stmt.message());
        return -1;
    }

    // 本程序执行的是查询语句，执行stmt.execute()后，将会在数据库的缓冲区中产生一个结果集。
    while (true)
    {
        memset(&stgirl, 0, sizeof(stgirl)); // 先把结构体变量初始化。

        // 从结果集中获取一条记录，一定要判断返回值，0-成功，1403-无记录，其它-失败。
        // 在实际开发中，除了0和1403，其它的情况极少出现。
        if (stmt.next() != 0) break;

        // 把获取到的记录的值打印出来。
        printf("id=%ld,name=%s,weight=%.02f,btime=%s,memo=%s\n", stgirl.id, stgirl.name, stgirl.weight, stgirl.btime, stgirl.memo);
    }

    // 请注意，stmt.m_cda.rpc变量非常重要，它保存了SQL被执行后影响的记录数。
    printf("本次查询了girls表%ld条记录。\n", stmt.rpc());

    return 0;
}