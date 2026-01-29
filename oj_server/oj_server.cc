#include <iostream>
#include <signal.h>
#include "../comm/httplib.h"
#include "oj_control.hpp"
#include <memory>
using namespace ns_control;
using namespace httplib;

static Control *ctrl_ptr = nullptr;
static UserControl *ctrl_ptr2 = nullptr;
static UserProgressControl *ctrl_ptr3 = nullptr;
void InitializeControllers()
{
    static Control ctrl;
    ctrl_ptr = &ctrl;

    static UserControl ctrl2;
    ctrl_ptr2 = &ctrl2;

    static UserProgressControl ctrl3;
    ctrl_ptr3 = &ctrl3;
}

void Recovery(int signo)
{
    if (ctrl_ptr)
    {
        ctrl_ptr->RecoveryMachine();
    }
}

std::map<std::string, std::string> sessions;
int main()
{
    signal(SIGQUIT, Recovery);
    InitializeControllers();

    Server svr;
    // 用户修改信息功能
    svr.Post("/editUser", [](const httplib::Request &req, httplib::Response &resp)
             {
        // 解析请求体中的 JSON 数据
    Json::Value json_data;
    Json::Reader reader;
    if (!reader.parse(req.body, json_data)) {
        Json::Value json_error;
        json_error["success"] = false;
        json_error["error"] = "Invalid JSON data";
        resp.set_content(json_error.toStyledString(), "application/json");
        return;
    }

    std::string number = json_data.get("number", "").asString();
    std::string username = json_data.get("username", "").asString();
    std::string password = json_data.get("password", "").asString();
    std::string isAdmin = json_data.get("role", "").asString();

    // 从请求头中获取 session_id，验证是否是管理员
    std::string session_id = SessionUtil::get_cookie_value(req, "session_id");
    auto it = sessions.find(session_id);
    if (it != sessions.end()) {
        std::string username2 = it->second;
        if (ctrl_ptr2->IsAdmin(username2)) {
            bool success = ctrl_ptr2->EditUser(number, username, password, isAdmin);
            Json::Value json_response;
            json_response["success"] = success;
            resp.set_content(json_response.toStyledString(), "application/json"); 
        } else {
            Json::Value json_error;
            json_error["success"] = false;
            json_error["error"] = "You are not admin";
            resp.set_content(json_error.toStyledString(), "application/json");
        }
    } else {
        Json::Value json_error;
        json_error["success"] = false;
        json_error["error"] = "Invalid session";
        resp.set_content(json_error.toStyledString(), "application/json");
    } });
    // 删除指定用户功能
    svr.Delete(R"(/deleteUser/(\d+))", [](const httplib::Request &req, httplib::Response &resp)
               {
                Json::Value json_data;
                // 获取传来的要删除的用户id
                std::string user_id = req.matches[1];
                // 从req请求头中获取session_id,验证是否是管理员
                std::string session_id = SessionUtil::get_cookie_value(req, "session_id");
                auto it = sessions.find(session_id);
                if (it != sessions.end()) {
                    std::string username = it->second;
                    if (ctrl_ptr2->IsAdmin(username)) {
                        bool success = ctrl_ptr2->DeleteUser(user_id);
                        json_data["success"] = success;
                       
                    } else {
                        json_data["success"] = false;
                        json_data["error"]="You are not admin";
                    }
                } else {
                        json_data["success"] = false;
                        json_data["error"]="Invalid session";
                }
                 std::string json_str = json_data.toStyledString();
                 resp.set_content(json_str, "application/json"); });
    // 新增用户功能
    svr.Post("/addUser", [](const httplib::Request &req, httplib::Response &resp)
             {
    Json::Value json_data;
    Json::Reader reader;
    Json::Value req_data;

    // 解析 JSON 数据
    if (reader.parse(req.body, req_data)) {
        std::string username = req_data.get("username", "").asString();
        std::string password = req_data.get("password", "").asString();
        std::string isAdmin = req_data.get("role", "").asString();

        cout << "username: " << username << " password: " << password << " isAdmin: " << isAdmin << endl;

        // 从req请求头中获取session_id,验证是否是管理员
        std::string session_id = SessionUtil::get_cookie_value(req, "session_id");
        auto it = sessions.find(session_id);
        if (it != sessions.end()) {
            std::string username2 = it->second;
            if (ctrl_ptr2->IsAdmin(username2)) {
                User* newUser = new User();
                // 对密码进行加密
                string salt = UUIDUtil::generate_hex(10);
                string encryptedPassword = MD5Util::generateSaltedMD5(password, salt);
                newUser->name = username;
                newUser->password = encryptedPassword;
                newUser->salt = salt;
                newUser->isAdmin = isAdmin == "true";
                bool success = ctrl_ptr2->AddUser(newUser);

                json_data["success"] = success;

            } else {
                json_data["success"] = false;
                json_data["error"] = "You are not admin";
            }
        } else {
            json_data["success"] = false;
            json_data["error"] = "Invalid session";
        }
    } else {
        json_data["success"] = false;
        json_data["error"] = "Invalid JSON format";
    }

    std::string json_str = json_data.toStyledString();
    resp.set_content(json_str, "application/json"); });

    // 获取用户信息功能
    svr.Get("/getUserInfo", [](const httplib::Request &req, httplib::Response &resp)
            {
            // 从req请求头中获取session_id
            std::string session_id = SessionUtil::get_cookie_value(req, "session_id");
            // 从sessions中查找session_id对应的用户名
            auto it = sessions.find(session_id);
            if (it != sessions.end()) {
                std::string username = it->second;
                User* user = new User();
                if (ctrl_ptr2->GetUserInfo(username,user)) {
                    Json::Value json_data;
                    json_data["success"] = true;
                    json_data["username"] = user->name;
                    json_data["isAdmin"]=user->isAdmin;
                    std::string json_str = json_data.toStyledString();
                    resp.set_content(json_str, "application/json");
                } else {
                    resp.set_content("User not found", "text/plain;charset=utf-8");
                }
                delete user;
            } else {
                resp.set_content("Invalid session", "text/plain;charset=utf-8");
            } });

    svr.Post("/register", [](const httplib::Request &req, httplib::Response &resp)
             {
            // 获取注册信息
            std::string username = req.has_param("username") ? req.get_param_value("username") : req.get_file_value("username").content;
            std::string password = req.has_param("password") ? req.get_param_value("password") : req.get_file_value("password").content;

            std::string salt = UUIDUtil::generate_hex(10);//生成随机盐
            std::string encryptedPassword = MD5Util::generateSaltedMD5(password, salt);//加密密码
            User* newUser = new User(username, encryptedPassword, salt);

            bool success = ctrl_ptr2->RegisterUser(newUser);// 调用model模块和数据库链接，注册用户
            LOG(INFO)<<"注册成功"<<endl;
            Json::Value json_data;
            json_data["success"] = success; 

            delete newUser;
            // 注册成功，返回json数据
            std::string json_str = json_data.toStyledString();
            resp.set_content(json_str, "application/json"); });

    svr.Post("/login", [](const httplib::Request &req, httplib::Response &resp)
             {
            // 获取登录信息
            std::string username = req.has_param("username") ? req.get_param_value("username") : req.get_file_value("username").content;
            std::string password = req.has_param("password") ? req.get_param_value("password") : req.get_file_value("password").content;

            if (ctrl_ptr2->LoginUser(username, password)) {// 调用model模块和数据库链接，登录用户
                std::string session_id = SessionUtil::generate_alpha_numeric_session_id(32);//生成session_id
                sessions[session_id] = username;//将session_id和用户名存入sessions

                resp.set_header("Set-Cookie", "session_id=" + session_id);//设置cookie
                // 登录成功，返回json数据
                Json::Value json_data;
                json_data["success"] = "success";
                json_data["session_id"] = session_id;

                std::string json_str = json_data.toStyledString();
                resp.set_content(json_str, "application/json");
                } else {
                    resp.set_content("Login Failed", "text/plain;charset=utf-8");
                } });

    svr.Get("/check_login", [](const httplib::Request &req, httplib::Response &resp)
            {
            std::string session_id = SessionUtil::get_cookie_value(req, "session_id");
            auto it = sessions.find(session_id);

            Json::Value json_data;
            if (it != sessions.end()) {
                json_data["success"] = true;
                json_data["username"] = it->second;
            } else {
                json_data["success"] = false;
            }

            std::string json_str = json_data.toStyledString();
            resp.set_content(json_str, "application/json"); });
    // 实现注销功能
    svr.Post("/logout", [](const httplib::Request &req, httplib::Response &resp)
             {
            std::string session_id = SessionUtil::get_cookie_value(req, "session_id");
            auto it = sessions.find(session_id);

            Json::Value json_data;
            if (it != sessions.end()) {
                // 打印注销成功
                LOG(INFO) << "User " << it->second << " logout success."<<endl;
                sessions.erase(it);
                json_data["success"] = true;
            } else {
                json_data["success"] = false;
            }
            std::string json_str = json_data.toStyledString();
            resp.set_content(json_str, "application/json"); });

    svr.Post("/updateQuestion", [](const httplib::Request &req, httplib::Response &resp)
             {
        std::string number = req.has_param("number") ? req.get_param_value("number") : "";
        std::string title = req.has_param("title") ? req.get_param_value("title") : "";
        std::string star = req.has_param("star") ? req.get_param_value("star") : "";
        std::string cpu_limit = req.has_param("cpu_limit") ? req.get_param_value("cpu_limit") : "";
        std::string mem_limit = req.has_param("mem_limit") ? req.get_param_value("mem_limit") : "";
        std::string desc = req.has_param("desc") ? req.get_param_value("desc") : "";
        std::string header = req.has_param("header") ? req.get_param_value("header") : "";
        std::string tail = req.has_param("tail") ? req.get_param_value("tail") : "";

        Json::Value json_data;

        try {
            int cpu_limit_int = std::stoi(cpu_limit);
            int mem_limit_int = std::stoi(mem_limit);

            Question question(number, title, star, cpu_limit_int, mem_limit_int, desc, header, tail);
            bool success = ctrl_ptr->ModifyQuestion(question);
            json_data["success"] = success;
        } catch (const std::exception &e) {
            json_data["success"] = false;
            json_data["error"] = e.what();
        }

        std::string json_str = json_data.toStyledString();
        resp.set_content(json_str, "application/json"); });

    svr.Post("/addQuestion", [](const httplib::Request &req, httplib::Response &resp)
             {
        std::cout << "Request body: " << req.body << std::endl;
        
        // 从请求头中获取session_id
        std::string session_id = SessionUtil::get_cookie_value(req, "session_id");
        // 从sessions中查找session_id对应的用户名
        auto it = sessions.find(session_id);
        std::string username = it->second;//得到创作者用户名

        // 打印请求体以调试
        Json::CharReaderBuilder readerBuilder;
        Json::Value jsonData;
        std::string errs;

        std::istringstream s(req.body);
        if (!Json::parseFromStream(readerBuilder, s, &jsonData, &errs)) {// 解析json数据
            Json::Value json_data;
            json_data["success"] = false;
            json_data["error"] = "Invalid JSON data";
            std::string json_str = json_data.toStyledString();
            resp.set_content(json_str, "application/json");
            return;
        }
        // 
        std::string title = jsonData.get("title", "").asString();
        std::string star = jsonData.get("star", "").asString();
        std::string question_desc = jsonData.get("question_desc", "").asString();
        std::string header = jsonData.get("header", "").asString();
        std::string tail = jsonData.get("tail", "").asString();
        std::string time_limit = jsonData.get("time_limit", "").asString();
        std::string mem_limit = jsonData.get("mem_limit", "").asString();

        Json::Value json_data;

        try {
            if (!StringUtil::isValidNumber(time_limit) || 
                ! StringUtil::isValidNumber(mem_limit)) {
                throw std::invalid_argument("Invalid time_limit or mem_limit value");
            }

            int time_limit_int = std::stoi(time_limit);
            int mem_limit_int = std::stoi(mem_limit);

            const std::string number = "";  // 新增题目不需要提供number
            Question question(number, title, star, time_limit_int, mem_limit_int, question_desc, header, tail);

            if (ctrl_ptr->AddQuestion(question)) {
                json_data["success"] = true;
            } else {
                json_data["success"] = false;
                json_data["error"] = "Failed to add question";
            }
        } catch (const std::exception &e) {
            json_data["success"] = false;
            json_data["error"] = e.what();
        }

        std::string json_str = json_data.toStyledString();
        resp.set_content(json_str, "application/json"); });

    // 实现删除题目功能
    svr.Delete(R"(/deleteQuestion/(\d+))", [](const httplib::Request &req, httplib::Response &resp)
               {
        Json::Value json_data;
        
        // 获取session-id
        std::string session_id = SessionUtil::get_cookie_value(req, "session_id");
        auto it = sessions.find(session_id);
        if(it == sessions.end())
        {
            // 说明用户未登录
            json_data["success"] = false;
            json_data["error"] = "Invalid session";
        }
        else
        {
            // 需要是管理员身份才能删除
            // 从sessions中查找session_id对应的用户名
            std::string username = it->second;
            User* user = new User();
            if (ctrl_ptr2->GetUserInfo(username,user) && user->isAdmin) 
            {
                // 从请求中获取题目编号
                std::string number = req.matches[1];
                try
                {
                    if(ctrl_ptr->DeleteQuestion(number))
                    {
                        json_data["success"] = true;
                    }
                    else
                    {
                        json_data["success"] = false;
                        json_data["error"] = "Failed to delete question";
                    }   
                }
                catch(const std::exception& e)
                {
                    json_data["success"] = false;
                    json_data["error"] = e.what();
                }
            }
            else
            {
                json_data["success"] = false;
                json_data["error"] = "You are not admin";
            }
        }
        std::string json_str = json_data.toStyledString();
        resp.set_content(json_str, "application/json"); });

    // 完成某一道题目接口
    svr.Post(R"(/setCompleted/(\d+))", [](const httplib::Request &req, httplib::Response &resp)
             {
        std::string number = req.matches[1];
        std::cout << "Received completion request for question number: " << number << std::endl;
        // 获取session-id
        std::string session_id = SessionUtil::get_cookie_value(req, "session_id");
        auto it = sessions.find(session_id);
        std::string username = it->second;
        User *user = new User();
        if(ctrl_ptr2->GetUserInfo(username,user))
        {
            UserProgress *new_user_progress = new UserProgress();
            new_user_progress->userId = user->id;
            new_user_progress->questionId = stoi(number);
            ctrl_ptr3->setCompleted(new_user_progress);
            delete new_user_progress;
        }
        delete user;
        Json::Value json_data;
        json_data["success"] = true;
        std::string json_str = json_data.toStyledString();
        resp.set_content(json_str, "application/json"); });
    // 获取用户某一道题目的完成情况和提交次数
    svr.Get(R"(/getUserProgress/(\d+))", [](const Request &req, Response &resp)
            {
                std::string number = req.matches[1];
                std::string username;
                // 获取session-id
                std::string session_id = SessionUtil::get_cookie_value(req, "session_id");
                auto it = sessions.find(session_id);
                if(it != sessions.end())
                {
                    username = it->second;
                }
                User *user = new User();
                if(ctrl_ptr2->GetUserInfo(username,user))
                {
                    UserProgress *user_progress = new UserProgress();
                    user_progress->userId = user->id;
                    user_progress->questionId = stoi(number);
                    bool success = ctrl_ptr3->queryProgress(user->id, stoi(number), user_progress);
                    if(success)
                    {
                        Json::Value json_data;
                        json_data["success"] = true;
                        json_data["completed"] = user_progress->isCompleted;
                        json_data["submit_times"] = user_progress->attemptCount;
                        std::string json_str = json_data.toStyledString();
                        resp.set_content(json_str, "application/json");
                    }
                    else
                    {
                        Json::Value json_data;
                        json_data["success"] = false;
                        json_data["error"] = "Failed to get user progress";
                        json_data["completed"] = false; // 假设没有进度就是未完成  
                        json_data["submit_times"] = 0; // 假设提交次数为0  
                        std::string json_str = json_data.toStyledString();
                        resp.set_content(json_str, "application/json");
                    }
                    delete user_progress;
                }
                else
                {
                    Json::Value json_data;
                    json_data["success"] = false;
                    json_data["error"] = "User not found";
                    std::string json_str = json_data.toStyledString();
                    resp.set_content(json_str, "application/json");
                }
                delete user; });
    // 获取用户列表
    svr.Get("/all_users", [](const Request &req, Response &resp)
            {
                std::string html;
                ctrl_ptr2->AllUsers(&html);                        // 调用model模块和数据库链接，获取用户列表
                resp.set_content(html, "text/html;charset=utf-8"); // 将view层渲染后的结果返回json数据
            });

    // 获取题目列表
    svr.Get("/all_questions", [](const Request &req, Response &resp)
            {
                std::string html;
                ctrl_ptr->AllQuestions(&html);                     // 调用model模块和数据库链接，获取题目列表
                resp.set_content(html, "text/html;charset=utf-8"); // 将view层渲染后的结果返回json数据
            });
    // 获取某一用户
    svr.Get(R"(/user/(\d+))", [](const Request &req, Response &resp)
            {
                std::string id = req.matches[1];
                std::string html;
                ctrl_ptr2->GetTheUser(id, &html);                  // 调用model模块和数据库链接，获取用户详情
                resp.set_content(html, "text/html;charset=utf-8"); // 将view层渲染后的结果返回json数据
            });
    // 获取某一个题目
    svr.Get(R"(/question/(\d+))", [](const Request &req, Response &resp)
            {
                std::string number = req.matches[1];
                std::string html;
                ctrl_ptr->Question(number, &html);                  // 调用model模块和数据库链接，获取题目详情
                resp.set_content(html, "text/html; charset=utf-8"); // 将view层渲染后的结果返回json数据
            });
    // 判题功能
    svr.Post(R"(/judge/(\d+))", [](const Request &req, Response &resp)
             {
                 std::string number = req.matches[1];
                 // 每点击以及提交，也就是每次判题，都更新一次提交次数
                 // 获取session-id
                 std::string session_id = SessionUtil::get_cookie_value(req, "session_id");
                 auto it = sessions.find(session_id);
                 std::string username = it->second;
                 User *user = new User();
                 if (ctrl_ptr2->GetUserInfo(username, user))
                 {
                     UserProgress *user_progress = new UserProgress();
                     user_progress->userId = user->id;
                     user_progress->questionId = stoi(number);
                     ctrl_ptr3->AddOrUpdateUserProgress(user_progress);
                     delete user_progress;
                 }
                 delete user;

                 // 调用判题功能
                 std::string result_json;
                 ctrl_ptr->Judge(number, req.body, &result_json);                 // 调用model模块和数据库链接，判题
                 resp.set_content(result_json, "application/json;charset=utf-8"); // 将view层渲染后的结果返回json数据
             });

    svr.set_base_dir("./wwwroot");
    svr.listen("0.0.0.0", 8180);
    return 0;
}