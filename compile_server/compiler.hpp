#pragma once

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "../comm/util.hpp"
#include "../comm/Log.hpp"

namespace ns_compiler
{
    // 使用工具类对字符串进行拼接
    using namespace ns_util;
    using namespace ns_log;
    class Compiler
    {
    public:
        Compiler() {}
        ~Compiler() {}
        // 输入文件名:filename,要对文件名的后缀进行拼接
        static bool compile(const std::string &filename)
        {
            pid_t pid = fork();
            if (pid < 0)
            {
                LOG(ERROR) << "内部错误，创建子进程失败" << "\n";
                return false;
            }
            else if (pid == 0)
            {
                // 对标准错误进行重定向
                umask(0);
                int _errfd = open(PathUtil::CompileErr(filename).c_str(), O_CREAT | O_WRONLY,0644);
                if (_errfd < 0)
                {
                    LOG(WARNING) << "创建stderror文件失败" << "\n";
                    exit(1);
                }
                dup2(_errfd, 2);
                // 子进程
                // LOG(INFO)<<PathUtil::Exe(filename)<<"->"<<PathUtil::Src(filename)<<"\n";
                execlp("g++","g++", "-o", PathUtil::Exe(filename).c_str(), PathUtil::Src(filename).c_str(), "-std=c++11", nullptr);
                LOG(ERROR) << "子进程程序替换失败，可能参数错误" << "\n";
                exit(2);
            }
            else
            {
                waitpid(pid, nullptr, 0);
                // 确认是否编译成功,判断文件存在则是编译成功
                if (FileUtil::FileIsExist(PathUtil::Exe(filename)))
                {
                    LOG(INFO) << "编译成功" << "\n";
                    return true;
                }
            }
            LOG(ERROR) << "编译失败，不存在目标文件" << "\n";
            return false;
        }
    };
}