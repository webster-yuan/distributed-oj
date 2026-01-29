// 设计测试用例
//编译的时候，会将这三行去掉，仅仅是为了方便我们设计测试用例
#ifndef COMPILER_ONLINE
#include "header.cpp"
#endif

void Test1()
{
    //定义临时对象来完成方法的调用
    vector<int>arr{1,3,2,45,5,6};
    Solution().quickSort(arr);
    vector<int>test={1,3,2,45,5,6};
    sort(test.begin(),test.end());
    bool ret =true;
    for(int i=0;i<test.size();i++)
    {
        if(arr[i] != test[i])
        {
            ret=false;
        }
    }
    if (ret)
    {
        std::cout << "Test1 ok!" << std::endl;
    }
    else
    {
        std::cout << "Test1 failed!" << std::endl;
    }
}
void Test2()
{
     //定义临时对象来完成方法的调用
    vector<int>arr{22,33,11,12,14,25};
    Solution().quickSort(arr);
    vector<int>test={22,33,11,12,14,25};
    sort(test.begin(),test.end());
    bool ret =true;
    for(int i=0;i<test.size();i++)
    {
        if(arr[i] != test[i])
        {
            ret=false;
        }
    }
    if (ret)
    {
        std::cout << "Test2 ok!" << std::endl;
    }
    else
    {
        std::cout << "Test2 failed!" << std::endl;
    }
}
int main()
{
    Test1();
    Test2();
    return 0;
}
