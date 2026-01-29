#include <iostream>
#include <ctemplate/template.h>
#include <fstream>
using namespace std;
void test1()
{
    // 创建数据字典
    ctemplate::TemplateDictionary dict("test1");
    // 设置模板变量的值
    dict.SetValue("TITLE", "测试标题");
    dict.SetValue("HEADER", "测试头部");
    dict.SetValue("BODY", "测试内容");
    // 加载模板文件
    ctemplate::Template *tpl = ctemplate::Template::GetTemplate("template.html", ctemplate::DO_NOT_STRIP);
    // 拓展模板，将结果写入字符串
    std::string output;
    tpl->Expand(&output, &dict);
    // 输出渲染结果
    std::cout << output << std::endl;
    // 将生成的HTML文件保存到文件中
    std::ofstream outFile("output1.html");
    //文件流检查：在写入文件前，添加检查 outFile.is_open() 以确保文件成功打开
    if (outFile.is_open())
    {
        outFile << output;
        outFile.close();
    }
    else
    {
        std::cout << "文件打开失败！" << std::endl;
    }
}
void test2()
{
    struct Question
    {
        int number;
        std::string title;
        int star;
    };
    // 1. 创建数据字典
    ctemplate::TemplateDictionary dict("test2");

    std::vector<Question> questions = {
        {1, "题目一", 3},
        {2, "题目二", 2},
        {3, "题目三", 1}
    };
    // 2. 设置模板变量的值
    for(auto & q:questions)
    {
        // 2.1 设置单个问题的变量
        ctemplate::TemplateDictionary* subDict=dict.AddSectionDictionary("question_list");
        subDict->SetIntValue("number",q.number);
        subDict->SetValue("title",q.title);
        subDict->SetIntValue("star",q.star);
    }
    // 3. 加载模板文件
    // ctemplate::DO_NOT_STRIP: 保留模板文件中的空白符号和注释
    ctemplate::Template*tpl=ctemplate::Template::GetTemplate("template.html",ctemplate::DO_NOT_STRIP);
    // 4. 拓展模板，将结果写入字符串
    std::string output;
    tpl->Expand(&output,&dict);
    // 5. 输出渲染结果
    std::cout << output << std::endl;
    // 6. 将生成的HTML文件保存到文件中
    std::ofstream outFile("output2.html");
    // 文件流检查：在写入文件前，添加检查 outFile.is_open() 以确保文件成功打开
    if (outFile.is_open())
    {
        outFile << output;
        outFile.close();
    }
    else
    {
        std::cout << "文件打开失败！" << std::endl;
    }
}   
int main()
{
    test1();
    test2();
    return 0;
}