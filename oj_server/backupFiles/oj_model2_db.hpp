#pragma once
// mysql版本
// model主要和数据进行交互，对外提供访问数据的接口
#include <iostream>
#include "../comm/util.hpp"
#include <string>
#include "../comm/log.hpp"
#include <unordered_map>
#include <vector>
#include <assert.h>
#include <fstream>

#include "include/mysql.h"
namespace ns_model
{
    using namespace std;
    using namespace ns_log;
    using namespace ns_util;
    struct Question
    {
        std::string number;
        std::string title;
        std::string star; // 难度
        int cpu_limit;    // 时间要求
        int mem_limit;
        std::string desc;   // 题目的描述
        std::string header; // 题目预设代码
        std::string tail;   // 题目的测试用例，需要和header拼接
    };

    class Model
    {
        // 两张表
        const std::string oj_questions = "questions";
        const std::string host = "127.0.0.1";
        const std::string user = "user1";
        const std::string passwd = "Ab12345@";
        // const std::string passwd="";
        const std::string db = "oj";
        const int port = 3306;

    public:
        Model()
        {
        }
        bool QueryMysql(const std::string &sql, std::vector<Question> *out)
        {
            MYSQL *my = mysql_init(nullptr);
            if (my == nullptr)
            {
                LOG(ERROR) << "初始化MYSQL失败!" << endl;
                std::cerr << "初始化 MySQL 失败！\n";
                return false;
            }

            // 禁用 SSL 连接
            unsigned int ssl_mode = SSL_MODE_DISABLED;
            mysql_options(my, MYSQL_OPT_SSL_MODE, &ssl_mode);

            if (nullptr == mysql_real_connect(my, host.c_str(), user.c_str(), passwd.c_str(), db.c_str(), port, nullptr, 0))
            {
                LOG(FATAL) << "连接数据库失败！"
                           << "\n";
                std::cerr << "连接数据库失败：" << mysql_error(my) << "\n";
                mysql_close(my);
                return false;
            }

            LOG(INFO) << "连接数据库成功！" << std::endl;
            // std::cout << "连接数据库成功！\n";

            // 一定要设置编码格式，默认是拉丁，会出现乱码
            mysql_set_character_set(my, "utf8");

            if (0 != mysql_query(my, sql.c_str()))
            {
                std::cerr << sql << " 执行错误：" << mysql_error(my) << "\n";
                mysql_close(my);
                return false;
            }

            MYSQL_RES *res = mysql_store_result(my);
            if (res == nullptr)
            {
                std::cerr << "获取查询结果失败：" << mysql_error(my) << "\n";
                mysql_close(my);
                return false;
            }

            int rows = mysql_num_rows(res);
            int cols = mysql_num_fields(res);

            for (int i = 0; i < rows; ++i)
            {
                MYSQL_ROW row = mysql_fetch_row(res);
                Question q;
                q.number = row[0];
                q.title = row[1];
                q.star = row[2];
                q.desc = row[3];
                q.header = row[4];
                q.tail = row[5];
                q.cpu_limit = atoi(row[6]);
                q.mem_limit = atoi(row[7]);
                out->push_back(q);
            }

            // 释放结果集
            mysql_free_result(res);
            // 关闭数据库连接
            mysql_close(my);

            return true;
        }
public:
        bool GetAllQuestions(vector<Question> *out)
        {
            std::string sql = "select * from ";
            sql += oj_questions;
            return QueryMysql(sql, out);
        }
        // 根据索引得到题目
        bool GetOneQuestion(const std::string &number, Question *q)
        {
            bool res = false;
            std::string sql = "select * from ";
            sql += oj_questions;
            sql += " where id=";
            sql += number;
            vector<Question> result;
            if (QueryMysql(sql, &result))
            {
                // 选择一道题
                if (result.size() == 1)
                {
                    *q = result[0];
                    res = true;
                }
            }
            return res;
        }
        ~Model() {}
        public:
        // bool GetCompletedQuestions(vector<Question> *out)
        // {
        //     std::string sql = "select * from ";
        //     sql+="relation where "
        //     return QueryMysql(sql, out);
        // }
    };
    /// 用户控制
    struct User
    {
        int id;
        std::string name; // 用户名或者邮箱
        std::string password;
        std::string salt; // 密码盐
        int isAdmin = 0; // 是否为管理员
        User(const std::string& n="" ,const std::string & p="",const std::string & s="")
            :name(n)
            ,password(p)
            ,salt(s)
            {}
    };
    class UserModel
    {
    private:
        const std::string oj_users = "user";
        const std::string host = "127.0.0.1";
        const std::string user = "user1";
        const std::string passwd = "Ab12345@";
        const std::string db = "oj";
        const int port = 3306;

