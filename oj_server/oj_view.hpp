#pragma once
#include <iostream>
#include <ctemplate/template.h>
#include <string>
#include "oj_model2.hpp"
// #include "oj_model.hpp"

namespace ns_view
{
    using namespace ns_model;
    using namespace std;

    const std::string template_path = "./template_html/";

    class View
    {
    public:
        // 显示题目列表
        void ExpandAllHtml(const vector<struct Question> &questions, std::string *html)
        {
            // 形成路径
            std::string src_html = template_path + "all_questions.html";
            // 形成数据字典
            ctemplate::TemplateDictionary root("all_questions");
            for (const auto &q : questions)
            {
                // 字典中添加很多子字典
                ctemplate::TemplateDictionary *sub = root.AddSectionDictionary("question_list");
                sub->SetValue("number", q.number);
                sub->SetValue("title", q.title);
                sub->SetValue("star", q.star);
            }
            // 获取被渲染的html
            ctemplate::Template *tpl = ctemplate::Template::GetTemplate(src_html, ctemplate::DO_NOT_STRIP);
            // 开始完成渲染功能
            tpl->Expand(html, &root); // 把数据字典渲染进html
        }
        // 点击列表中的索引跳转到指定题目的功能
        void OneExpandHtml(const struct Question &q, std::string *html)
        {
            // 1. 形成路径
            std::string src_html = template_path + "one_question.html";

            // 2. 形成数字字典?
            ctemplate::TemplateDictionary root("one_question");
            root.SetValue("number", q.number);
            root.SetValue("title", q.title);
            root.SetValue("star", q.star);
            root.SetValue("desc", q.desc);
            root.SetValue("pre_code", q.header);

            // 3. 获取被渲染的html
            ctemplate::Template *tpl = ctemplate::Template::GetTemplate(src_html, ctemplate::DO_NOT_STRIP);

            // 4. 开始完成渲染功能?
            tpl->Expand(html, &root);
        }
        // 显示所有用户页面
        void UserAllHtml(const vector<struct User> &users, std::string *html)
        {
            // 1. 形成路径
            std::string src_html = template_path + "all_users.html";

            // 2. 形成数据字典
            ctemplate::TemplateDictionary root("all_users");
            for (const auto &u : users)
            {
                // 字典中添加很多子字典
                ctemplate::TemplateDictionary *sub = root.AddSectionDictionary("user_list");
                sub->SetValue("number", to_string(u.id));
                sub->SetValue("username", u.name);
                sub->SetValue("isAdmin", u.isAdmin ? "管理员" : "普通用户");
            }
            // 3. 获取被渲染的html
            ctemplate::Template *tpl = ctemplate::Template::GetTemplate(src_html, ctemplate::DO_NOT_STRIP);

            // 4. 开始完成渲染功能
            tpl->Expand(html, &root);
        }
        // 显示指定用户页面
        void UserOneHtml(const User &user, std::string *html)
        {
            // 1. 形成路径
            std::string src_html = template_path + "one_user.html";

            // 2. 形成数据字典
            ctemplate::TemplateDictionary root("one_user");
            root.SetValue("username", user.name);
            root.SetValue("isAdmin", user.isAdmin ? "管理员" : "普通用户");

            // 3. 获取被渲染的html
            ctemplate::Template *tpl = ctemplate::Template::GetTemplate(src_html, ctemplate::DO_NOT_STRIP);

            // 4. 开始完成渲染功能
            tpl->Expand(html, &root);
        }
        // 显示所有提交页面（因为一个用户会做很多的题目）
        void AllUserProgress(const vector<struct UserProgress> &user_progresses, std::string *html)
        {
            // 1. 形成路径 现在的思路是在用户个人中心页面显示做过题目的记录
            std::string src_html = template_path + "one_user.html";
            // 2. 形成数据字典
            ctemplate::TemplateDictionary root("all_user_progress");
            for (const auto &up : user_progresses)
            {
                // 字典中添加很多子字典
                ctemplate::TemplateDictionary *sub = root.AddSectionDictionary("user_progress_list");
                sub->SetValue("question_number", to_string(up.questionId));
                sub->SetValue("isCompleted", up.isCompleted ? "已完成" : "未完成");
                sub->SetValue("completedTime", StringUtil::timeToStringForFrontEnd(up.completedTime));
                sub->SetValue("attemptCount", to_string(up.attemptCount));
            }
            // 3. 获取被渲染的html
            ctemplate::Template *tpl = ctemplate::Template::GetTemplate(src_html, ctemplate::DO_NOT_STRIP);

            // 4. 开始完成渲染功能
            tpl->Expand(html, &root);
        }

        void GenerateUserDetailPage(const struct User &user, const vector<struct UserProgress> &user_progresses, std::string *html)
        {
            // 1. 形成路径
            std::string src_html = template_path + "one_user.html";

            // 2. 形成数据字典
            ctemplate::TemplateDictionary root("one_user");
            root.SetValue("username", user.name);
            root.SetValue("isAdmin", user.isAdmin ? "管理员" : "普通用户");

            // 3. 填充用户进度列表
            for (const auto &up : user_progresses)
            {
                ctemplate::TemplateDictionary *sub = root.AddSectionDictionary("user_progress_list");
                sub->SetValue("question_number", to_string(up.questionId));
                sub->SetValue("isCompleted", up.isCompleted ? "已完成" : "未完成");
                sub->SetValue("completedTime", StringUtil::timeToStringForFrontEnd(up.completedTime));
                sub->SetValue("attemptCount", to_string(up.attemptCount));
            }

            // 4. 获取被渲染的html
            ctemplate::Template *tpl = ctemplate::Template::GetTemplate(src_html, ctemplate::DO_NOT_STRIP);

            // 5. 开始完成渲染功能
            tpl->Expand(html, &root);
        }
    };
}