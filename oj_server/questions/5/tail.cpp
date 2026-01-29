#ifndef COMPILER_ONLINE
#include "header.cpp"
#endif

void validateInputAndTest(Solution &s)
{
    int a = 0;
    while (!(cin >> a) || a <= 0)
    {
        cout << "Invalid input. Please enter a positive integer: ";
        cin.clear();//clear the error flag
        cin.ignore(numeric_limits<streamsize>::max(), '\n');//clear the input buffer
    }

    int result = s.square(a);
    cout << "The square of " << a << " is " << result << endl;
}

void Test1()
{
    Solution s;
    int a = 5;
    int expected = 25;
    if (s.square(a) == expected)
    {
        cout << "Test1 Passed" << endl;
    }
    else
    {
        cout << "Test1 Failed" << endl;
    }
}

void Test2()
{
    Solution s;
    int a = 10;
    int expected = 100;
    if (s.square(a) == expected)
    {
        cout << "Test2 Passed" << endl;
    }
    else
    {
        cout << "Test2 Failed" << endl;
    }
}

int main()
{
    Test1();
    Test2();

    Solution s;
    validateInputAndTest(s);

    return 0;
}
