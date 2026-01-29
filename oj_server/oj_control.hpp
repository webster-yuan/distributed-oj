#pragma once
#include <iostream>
#include <string>
#include <signal.h>
// #include "oj_model.hpp"
#include "oj_model2.hpp"
#include "../comm/log.hpp"
#include "../comm/util.hpp"
#include <mutex>
#include <algorithm>
#include "oj_view.hpp"
#include <fstream>
#include <vector>
#include "../comm/httplib.h"
#include <cassert>
#include <jsoncpp/json/json.h>
#include <memory>
// #include<json/json.h>
namespace ns_control
{
    using namespace ns_util;
    using namespace ns_log;
    using namespace ns_view;
    using namespace ns_model;
    using namespace httplib;

    // 提供服务的主机
    class Machine
    {
    public:
        std::string ip;  // 编译服务的ip
        int port;        // 编译服务的端口
        uint64_t load;   // 编译服务的负载，需要锁进行保护，可能同时来很多主机访问服务
        std::mutex *mtx; // 禁止拷贝，所以用拷贝指针的方式调用这把锁
    public:
        Machine() : ip(""), port(0), load(0), mtx(nullptr)
        {
        }
        ~Machine() {}

    public:
        // 提升主机负载
        void IncLoad()
        {
            if (mtx)
                mtx->lock();
            load++;
            if (mtx)
                mtx->unlock();
        }
        // 减少主机负载
        void DecLoad()
        {
            if (mtx)
                mtx->lock();
            load--;
            if (mtx)
                mtx->unlock();
        }
        // 获取主机负载
        uint64_t Load()
        {
            uint64_t _load = 0;
            if (mtx)
                mtx->lock();
            _load = load;
            if (mtx)
                mtx->unlock();

            return _load;
        }
        void ResetLoad()
        {
            if (mtx)
                mtx->lock();
            load = 0;
            if (mtx)
                mtx->unlock();
        }
        // 获取主机的实际状态信息
        bool UpdateStatus()
        {
            httplib::Client cli(ip, port);
            auto res = cli.Get("/status");

            if (res && res->status == 200)
            {
                Json::CharReaderBuilder reader;
                Json::Value status;
                std::string errs;
                std::istringstream s(res->body);
                if (Json::parseFromStream(reader, s, &status, &errs))
                {
                    // 获取各个属性的值
                    uint64_t cpu_usage = status["cpu_usage"].asUInt64();
                    uint64_t memory_usage = status["memory_usage"].asUInt64();
                    uint64_t network_usage = status["network_usage"].asUInt64();
                    uint64_t disk_io_usage = status["disk_io_usage"].asUInt64();
                    uint64_t active_connections = status["active_connections"].asUInt64();
                    double load_avg_1min = status["load_average"]["1min"].asDouble();
                    double load_avg_5min = status["load_average"]["5min"].asDouble();
                    double load_avg_15min = status["load_average"]["15min"].asDouble();
                    float response_time = status["response_time"].asFloat();

                    // 设置权重
                    double cpu_weight = 0.4;
                    double memory_weight = 0.2;
                    double network_weight = 0.1;
                    double disk_io_weight = 0.1;
                    double connections_weight = 0.05;
                    double load_avg_weight = 0.1;
                    double response_time_weight = 0.05;

                    // 计算综合负载值
                    double total_load = cpu_usage * cpu_weight +
                                        memory_usage * memory_weight +
                                        network_usage * network_weight +
                                        disk_io_usage * disk_io_weight +
                                        active_connections * connections_weight +
                                        (load_avg_1min + load_avg_5min + load_avg_15min) * load_avg_weight / 3 +
                                        response_time * response_time_weight;

                    if (mtx)
                        mtx->lock();
                    load = static_cast<uint64_t>(total_load);
                    if (mtx)
                        mtx->unlock();

                    // 打印这些信息或用于其他用途
                    std::cout << "Machine " << ip << ":" << port << " status:" << std::endl;
                    std::cout << "CPU Usage: " << cpu_usage << "%" << std::endl;
                    std::cout << "Memory Usage: " << memory_usage << "%" << std::endl;
                    std::cout << "Network Usage: " << network_usage << " KB/s" << std::endl;
                    std::cout << "Disk I/O Usage: " << disk_io_usage << " KB/s" << std::endl;
                    std::cout << "Active Connections: " << active_connections << std::endl;
                    std::cout << "Load Average (1min): " << load_avg_1min << std::endl;
                    std::cout << "Load Average (5min): " << load_avg_5min << std::endl;
                    std::cout << "Load Average (15min): " << load_avg_15min << std::endl;
                    std::cout << "Response Time: " << response_time << " ms" << std::endl;

                    return true;
                }
            }
            return false;
        }
    };

