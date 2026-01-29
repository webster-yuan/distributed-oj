#pragma once
// mysql版本
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <ctime>
#include "include/mysql.h"
#include "../comm/util.hpp"
#include "../comm/log.hpp"

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
        Question(const std::string &n = "",
                 const std::string &t = "",
                 const std::string &s = "",
                 int cl = 0,
                 int ml = 0,
                 const std::string &d = "",
                 const std::string &h = "",
                 const std::string &t1 = "")
            : number(n), title(t), star(s), cpu_limit(cl), mem_limit(ml), desc(d), header(h), tail(t1) {}
    };

    struct User
    {
        int id;
        std::string name; // 用户名或者邮箱
        std::string password;
        std::string salt; // 密码盐
        int isAdmin = 0;  // 是否为管理员
        User(const std::string &n = "", const std::string &p = "", const std::string &s = "")
            : name(n), password(p), salt(s) {}
    };

    struct UserProgress
    {
        int progressId;
        int userId;
        int questionId;
        bool isCompleted;
        time_t completedTime;
        int attemptCount;
        UserProgress(int id = 0, int u = 0, int q = 0, bool c = false, time_t t = 0, int a = 0)
            : progressId(id), userId(u), questionId(q), isCompleted(c), completedTime(t), attemptCount(a) {}
    };

    class MySQLConnection
    {
    private:
        const std::string host = "127.0.0.1";
        const std::string user = "user1";
        const std::string passwd = "Ab12345@";
        const std::string db = "oj";
        const int port = 3306;
        MYSQL *connection;

    public:
        MySQLConnection() : connection(mysql_init(nullptr))
        {
            if (connection == nullptr)
            {
                throw std::runtime_error("初始化 MySQL 失败！");
            }
            // 禁用 SSL 连接
            unsigned int ssl_mode = SSL_MODE_DISABLED;
            mysql_options(connection, MYSQL_OPT_SSL_MODE, &ssl_mode);

            if (nullptr == mysql_real_connect(connection, host.c_str(), user.c_str(), passwd.c_str(), db.c_str(), port, nullptr, 0))
            {
                mysql_close(connection);
                throw std::runtime_error("连接数据库失败：" + std::string(mysql_error(connection)));
            }
            mysql_set_character_set(connection, "utf8");
        }

        ~MySQLConnection()
        {
            if (connection != nullptr)
            {
                mysql_close(connection);
            }
        }

        MYSQL *get()
        {
            return connection;
        }
    };

    class Model
    {
    private:
        const std::string oj_questions = "questions";

    public:
        Model() {}
        ~Model() {}

        bool QueryMysql(const std::string &sql, std::vector<Question> *out)
        {
            try
            {
                MySQLConnection db;
                if (0 != mysql_query(db.get(), sql.c_str()))
                {
                    std::cerr << sql << " 执行错误：" << mysql_error(db.get()) << "\n";
                    return false;
                }
                MYSQL_RES *res = mysql_store_result(db.get());
                if (res == nullptr)
                {
                    std::cerr << "获取查询结果失败：" << mysql_error(db.get()) << "\n";
                    return false;
                }

                int rows = mysql_num_rows(res);
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
                mysql_free_result(res);
                return true;
            }
            catch (const std::runtime_error &e)
            {
                std::cerr << e.what() << '\n';
                return false;
            }
        }

    public:
        bool GetAllQuestions(vector<Question> *out)
        {
            std::string sql = "SELECT * FROM " + oj_questions;
            return QueryMysql(sql, out);
        }

        bool GetOneQuestion(const std::string &number, Question *q)
        {
            std::string sql = "SELECT * FROM " + oj_questions + " WHERE id=" + number;
            std::vector<Question> result;
            if (QueryMysql(sql, &result) && result.size() == 1)
            {
                *q = result[0];
                return true;
            }
            return false;
        }
        bool InsertQuestion(const Question &q)
        {
            std::cout << "InsertQuestion" << std::endl;

            // std::string sql = "INSERT INTO " + oj_questions + " (number, title, star, desc, header, tail, cpu_limit, mem_limit) VALUES ('" + q.number + "', '" + q.title + "', '" + q.star + "', '" + q.desc + "', '" + q.header + "', '" + q.tail + "', " + std::to_string(q.cpu_limit) + ", " + std::to_string(q.mem_limit) + ")";
            std::string sql = "INSERT INTO " + oj_questions +
                              " (title, star, question_desc, header, tail, time_limit, mem_limit) VALUES ('" +
                              q.title + "', '" + q.star + "', '" + q.desc + "', '" + q.header + "', '" +
                              q.tail + "', " + std::to_string(q.cpu_limit) + ", " + std::to_string(q.mem_limit) + ")";

            try
            {
                MySQLConnection db;
                if (0 != mysql_query(db.get(), sql.c_str()))
                {
                    std::cerr << "插入失败：" << mysql_error(db.get()) << "\n";
                    return false;
                }
                return true;
            }
            catch (const std::runtime_error &e)
            {
                std::cerr << e.what() << '\n';
                return false;
            }
        }
        bool UpdateQuestion(const Question &q)
        {
            std::string sql = "UPDATE " + oj_questions + " SET " +
                              "title='" + q.title + "', star='" + q.star + "', question_desc='" + q.desc +
                              "', header='" + q.header + "', tail='" + q.tail + "', time_limit=" +
                              std::to_string(q.cpu_limit) + ", mem_limit=" + std::to_string(q.mem_limit) +
                              " WHERE id='" + q.number + "'";
            try
            {
                MySQLConnection db;
                if (0 != mysql_query(db.get(), sql.c_str()))
                {
                    std::cerr << "更新失败：" << mysql_error(db.get()) << "\n";
                    return false;
                }
                return true;
            }
            catch (const std::runtime_error &e)
            {
                std::cerr << e.what() << '\n';
                return false;
            }
        }
        bool DeleteQuestion(const std::string &number)
        {
            std::string sql = "DELETE FROM " + oj_questions + " WHERE id ='" + number + "'";
            try
            {
                MySQLConnection db;
                if (0 != mysql_query(db.get(), sql.c_str()))
                {
                    std::cerr << "删除失败：" << mysql_error(db.get()) << "\n";
                    return false;
                }
                return true;
            }
            catch (const std::runtime_error &e)
            {
                std::cerr << e.what() << '\n';
                return false;
            }
        }
    };

    class UserModel
    {
    private:
        const std::string oj_users = "user";

    public:
        UserModel() {}
        ~UserModel() {}

        bool AddUser(const User *u)
        {
            std::string sql = "INSERT INTO user (username, password, salt) VALUES ('" + u->name + "', '" + u->password + "', '" + u->salt + "')";
            return UpdateMysql(sql);
        }
        bool InsertUserByAdmin(const User *u)
        {
            std::string sql = "INSERT INTO user (username, password, salt, isAdmin) VALUES ('" + u->name + "', '" + u->password + "', '" + u->salt + "', " + std::to_string(u->isAdmin) + ")";
            return UpdateMysql(sql);
        }
        bool QueryUser(const std::string &username, User *u)
        {
            std::string sql = "SELECT * FROM " + oj_users + " WHERE username = '" + username + "'";
            return QueryMysql(sql, u);
        }
        bool QueryUserById(const std::string &userId, User *u)
        {
            std::string sql = "SELECT * FROM " + oj_users + " WHERE userid = '" + userId + "'";
            return QueryMysql(sql, u);
        }
        bool UpdateUser(const User *u)
        {
            std::string sql = "UPDATE user SET password='" + u->password + "', salt='" + u->salt + "' WHERE username='" + u->name + "'";
            return UpdateMysql(sql);
        }
        bool UpdateUserById(const User *u)
        {
            // 更新用户名，密码，盐，是否为管理员
            std::string sql = "UPDATE user SET username='" + u->name + "', password='" + u->password + "', salt='" + u->salt + "', isAdmin=" + std::to_string(u->isAdmin) + " WHERE userid='" + std::to_string(u->id) + "'";
            return UpdateMysql(sql);
        }
        bool DeleteUser(const std::string &username)
        {
            std::string sql = "DELETE FROM user WHERE username = '" + username + "'";
            return UpdateMysql(sql);
        }
        bool DeleteUserById(const std::string &userId)
        {
            std::string sql = "DELETE FROM user WHERE userid = '" + userId + "'";
            return UpdateMysql(sql);
        }

        bool GetAllUsers(vector<User> *out)
        {
            // 将CreationTime创建晚的放在前面
            std::string sql = "SELECT * FROM user ORDER BY CreationTime DESC";
            return QueryMysql2(sql, out);
        }
    private:
        bool QueryMysql2(const std::string &sql, vector<User> *out)
        {
            try
            {
                MySQLConnection db;
                if (0 != mysql_query(db.get(), sql.c_str()))
                {
                    std::cerr << sql << " 执行错误：" << mysql_error(db.get()) << "\n";
                    return false;
                }
                MYSQL_RES *res = mysql_store_result(db.get());
                if (res == nullptr)
                {
                    std::cerr << "获取查询结果失败：" << mysql_error(db.get()) << "\n";
                    return false;
                }

                int rows = mysql_num_rows(res);
                for (int i = 0; i < rows; ++i)
                {
                    MYSQL_ROW row = mysql_fetch_row(res);
                    User u;
                    u.id = atoi(row[0]);
                    u.name = row[1];
                    u.password = row[2];
                    u.salt = row[3];
                    u.isAdmin = atoi(row[4]);
                    out->push_back(u);
                }
                mysql_free_result(res);
                return true;
            }
            catch (const std::runtime_error &e)
            {
                std::cerr << e.what() << '\n';
                return false;
            }
        }
        bool QueryMysql(const std::string &sql, User *out)
        {
            try
            {
                MySQLConnection db;
                if (0 != mysql_query(db.get(), sql.c_str()))
                {
                    std::cerr << sql << " 执行错误：" << mysql_error(db.get()) << "\n";
                    return false;
                }

                MYSQL_RES *res = mysql_store_result(db.get());
                if (res == nullptr)
                {
                    std::cerr << "获取查询结果失败：" << mysql_error(db.get()) << "\n";
                    return false;
                }

                int rows = mysql_num_rows(res);
                if (rows > 0)
                {
                    MYSQL_ROW row = mysql_fetch_row(res);
                    out->id = atoi(row[0]);
                    out->name = row[1];
                    out->password = row[2];
                    out->salt = row[3];
                    out->isAdmin = atoi(row[4]);
                }
                mysql_free_result(res);
                return true;
            }
            catch (const std::runtime_error &e)
            {
                std::cerr << e.what() << '\n';
                return false;
            }
        }

        bool UpdateMysql(const std::string &sql)
        {
            try
            {
                MySQLConnection db;
                if (0 != mysql_query(db.get(), sql.c_str()))
                {
                    std::cerr << sql << " 执行错误：" << mysql_error(db.get()) << "\n";
                    return false;
                }
                return true;
            }
            catch (const std::runtime_error &e)
            {
                std::cerr << e.what() << '\n';
                return false;
            }
        }
    };
    class UserProgressModel
    {
        private:
            const std::string oj_user_progress = "UserProgress";

        public:
            UserProgressModel() {}
            ~UserProgressModel() {}

            bool AddUserProgress(const UserProgress *up)
            {
                std::string sql = "INSERT INTO UserProgress (user_id, question_id, is_completed, completed_time, attempt_count) VALUES (" + std::to_string(up->userId) + ", " + std::to_string(up->questionId) + ", " + std::to_string(up->isCompleted) + ", " + StringUtil::timeToString(up->completedTime) + ", " + std::to_string(up->attemptCount) + ")";
                return UpdateMysql(sql);
            }

            bool QueryUserProgress(const int userId, const int questionId, UserProgress * &up)
            {
                std::string sql = "SELECT * FROM " + oj_user_progress + " WHERE user_id = " + std::to_string(userId) + " AND question_id = " + std::to_string(questionId);
                return QueryMysql(sql, up);
            }

            bool UpdateUserProgress(const UserProgress *up)
            {
                std::string sql = "UPDATE UserProgress SET is_completed = " + std::to_string(up->isCompleted) + ", completed_time = " + StringUtil::timeToString(up->completedTime) + ", attempt_count=" + std::to_string(up->attemptCount) + " WHERE user_id=" + std::to_string(up->userId) + " AND question_id=" + std::to_string(up->questionId);
                return UpdateMysql(sql);
            }

            bool DeleteUserProgress(const int userId, const int questionId)
            {
                std::string sql = "DELETE FROM " + oj_user_progress + " WHERE user_id = " + std::to_string(userId) + " AND question_id = " + std::to_string(questionId);
                return UpdateMysql(sql);
            }
            bool QueryUserProgressByUserId(const string & userId,vector<UserProgress>* out)
            {
                std::string sql = "SELECT * FROM " + oj_user_progress + " WHERE user_id = " + userId +" and is_completed = 1"+ " ORDER BY completed_time DESC";
                return QueryMysql2(sql, out);
            }
        private:
            bool QueryMysql2(const std::string &sql, vector<UserProgress> *out)
            {
                try
                {
                    MySQLConnection db;
                    if (0 != mysql_query(db.get(), sql.c_str()))
                    {
                        std::cerr << sql << " 执行错误：" << mysql_error(db.get()) << "\n";
                        return false;
                    }
                    MYSQL_RES *res = mysql_store_result(db.get());
                    if (res == nullptr)
                    {
                        std::cerr << "获取查询结果失败：" << mysql_error(db.get()) << "\n";
                        return false;
                    }

                    int rows = mysql_num_rows(res);
                    for (int i = 0; i < rows; ++i)
                    {
                        MYSQL_ROW row = mysql_fetch_row(res);
                        UserProgress up;
                        up.progressId = atoi(row[0]);
                        up.userId = atoi(row[1]);
                        up.questionId = atoi(row[2]);
                        up.isCompleted = atoi(row[3]);
                        up.completedTime = StringUtil::convertStringToTimeT(row[4]);
                        up.attemptCount = atoi(row[5]);
                        out->push_back(up);
                    }
                    mysql_free_result(res);
                    return true;
                }
                catch (const std::runtime_error &e)
                {
                    std::cerr << e.what() << '\n';
                    return false;
                }
            }
            bool QueryMysql(const std::string &sql, UserProgress *out)
            {
                try
                {
                    MySQLConnection db;
                    if (0 != mysql_query(db.get(), sql.c_str()))
                    {
                        std::cerr << sql << " 执行错误：" << mysql_error(db.get()) << "\n";
                        return false;
                    }

                    MYSQL_RES *res = mysql_store_result(db.get());
                    if (res == nullptr)
                    {
                        std::cerr << "获取查询结果失败：" << mysql_error(db.get()) << "\n";
                        return false;
                    }

                    int rows = mysql_num_rows(res);
                    if (rows > 0)
                    {
                        MYSQL_ROW row = mysql_fetch_row(res);
                        out->progressId = atoi(row[0]);
                        out->userId = atoi(row[1]);
                        out->questionId = atoi(row[2]);
                        out->isCompleted = atoi(row[3]);
                        out->completedTime = StringUtil::convertStringToTimeT(row[4]);
                        out->attemptCount = atoi(row[5]);
                    }
                    mysql_free_result(res);
                    return true;
                }
                catch (const std::runtime_error &e)
                {
                    std::cerr << e.what() << '\n';
                    return false;
                }
            }

            bool UpdateMysql(const std::string &sql)
            {
                try
                {
                    MySQLConnection db;
                    if (0 != mysql_query(db.get(), sql.c_str()))
                    {
                        std::cerr << sql << " 执行错误：" << mysql_error(db.get()) << "\n";
                        return false;
                    }
                    return true;
                }
                catch (const std::runtime_error &e)
                {
                    std::cerr << e.what() << '\n';
                    return false;
                }
            }
    };
}
