#pragma once

#include <string>
#include <iostream>

#include "util.hpp"
namespace ns_log
{
    using namespace ns_util;
    // 日志等级
    enum
    {
        INFO,
        DEBUG,
        WARNING,
        ERROR,
        FATAL
    };

    std::ostream &Log(const std::string level, const std::string filename, int line)
    {
        // 日志等级
        std::string message;
        message += "[";
        message += level;
        message += "]";

        // 文件名字
        message += "[";
        message += filename;
        message += "]";

        // 错误行
        message += "[";
        message += std::to_string(line);
        message += "]";

        // 添加时间戳
        message += "[";
        message += TimeUtil::GetTimeStamp();
        message += "]";
        std::cout << message;
        return std::cout;
    }
#define LOG(level) Log(#level, __FILE__, __LINE__)
}