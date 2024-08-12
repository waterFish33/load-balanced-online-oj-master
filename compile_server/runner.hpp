#pragma once

#include <string>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "../comm/Log.hpp"
#include "../comm/util.hpp"

namespace ns_runner
{
    using namespace ns_log;
    using namespace ns_util;

    class Runner
    {
    public:
        static void SetLimit(int _cpu_limit, int _mem_limit)
        {
            // 设置cpu资源的限制
            struct rlimit _cpu;
            _cpu.rlim_cur = _cpu_limit;
            _cpu.rlim_max = RLIM_INFINITY;
            setrlimit(RLIMIT_CPU, &_cpu);

            // 设置内存资源的限制
            struct rlimit _mem;
            _mem.rlim_cur = _mem_limit;
            _mem.rlim_max = RLIM_INFINITY;
            setrlimit(RLIMIT_AS, &_mem);
        }
        /**
         * 程序运行：
         * 1.返回值 == 0：运行成功，结果正确
         * 2.返回值 > 0：运行成功，结果错误
         * 3.返回值 < 0：运行失败，程序内部异常
         *
         * Run只关心运行成功或运行失败，并不关心结果正确与否
         * 运行时打开三个文件：
         * 标准输出：
         * 标准输入：
         * 标准错误：
         *
         * 限制程序运行使用的资源上限，目前限制cpu资源和内存资源(KB)
         * **/
        // 只需要文件名
        static int Run(const std::string filename, int _cpu, int _mem)
        {
            umask(0);
            int _stdin_fd = open(PathUtil::Stdin(filename).c_str(), O_CREAT | O_RDONLY, 0644);
            int _stdout_fd = open(PathUtil::Stdout(filename).c_str(), O_CREAT | O_WRONLY, 0644);
            int _stderr_fd = open(PathUtil::Stderr(filename).c_str(), O_CREAT | O_WRONLY, 0644);

            if (_stdin_fd < 0 || _stdout_fd < 0 || _stderr_fd < 0)
            {
                LOG(ERROR) << "打开文件失败" << "\n";
                return -1; // 代表打开文件失败
            }
            pid_t pid = fork();
            if (pid < 0)
            {
                close(_stdin_fd);
                close(_stdout_fd);
                close(_stderr_fd);
                LOG(ERROR) << "创建线程失败" << "\n";
                return -2; // 代表创建线程失败
            }
            else if (pid == 0)
            {
                dup2(_stdin_fd, 0);
                dup2(_stdout_fd, 1);
                dup2(_stderr_fd, 2);

                SetLimit(_cpu, _mem*1024);
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
                LOG(INFO) << "程序运行完成" << "\n";
                return status & 0x7f;
            }
        }
    };

}