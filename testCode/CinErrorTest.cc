#include <iostream>
#include <vector>
#include <limits>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>

using namespace std;

// 超时处理函数
void timeout_handler(int signum)
{
    cerr << "Program timed out." << endl;
    exit(1);
}

// 设置超时机制
void set_timeout(int seconds)
{
    struct itimerval timer;
    timer.it_value.tv_sec = seconds;
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;

    if (setitimer(ITIMER_REAL, &timer, NULL) == -1)
    {
        perror("Error setting timer");
        exit(1);
    }

    struct sigaction sa;
    sa.sa_handler = timeout_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGALRM, &sa, NULL) == -1)
    {
        perror("Error setting signal handler");
        exit(1);
    }
}

void test()
{
    int a = 0;
    cout << "Please enter the number of elements: ";
    while (true)
    {
        cin >> a;
        if (cin.fail() || a <= 0)
        {
            cin.clear(); // 清除错误标志
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // 忽略剩余输入
            cout << "Invalid input. Please enter a positive integer: ";
        }
        else
        {
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // 忽略剩余输入
            break;
        }
    }

    vector<int> arr(a, 0);
    cout << "Please enter " << a << " integers: ";
    for (int i = 0; i < a; i++)
    {
        while (true)
        {
            cin >> arr[i];
            if (cin.fail())
            {
                cin.clear(); // 清除错误标志
                cin.ignore(numeric_limits<streamsize>::max(), '\n'); // 忽略剩余输入
                cout << "Invalid input. Please enter an integer: ";
            }
            else
            {
                break;
            }
        }
    }

    // 打印
    cout << "You entered: " << endl;
    for (int i = 0; i < a; i++)
    {
        cout << arr[i] << endl;
    }
}

int main()
{
    set_timeout(10); // 设置10秒的超时时间
    test();
    return 0;
}