    const std::string service_machine = "./conf/service_machine.conf";

    // 负载均衡板块
    class LoadBalance
    {
    private:
        std::vector<Machine> machines; // 可以给我们提供编译服务的主机，下标对应主机编�?
        // 所有在线的主机id
        std::vector<int> online;
        // 所有离线的主机id
        std::vector<int> offline;
        // 保证数据安全
        std::mutex mtx;

    public:
        LoadBalance()
        {
            assert(LoadConf(service_machine));
            LOG(INFFO) << "加载 " << service_machine << "成功"
                       << "\n";
        }
        ~LoadBalance() {}

    public:
        bool LoadConf(const std::string &machine_conf)
        {
            std::ifstream in(machine_conf);
            if (!in.is_open())
            {
                LOG(FATAL) << "加载" << machine_conf << "失败"
                           << "\n";
                return false;
            }
            std::string line;
            while (std::getline(in, line))
            {
                // :作为分隔符切割为两部分，ip+port
                std::vector<std::string> tokens;
                StringUtil::SplitString(line, &tokens, ":");
                if (tokens.size() != 2)
                {
                    LOG(WARNING) << "切分" << line << "失败"
                                 << "\n";
                    continue;
                }
                Machine m;
                m.ip = tokens[0];
                m.port = atoi(tokens[1].c_str());
                m.load = 0;
                m.mtx = new std::mutex();

                online.push_back(machines.size()); // 先插入在线主机，0
                machines.push_back(m);             // size=1
            }
            in.close();
            return true;
        }
        // 输出型参数，根据主机地址直接操控主机
        bool SmartChoice(int *id, Machine **m) // 解引用是Machine* ，让外面通过地址得到主机
        {
            // 1. 使用选择好的主机，更新主机负载
            // 2. 我们需要可能离线该主机，online->offline
            mtx.lock();
            // 负载均衡算法
            int online_num = online.size();
            if (online_num == 0)
            {
                mtx.unlock();
                LOG(FATAL) << "所有的后端编译器已经全部离线"
                           << "\n";
                return false;
            }
            // 更新所有主机的负载
            for (int i = 0; i < online_num; i++)
            {
                machines[online[i]].UpdateStatus();
            }

            // 遍历找到所有的负载最小的机器
            uint64_t min_load = machines[online[0]].Load();
            *id = online[0];
            *m = &machines[online[0]];
            for (int i = 0; i < online_num; i++)
            {
                uint64_t cur_load = machines[online[i]].Load();
                if (min_load > cur_load)
                {
                    min_load = cur_load;
                    *id = online[i];
                    *m = &machines[online[i]];
                }
            }

            mtx.unlock();
            return true;
        }
        void OfflineMachine(int which)
        {
            mtx.lock();
            for (auto iter = online.begin(); iter != online.end(); iter++)
            {
                if (*iter == which)
                {
                    // 将负载清零之后，将该主机从online中删除，加入offline中
                    machines[which].ResetLoad();
                    online.erase(iter);
                    offline.push_back(which); // 删除之后指向下一个位置，发生迭代器失效。所以用which不是*iter
                    break;                    // 不用考虑迭代器失效的问题
                }
            }
            mtx.unlock();
        }
        void OnlineMachine()
        {
            // 统一上线的所有服务器,把offline的内容都移到online数组
            mtx.lock();
            online.insert(online.end(), offline.begin(), offline.end());
            offline.erase(offline.begin(), offline.end());
            mtx.unlock();
            LOG(INFO) << "所有的主机上线成功"
                      << "\n";
        }
        // fortest
        void ShowMachines()
        {
            mtx.lock();
            std::cout << "当前在线主机列表: ";
            for (auto &id : online)
            {
                std::cout << id << " ";
            }
            std::cout << std::endl;
            std::cout << "当前离线主机列表: ";
            for (auto &id : offline)
            {
                std::cout << id << " ";
            }
            std::cout << std::endl;
            mtx.unlock();
        }
    };

