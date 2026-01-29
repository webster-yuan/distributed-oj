#include <iostream>
#include <signal.h>
#include "../comm/httplib.h"
#include "oj_control.hpp"
using namespace ns_control;
using namespace httplib;

static Control *ctrl_ptr = nullptr;

static UserControl *ctrl_ptr2 = nullptr;

// 当收到信号之后就重新上线所有服务器
void Recovery(int sigo)
{
    ctrl_ptr->RecoveryMachine();
}

static std::string get_cookie_value(const httplib::Request &req, const std::string &cookie_name)
{
    // 尝试获取"Cookie"请求头的值
    auto cookie_header = req.get_header_value("Cookie");
    if (cookie_header.empty())
    {
        return ""; // 如果没有Cookie头，返回空字符串
    }

    // 分割Cookie字符串，Cookies通常以"; "分割
    std::istringstream cookie_stream(cookie_header);
    std::string cookie;
    while (std::getline(cookie_stream, cookie, ';'))
    {
        // 去除可能的空格
        auto pos = cookie.find_first_not_of(" ");
        if (pos != std::string::npos)
        {
            cookie.erase(0, pos);
        }
        // 分割cookie名和值
        pos = cookie.find('=');
        if (pos != std::string::npos)
        {
            std::string key = cookie.substr(0, pos);
            std::string value = cookie.substr(pos + 1);
            // 检查cookie名是否是我们想要的
            if (key == cookie_name)
            {
                return value;
            }
        }
    }

    // 如果没有找到匹配的cookie名，返回空字符串
    return "";
}

// 会话存储，将会话ID映射到用户名
std::map<std::string, std::string> sessions;

int main()
{
    signal(SIGQUIT, Recovery);
    // 用户请求路由服务功能
    Server svr;
    ns_control::Control ctrl;
    ctrl_ptr = &ctrl;

    ns_control::UserControl ctrl2;
    ctrl_ptr2 = &ctrl2;

    svr.Post("/register", [&ctrl2](const httplib::Request &req, httplib::Response &resp)
             {
              // 实际应用中应对密码进行加密处理
            std::string username = req.has_param("username") ? req.get_param_value("username") : req.get_file_value("username").content;
            std::string password = req.has_param("password") ? req.get_param_value("password") : req.get_file_value("password").content;
            // 生成salt算法
            string salt = UUIDUtil::generate_hex(10);// 生成10个16进制数字作为盐
            string encryptedPassword =MD5Util::generateSaltedMD5(password,salt);
            User* newUser = new User(username, encryptedPassword, salt);
            // 调用注册接口
            bool success = ctrl2.RegisterUser(newUser);

            if (success) {
                Json::Value json_data;
                json_data["success"] = success; // 设置 success 字段

                // 将 JSON 对象转换为字符串，并设置为响应内容
                std::string json_str = json_data.toStyledString();
                resp.set_content(json_str, "application/json");
            } else {
                string res="注册失败";
                resp.set_content(res, "text/html");
            } });

    svr.Post("/login", [&ctrl2](const httplib::Request &req, httplib::Response &resp)
             {
                std::string username = req.has_param("username") ? req.get_param_value("username") : req.get_file_value("username").content;
                std::string password = req.has_param("password") ? req.get_param_value("password") : req.get_file_value("password").content;
                if (ctrl2.LoginUser(username, password)) {
                    std::string session_id = SessionUtil::generate_alpha_numeric_session_id(32);
                    sessions[session_id] = username; // 保存会话ID和用户名的映射
                    // 设置会话ID到响应头中
                    resp.set_header("Set-Cookie", "session_id=" + session_id);
                    // 构造 JSON 响应
                    Json::Value json_data;
                    json_data["success"] = "success"; // 设置 success 字段
                    json_data["session_id"] = session_id; // 设置 session_id 字段
                    
                    cout<<"session-id:"<<session_id<<endl;
                    
                    // 将 JSON 对象转换为字符串，并设置为响应内容
                    std::string json_str = json_data.toStyledString();
                    resp.set_content(json_str, "application/json");
                }else{
                    resp.set_content("Login Failed", "text/plain;charset=utf-8");
                } });

    // 检查登录状态
    svr.Get("/check_login", [](const httplib::Request &req, httplib::Response &resp) {
        std::string session_id = get_cookie_value(req, "session_id");
        auto it = sessions.find(session_id);
        Json::Value json_data;
        if (it != sessions.end()) {
            json_data["success"] = true;
            json_data["username"] = it->second;
        } else {
            json_data["success"] = false;
        }
        std::string json_str = json_data.toStyledString();
        resp.set_content(json_str, "application/json");
    });
    // 查询用户信息
    svr.Get("/user_info", [](const httplib::Request &req, httplib::Response &resp) {
        std::string session_id = get_cookie_value(req, "session_id");
        auto it = sessions.find(session_id);
        Json::Value json_data;

        if (it != sessions.end()) {
            const std::string username = it->second;
            User* user = new User();
            if (ctrl_ptr2->GetUserInfo(username, user)) {
                json_data["success"] = true;
                json_data["username"] = user->name;
                json_data["isAdmin"] = user->isAdmin;
            } else {
                json_data["success"] = false;
            }
        } else {
            json_data["success"] = false;
        }

        std::string json_str = json_data.toStyledString();
        resp.set_content(json_str, "application/json");
    });
    // 获取所有的题目
    svr.Get("/all_questions", [&ctrl](const Request &req, Response &resp)
            {
                // resp.set_content("这是所有题目的列表","text/plain;charset=utf-8");
                // 返回一张包含有所有题目的.html的网页,根据Ctrl获取model信息
                std::string html;
                ctrl.AllQuestions(&html);
                resp.set_content(html, "text/html;charset=utf-8"); });

    // 根据题目编号获取内容，拼接代码组成网页
    // 正则匹配,question/(/d+))
    // R"()",保持字符串的原貌，不做相关的转义
    svr.Get(R"(/question/(\d+))", [&ctrl](const Request &req, Response &resp)
            {
                std::string number = req.matches[1]; // 1号内容存放的就是内个题号;%d
                std::string html;
                ctrl.Question(number, &html);
                resp.set_content(html, "text/html; charset=utf-8"); });

    // 用户提交代码使用判题功能（1. 每道题的测试用例2. compile_and_run）
    svr.Post(R"(/judge/(\d+))", [&ctrl](const Request &req, Response &resp)
             {
                 std::string number = req.matches[1]; // 通过字段获取
                 std::string result_json;
                 ctrl.Judge(number, req.body, &result_json);
                 // resp.set_content("这是指定的判题"+ number,"text/plain;charset=utf-8");
                 resp.set_content(result_json, "application/json;charset=utf-8"); });

    svr.set_base_dir("./wwwroot"); // 设置默认首页是wwwroot目录下
    svr.listen("0.0.0.0", 8180);
    return 0;
}