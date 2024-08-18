#pragma once

#include <string>
#include <sys/time.h>
#include <atomic>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <iostream>
namespace ns_util
{
    class TimeUtil
    {
    public:
        static std::string GetTimeStamp()
        {
            struct timeval _time;
            gettimeofday(&_time, nullptr);
            return std::to_string(_time.tv_sec);
        }
        static std::string GetuTimeStamp()
        {
            struct timeval _time;
            gettimeofday(&_time, nullptr);

            return std::to_string(_time.tv_sec * 1000 + _time.tv_usec / 1000);
        }
    };
    static std::string tmp_file = "./temp/";

    class PathUtil
    {
    public:
        static std::string AddSuffix(const std::string filename, const std::string suffix)
        {
            std::string file = tmp_file;
            file += filename;
            file += suffix;
            return file;
        }
        // 编译时用到的：
        static std::string Src(const std::string filename)
        {
            return AddSuffix(filename, ".cpp");
        }
        static std::string Exe(const std::string filename)
        {
            return AddSuffix(filename, ".exe");
        }
        static std::string CompileErr(const std::string filename)
        {
            return AddSuffix(filename, ".compileerr");
        }
        // 运行时用到的：
        static std::string Stdin(const std::string filename)
        {
            return AddSuffix(filename, ".stdin");
        }
        static std::string Stdout(const std::string filename)
        {
            return AddSuffix(filename, ".stdout");
        }
        static std::string Stderr(const std::string filename)
        {
            return AddSuffix(filename, ".stderr");
        }
    };

    class FileUtil
    {
    public:
        // 需要传入文件的路径+文件名
        static bool FileIsExist(const std::string filepath)
        {
            struct stat res;
            if (stat(filepath.c_str(), &res) == 0)
            {
                return true;
            }
            return false;
        }
        // 用毫秒级时间戳+唯一性增长的id确保唯一性文件名
        static std::string UniFileName()
        {
            std::string time = TimeUtil::GetuTimeStamp();
            static std::atomic_int id(0);
            id++;

            return time + "_" + std::to_string(id);
        }

        // 需要传入文件的路径+文件名
        static bool WriteFile(const std::string filename, const std::string &content)
        {
            std::ofstream out(filename);
            if (!out.is_open())
            {
                return false;
            }
            out.write(content.c_str(), content.size());
            out.close();
            return true;
        }

        // 需要传入文件的路径+文件名
        static bool ReadFile(const std::string filename, std::string *content, bool keep)
        {
            content->clear();
            std::ifstream in(filename);

            if (!in.is_open())
            {
                return false;
            }
            std::string buf;
            while (std::getline(in, buf))
            {
                *content += buf;
                *content += keep ? "\n" : "";
            }
            in.close();
            return true;
        }
    };
    class StringUtil
    {
        /**str:输入型，要分割的字符串
         * v:输出型，存储分割的子串
         * seq:输入型，分隔符
         */
    public:
        static void SplitString(const std::string &str, std::vector<std::string> *v, std::string seq)
        {
            boost::split((*v), str, boost::is_any_of(seq), boost::algorithm::token_compress_on);
        }
    };
}