    // 核心执行业务控制模块
    class Control
    {
    private:
        Model model_;
        View view_;                // 提供网页渲染功能�?
        LoadBalance load_balance_; // 核心负载均衡�?
    public:
        Control() {}
        ~Control() {}

    public:
        void RecoveryMachine()
        {
            load_balance_.OnlineMachine();
        }
        // 实现新增题目
        bool AddQuestion(const struct Question &q)
        {
            std::cout << "AddQuestion" << endl;
            bool ret = true;
            if (model_.InsertQuestion(q))
            {
                LOG(INFO) << "新增题目成功"
                          << "\n";
            }
            else
            {
                ret = false;
                LOG(ERROR) << "新增题目失败"
                           << "\n";
            }
            return ret;
        }
        // 实现删除题目
        bool DeleteQuestion(const std::string &number)
        {
            bool ret = true;
            if (model_.DeleteQuestion(number))
            {
                LOG(INFO) << "删除题目成功"
                          << "\n";
            }
            else
            {
                ret = false;
                LOG(ERROR) << "删除题目失败"
                           << "\n";
            }
            return ret;
        }
        // 实现修改题目
        bool ModifyQuestion(const struct Question &q)
        {
            bool ret = true;
            if (model_.UpdateQuestion(q))
            {
                LOG(INFO) << "修改题目成功"
                          << "\n";
            }
            else
            {
                ret = false;
                LOG(ERROR) << "修改题目失败"
                           << "\n";
            }
            return ret;
        }
        // 根据获取到的题目数据构建网页，输出型参数
        bool AllQuestions(std::string *html)
        {
            bool ret = true;
            vector<struct Question> all;
            if (model_.GetAllQuestions(&all))
            {
                sort(all.begin(), all.end(), [](const struct Question &q1, const struct Question &q2) { // 是字符串，不想要用ASCII码表进行排序
                    return atoi(q1.number.c_str()) < atoi(q2.number.c_str());
                });
                // 获取题目信息成功到达all中，将所有的题目数据构建成网页?
                view_.ExpandAllHtml(all, html);
            }
            else
            {
                *html = "获取题目失败";
                ret = false;
            }
            return ret;
        }

        bool Question(const string &number, string *html)
        {
            bool ret = true;
            struct Question q;
            if (model_.GetOneQuestion(number, &q))
            {
                // 获取指定题目信息成功，将所有的题目数据构建成网页
                view_.OneExpandHtml(q, html);
            }
            else
            {
                *html = "指定题目: " + number + " 不存在!";
                ret = false;
            }
            return ret;
        }

