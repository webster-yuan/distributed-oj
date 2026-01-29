// 设计测试用例
//编译的时候，会将这三行去掉，仅仅是为了方便我们设计测试用例
#ifndef COMPILER_ONLINE
#include "header.cpp"
#endif

void Test1()
{
    // 生成测试代码1
    string str1="1AB2345CD";
    string str2= "12345EF";
    string ret = Solution().LCS(str1,str2);
    // 验证结果
    if(ret == "2345")
    {
        std::cout << "Test1 ok!" << std::endl;
    }
    else
    {
        std::cout << "Test1 不 ok!" << std::endl;
    }
}
void Test2()
{
    // 生成测试代码2,内容和1区别开来
    string str1="ABCD12345";
    string str2= "12345EF";
    string ret = Solution().LCS(str1,str2);
    // 验证结果
    if(ret == "12345")
    {
        std::cout << "Test2 ok!" << std::endl;
    }
    else
    {
        std::cout << "Test2 不 ok!" << std::endl;
    }
}
int main()
{
    Test1();
    Test2();
    return 0;
}
