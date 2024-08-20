#include <iostream>
#include <algorithm>
#include <vector>

using namespace std;

class Solution
{
public:
    int Max(vector<int>& arr)
    {
        // 请将你的代码写在这⾥
        return 0;
    }
};
#ifndef COMPILE_ONLINE
#include "header.cpp"
#endif

void Test1()
{
    vector<int> arr{1, 2, 3, 4, 5, 6, 7};
    int ret = Solution().Max(arr);
    if (ret == 7)
    {
        std::cout << "Test1 ok!" << std::endl;
    }
    else
    {
        std::cout << "Test1 failed! 最大值为7"
                  << std::endl;
    }
}
void Test2()
{
    vector<int> arr{-1, -2, -3, -4, -5, -6, -7};
    int ret = Solution().Max(arr);

    if (ret == -1)
    {
        std::cout << "Test2 ok!" << std::endl;
    }
    else
    {
        std::cout << "Test2 failed! 最大值为-1"
                  << std::endl;
    }
}
int main()
{

    Test1();
    Test2();

    return 0;
}