    public:
        UserModel(){}
        ~UserModel() {}
        // 添加用户
        bool AddUser(const User* u)
        {
            std::string sql = "INSERT INTO user (username, password,salt) VALUES ('" + u->name + "', '" + u->password + "', '" + u->salt + "')"; 
            return UpdateMysql(sql);
        }
        // 查询用户
        bool QueryUser(const std::string &username, User *u)
        {
            std::string sql = "select * from ";
            sql += oj_users;
            sql += " where username = '";
            sql += username;
            sql += "'";
            return QueryMysql(sql, u);
        }
    private:
        bool QueryMysql(const std::string &sql, User *&out)
        {
            MYSQL *my = mysql_init(nullptr);
            if (my == nullptr)
            {
                LOG(ERROR) << "初始化MYSQL失败!" << endl;
                std::cerr << "初始化 MySQL 失败！\n";
                return false;
            }

            // 禁用 SSL 连接
            unsigned int ssl_mode = SSL_MODE_DISABLED;
            mysql_options(my, MYSQL_OPT_SSL_MODE, &ssl_mode);

            if (nullptr == mysql_real_connect(my, host.c_str(), user.c_str(), passwd.c_str(), db.c_str(), port, nullptr, 0))
            {
                LOG(FATAL) << "连接数据库失败！"
                           << "\n";
                std::cerr << "连接数据库失败：" << mysql_error(my) << "\n";
                mysql_close(my);
                return false;
            }
            LOG(INFO) << "连接数据库成功！" << std::endl;
            // 一定要设置编码格式，默认是拉丁，会出现乱码
            mysql_set_character_set(my, "utf8");
            if (0 != mysql_query(my, sql.c_str()))
            {
                std::cerr << sql << " 执行错误：" << mysql_error(my) << "\n";
                mysql_close(my);
                return false;
            }
            MYSQL_RES *res = mysql_store_result(my);
            if (res == nullptr)
            {
                std::cerr << "获取查询结果失败：" << mysql_error(my) << "\n";
                mysql_close(my);
                return false;
            }
            int rows = mysql_num_rows(res);
            int cols = mysql_num_fields(res);
            LOG(INFO) << "查询用户结果成功" << std::endl;
            for (int i = 0; i < rows; ++i)
            {
                MYSQL_ROW row = mysql_fetch_row(res);
                User user;
                user.id = atoi(row[0]);
                user.name = row[1];
                user.password = row[2];
                user.salt = row[3];
                user.isAdmin = atoi(row[4]);
                
                *out = user;
            }
            // 释放结果集
            mysql_free_result(res);
            // 关闭数据库连接
            mysql_close(my);
            return true;
        }

        bool UpdateMysql(const std::string &sql)
        {
            MYSQL *my = mysql_init(nullptr);
            if (my == nullptr)
            {
                LOG(ERROR) << "初始化MYSQL失败!" << std::endl;
                std::cerr << "初始化 MySQL 失败！\n";
                return false;
            }

            // 禁用 SSL 连接
            unsigned int ssl_mode = SSL_MODE_DISABLED;
            mysql_options(my, MYSQL_OPT_SSL_MODE, &ssl_mode);

            if (nullptr == mysql_real_connect(my, host.c_str(), user.c_str(), passwd.c_str(), db.c_str(), port, nullptr, 0))
            {
                LOG(FATAL) << "连接数据库失败！\n";
                std::cerr << "连接数据库失败：" << mysql_error(my) << "\n";
                mysql_close(my);
                return false;
            }

            // LOG(INFO) << "连接数据库成功！" << std::endl;
            // std::cout << "连接数据库成功！\n";

            // 一定要设置编码格式，默认是拉丁，会出现乱码
            mysql_set_character_set(my, "utf8");

            if (0 != mysql_query(my, sql.c_str()))
            {
                std::cerr << sql << " 执行错误：" << mysql_error(my) << "\n";
                std::cout<<"插入的用户名已经在表中存在"<<endl;
                mysql_close(my);
                return false;
            }
            // 关闭数据库连接
            mysql_close(my);
            return true;
        }
    };
}