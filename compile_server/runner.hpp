#pragma once

#include <string>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "../comm/Log.hpp"
#include "../comm/util.hpp"

namespace ns_runner
{
    using namespace ns_log;
    using namespace ns_util;

    class Runner
    {
    public:
        /**
         * 程序运行：
         * 1.运行成功，结果正确
         * 2.运行成功，结果错误
         * 3.运行失败，程序异常
         *
         * Run只关心运行成功或运行失败，并不关心结果正确与否
         * 运行时打开三个文件：
         * 标准输出：
         * 标准输入：
         * 标准错误：
         * **/
        // 只需要文件名
        static int Run(const std::string filename)
        {
            umask(0);
            int _stdin_fd = open(PathUtil::Stdin(filename), O_CREAT | O_RDONLY, 0644);
            int _stdout_fd = open(PathUtil::Stdout(filename), O_CREAT | O_WRONLY, 0644);
            int _stderr_fd = open(PathUtil::Stderr(filename), O_CREAT | O_WRONLY, 0644);

            if (_stdin_fd < 0 || _stdout_fd < 0 || _stderr_fd < 0)
            {
                return -1; // 代表打开文件失败
            }
            pid_t pid = fork();
            if (pid < 0)
            {
                close(_stdin_fd);
                close(_stdout_fd);
                close(_stderr_fd);
                return -2; // 代表创建线程失败
            }
            else if (pif == 0)
            {
                dup2(_stdin_fd, 0);
                dup2(_stdout_fd, 1);
                dup2(_stderr_fd, 2);
                execl(PathUtil::Exe(filename).c_str(), PathUtil::Exe(filename).c_str(), nullptr);
                return -3; // 代表程序替换错误
            }
            else
            {
                // 父进程不关心文件描述符，只需要关心子进程运行程序是否发生异常，并把异常返回给上层
                close(_stdin_fd);
                close(_stdout_fd);
                close(_stderr_fd);

                int status;
                waitpid(pid, &status, 0); // 程序异常是通过信号实现的，status可以判断程序是否异常退出

                return status & 0x7f;
            }
        }
    };

}