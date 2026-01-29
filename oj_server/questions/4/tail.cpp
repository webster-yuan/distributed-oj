// 设计测试用例
//编译的时候，会将这三行去掉，仅仅是为了方便我们设计测试用例
#ifndef COMPILER_ONLINE
#include "header.cpp"
#endif

void Test1()
{
    Solution s;
    vector<int> arr ={2,7,11,15};
    vector<int> ret = s.twoSum(arr, 9);
    if(ret[0]!= 0 || ret[1]!= 1)
    {
        cout<<"Test1 Failed"<<endl;
    }
    else
    {
        cout<<"Test1 Passed"<<endl;
    }
}
void Test2()
{
    Solution s;
    vector<int> arr ={3,2,4};
    vector<int> ret = s.twoSum(arr, 6);
    if(ret[0]!= 1 || ret[1]!= 2)
    {
        cout<<"Test2 Failed"<<endl;
    }
    else
    {
        cout<<"Test2 Passed"<<endl;
    }
}
int main()
{
    Test1();
    Test2();
    return 0;
}