        // 实现提交代码接口
        void Judge(const std::string &number, const std::string in_json, std::string *out_json)
        {

            // 用户提交上来的json_string中字段，id: code: input:
            //  根据题目编号直接拿到题目细节
            struct Question q;
            model_.GetOneQuestion(number, &q);
            // 1. in_json进行反序列化，得到题目id，得到用户提交的源代码?
            Json::Reader reader;
            Json::Value in_value;
            reader.parse(in_json, in_value);

            std::string code = in_value["code"].asString();
            // 2. 重新拼接用户代码+测试用例代码形成完整代码
            Json::Value compile_value;
            compile_value["input"] = in_value["input"].asString();
            compile_value["code"] = code + "\n" + q.tail; // 测试用例添加�?
            compile_value["cpu_limit"] = q.cpu_limit;
            compile_value["mem_limit"] = q.mem_limit;
            Json::FastWriter writer;
            std::string compile_string = writer.write(compile_value);
            // 3. 选择负载最低的主机 一直选择，直到主机可用，否则就是全挂
            while (true)
            {
                int id = 0;
                Machine *m = nullptr;
                if (!load_balance_.SmartChoice(&id, &m))
                {
                    break;
                }
                // 4. 发起http请求得到结果
                Client cli(m->ip, m->port);
                m->IncLoad();
                LOG(INFO) << " 选择主机成功, 主机id: " << id << " 详情: " << m->ip << ":" << m->port << " 当前主机的负载是: " << m->Load() << "\n";
                if (auto res = cli.Post("/compile_and_run", compile_string, "application/json;charset=utf-8"))
                {
                    // 5. 将结果赋值给out_json
                    if (res->status == 200)
                    {
                        *out_json = res->body;
                        m->DecLoad();
                        LOG(INFO) << "请求服务成功"
                                  << "\n";
                        break;
                    }
                    // 错误码失败，接着选别的主机
                    m->DecLoad();
                }
                else
                {
                    // 请求失败
                    LOG(ERROR) << " 当前请求的主机id: " << id << " 详情: " << m->ip << ":" << m->port << " 可能已经离线"
                               << "\n";
                    load_balance_.OfflineMachine(id);
                    load_balance_.ShowMachines(); // for test
                }
            }
        }
    };

    class UserControl
    {
    private:
        UserModel userModel;
        View view;
        UserProgressModel upModel;

    public:
        bool EditUser(string &number, string &username, string &password, string &isAdmin)
        {
            bool ret = true;
            // 生成随机盐
            string salt = UUIDUtil::generate_hex(10);
            // 加密密码
            string real_password = MD5Util::generateSaltedMD5(password, salt);
            User user;
            user.id = atoi(number.c_str());
            user.name = username;
            user.salt = salt;
            user.password = real_password;
            user.isAdmin = (isAdmin == "true");
            if (userModel.UpdateUserById(&user))
            {
                LOG(INFO) << "修改用户成功" << endl;
            }
            else
            {
                ret = false;
                LOG(ERROR) << "修改用户失败" << endl;
            }
            return ret;
        }
        bool GetTheUser(const string &number, string *html)
        {
            bool ret = true;
            User newUser;
            if (userModel.QueryUserById(number, &newUser))
            {
                vector<UserProgress> userProgressAll;
                // 获取指定题目信息成功，将所有的题目数据构建成网页
                if (upModel.QueryUserProgressByUserId(to_string(newUser.id), &userProgressAll))
                {
                    view.GenerateUserDetailPage(newUser, userProgressAll, html);
                }
                else
                {
                    *html = "指定用户: " + number + " 不存在!";
                    ret = false;
                }
            }
            return ret;
        }
        bool AddUser(User *user)
        {
            bool ret = true;
            // userModel层,让他和数据库交互
            if (userModel.InsertUserByAdmin(user))
            {
                return ret;
            }
            else
            {
                ret = false;
                return ret;
            }
        }
        bool RegisterUser(User *user)
        {
            bool ret = true;
            // userModel层,让他和数据库交互
            if (userModel.AddUser(user))
            {
                return ret;
            }
            else
            {
                ret = false;
                return ret;
            }
        }
        bool LoginUser(const std::string &name, const std::string &password)
        {
            bool ret = true;
            User *user = new User();
            bool isExisted = userModel.QueryUser(name, user);
            string salt = user->salt;
            string real_password = MD5Util::generateSaltedMD5(password, salt);
            // userModel层,让他和数据库交互
            if (isExisted == true && user->password == real_password)
            {
                LOG(INFO) << "密码正确" << endl;
                return ret;
            }
            else
            {
                LOG(ERROR) << "密码错误" << endl;
                ret = false;
                return ret;
            }
        }
        bool GetUserInfo(const std::string &username, User *&user)
        {
            bool ret = true;
            bool isExisted = userModel.QueryUser(username, user);
            if (isExisted == true)
            {
                return ret;
            }
            else
            {
                ret = false;
                return ret;
            }
        }
        bool IsAdmin(const std::string &username)
        {
            bool ret = true;
            User *user = new User();
            if (GetUserInfo(username, user))
            {
                if (user->isAdmin)
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }
            else
            {
                LOG(ERROR) << "查询用户失败" << endl;
                return false;
            }
        }
        bool DeleteUser(const std::string &userId)
        {
            bool ret = true;
            if (userModel.DeleteUserById(userId))
            {
                LOG(INFO) << "删除用户成功" << userId << endl;
                return ret;
            }
            else
            {
                ret = false;
                LOG(INFO) << "删除用户成功" << userId << endl;
                return ret;
            }
        }

