#pragma once

#include <jsoncpp/json/json.h>
#include <string>

#include "compiler.hpp"
#include "runner.hpp"
#include "../comm/Log.hpp"

namespace ns_compile_server
{
    using namespace ns_log;
    using namespace ns_runner;
    using namespace ns_compiler;

    class compile_server
    {
    public:
        // 在运行中会生成的临时文件个数并不清楚，但总数确定，逐个判断文件是否存在然后删除
        static void RemoveTempFile(const std::string filename)
        {
            std::string _src = PathUtil::Src(filename);
            if (FileUtil::FileIsExist(_src))
                unlink(_src.c_str());

            std::string _compile_err = PathUtil::CompileErr(filename);
            if (FileUtil::FileIsExist(_compile_err))
                unlink(_compile_err.c_str());

            std::string _execute = PathUtil::Exe(filename);
            if (FileUtil::FileIsExist(_execute))
                unlink(_execute.c_str());

            std::string _stdin = PathUtil::Stdin(filename);
            if (FileUtil::FileIsExist(_stdin))
                unlink(_stdin.c_str());

            std::string _stdout = PathUtil::Stdout(filename);
            if (FileUtil::FileIsExist(_stdout))
                unlink(_stdout.c_str());

            std::string _stderr = PathUtil::Stderr(filename);
            if (FileUtil::FileIsExist(_stderr))
                unlink(_stderr.c_str());
        }

        static std::string ToDesc(const int out_status, const std::string &filename)
        {
            std::string desc;
            switch (out_status)
            {
            case 0:
                desc = "程序运行完成";
                break;
            case -1:
                desc = "用户代码上传为空";
                break;
            case -2:
                desc = "程序内部运行错误";
                break;
            case -3:
                FileUtil::ReadFile(PathUtil::CompileErr(filename), &desc, true);
                break;
            case SIGABRT: // 6
                desc = "超出内存使用限制";
                break;
            case SIGXCPU: // 24
                desc = "cpu使用时间超出限制";
                break;
            default:
                desc = ("未定义错误" + out_status);
                break;
            }
            return desc;
        }
        /**
         * 输入：
         * code：用户的代码
         * input：输入的参数
         * cpu_limit:cpu的限制
         * mem_limit:内存的限制
         * 输出：
         * 必填：
         * status：状态码
         * reason：请求结果
         *
         * 选填：
         * stdout：程序运行完的结果
         * stderr：程序运行完的错误结果
         * **/
        static void Start(const std::string &json_in, std::string *json_out)
        {
            Json::Value in_value;
            Json::Reader reader;
            std::string filename;
            reader.parse(json_in, in_value);

            std::string code = in_value["code"].asString();
            std::string input = in_value["input"].asString();
            int cpu_limit = in_value["cpu_limit"].asInt();
            int mem_limit = in_value["mem_limit"].asInt();

            int out_status = 0;
            int running_status;
            Json::Value _json_out;

            if (code.size() == 0)
            {
                // 用户上传代码为空
                out_status = -1;
                goto END;
            }
            // 形成唯一临时文件名
            filename = FileUtil::UniFileName();
            // 形成临时的src文件
            if (!FileUtil::WriteFile(PathUtil::Src(filename), code))
            {
                // 写临时文件失败
                out_status = -2;
                goto END;
            }

            if (!Compiler::compile(filename))
            {
                // 编译文件失败
                out_status = -3;
                goto END;
            }
            running_status = Runner::Run(filename, cpu_limit, mem_limit);
            if (running_status < 0)
            {
                // runner内部错误
                out_status = -2;
            }
            else if (running_status > 0)
            {
                // 用户提交的代码运行异常
                out_status = running_status;
            }
            else
            {
                // 程序运行完成
                out_status = 0;
            }
        END:
            _json_out["status"] = out_status;
            _json_out["reason"] = ToDesc(out_status, filename);
            if (out_status == 0)
            {
                std::string _stdout;
                FileUtil::ReadFile(PathUtil::Stdout(filename), &_stdout, true);
                _json_out["stdout"] = _stdout;

                std::string _stderr;
                FileUtil::ReadFile(PathUtil::Stderr(filename), &_stderr, true);
                _json_out["stderr"] = _stderr;
            }
            Json::FastWriter writer;
            *json_out = writer.write(_json_out);

            // 清理临时文件
            RemoveTempFile(filename);
        }
    };

}