    public:
        bool AllUsers(std::string *html)
        {
            bool ret = true;
            vector<User> all;
            if (userModel.GetAllUsers(&all))
            {
                view.UserAllHtml(all, html);
            }
            else
            {
                *html = "获取用户失败";
                ret = false;
            }
            return ret;
        }
    };
    class UserProgressControl
    {
    private:
        UserProgressModel userProgressModel;
        View view;

    public:
        // 封装查询逻辑以复用
        bool queryProgress(int userId, int questionId, UserProgress *&userProgress)
        {
            // cout<<"查询前 userProgress:"<<userProgress->userId<<" "<<userProgress->questionId<<" "<<userProgress->attemptCount<<endl;

            if (userProgressModel.QueryUserProgress(userId, questionId, userProgress))
            {
                // cout<<"查询后 userProgress:"<<userProgress->userId<<" "<<userProgress->questionId<<" "<<userProgress->attemptCount<<endl;
                if (userProgress->progressId != 0)
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }
            else
            {
                LOG(ERROR) << "查询用户答题进度失败" << std::endl;
                return false;
            }
        }
        bool GetTheUserProgress(const string &userId, string *html)
        {
            bool ret = true;
            vector<UserProgress> all;
            if (userProgressModel.QueryUserProgressByUserId(userId, &all))
            {
                view.AllUserProgress(all, html);
            }
            else
            {
                *html = "获取用户答题进度失败";
                ret = false;
            }
            return ret;
        }

    public:
        bool setCompleted(UserProgress *&userProgress)
        {
            if (queryProgress(userProgress->userId, userProgress->questionId, userProgress))
            {
                userProgress->isCompleted = true;
                userProgress->completedTime = std::time(nullptr);
                userProgress->attemptCount++;
                if (userProgressModel.UpdateUserProgress(userProgress))
                {
                    cout << "执行更新后 userProgress:" << userProgress->userId << " " << userProgress->questionId << " " << userProgress->attemptCount << endl;
                    return true;
                }
                else
                {
                    return false;
                }
            }
            else
            {
                if (userProgressModel.AddUserProgress(userProgress))
                {
                    cout << "执行新增后 userProgress:" << userProgress->userId << " " << userProgress->questionId << " " << userProgress->attemptCount << endl;
                    return true;
                }
                else
                {
                    return false;
                }
            }
        }
        bool AddOrUpdateUserProgress(UserProgress *&userProgress)
        {
            if (queryProgress(userProgress->userId, userProgress->questionId, userProgress))
            {
                userProgress->attemptCount++;
                if (userProgressModel.UpdateUserProgress(userProgress))
                {
                    cout << "执行更新后 userProgress:" << userProgress->userId << " " << userProgress->questionId << " " << userProgress->attemptCount << endl;
                    return true;
                }
                else
                {
                    return false;
                }
            }
            else
            {
                if (userProgressModel.AddUserProgress(userProgress))
                {
                    cout << "执行新增后 userProgress:" << userProgress->userId << " " << userProgress->questionId << " " << userProgress->attemptCount << endl;
                    return true;
                }
                else
                {
                    return false;
                }
            }
        }
    };

}