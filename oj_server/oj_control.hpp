#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <mutex>
#include <cassert>
#include <fstream>
#include <jsoncpp/json/json.h>

#include "../comm/httplib.h"
#include "./oj_model_MySQL.hpp"
#include "./oj_view.hpp"
#include "../comm/Log.hpp"
#include "../comm/util.hpp"

namespace ns_control
{
    using namespace ns_model;
    using namespace ns_view;
    using namespace ns_log;
    using namespace ns_util;
    using namespace httplib;

    class Machine
    {
    public:
        std::string ip;     // 主机IP
        int port;           // 主机端口号
        std::uint64_t load; // 主机的负载情况
        std::mutex *mtx;    // 保护load临界资源

        void IncLoad()
        {
            mtx->lock();
            ++load;
            mtx->unlock();
        }

        void DecLoad()
        {
            mtx->lock();
            --load;
            mtx->unlock();
        }
        void ResetLoad()
        {
            mtx->lock();
            load=0;
            mtx->unlock();
        }
    };

    std::string machine_conf = "./service_machine/service_machine.conf";
    // 负载均衡模块
    class LoadBalance
    {
    public:
        std::vector<Machine> machines;
        std::vector<int> on_machine;
        std::vector<int> off_machine;
        std::mutex mtx;

        LoadBalance()
        {
            assert(LoadMachine(machine_conf));
        }
        ~LoadBalance() {}
        // 加载机器信息
        bool LoadMachine(std::string machine_list)
        {
            std::ifstream in(machine_list);
            if (!in.is_open())
            {
                LOG(FATAL) << "加载machine_list失败" << std::endl;
                return false;
            }
            std::string line;
            while (std::getline(in, line))
            {
                std::vector<std::string> temp;
                StringUtil::SplitString(line, &temp, ":");
                if (temp.size() != 2)
                {
                    return false;
                }
                Machine machine;
                machine.ip = temp[0];
                machine.port = atoi(temp[1].c_str());
                machine.mtx = new std::mutex;
                machine.load = 0;
                on_machine.push_back(on_machine.size());
                machines.push_back(machine);
            }

            LOG(INFO) << "加载machine_list成功" << std::endl;
            in.close();
            return true;
        }
        // 选择负载低的服务器
        bool SmartChoice(int *id, Machine **machine)
        {
            mtx.lock();
            if (on_machine.size() == 0)
            {
                LOG(FATAL) << "所有服务器均已离线" << std::endl;
                mtx.unlock();
                return false;
            }
            *id = on_machine[0];
            *machine = &machines[on_machine[0]];

            uint64_t load = machines[on_machine[0]].load;

            for (int i = 1; i < on_machine.size(); i++)
            {
                if (load > machines[on_machine[i]].load)
                {
                    load = machines[on_machine[i]].load;
                    (*machine) = &machines[on_machine[i]];
                    (*id) = on_machine[i];
                }
            }
            mtx.unlock();
            return true;
        }

        void ShowMashine(){
            std::cout<<"在线主机：";
            for(int i=0;i<on_machine.size();i++){
                std::cout<<on_machine[i]<<" ";
            }
            std::cout<<std::endl;

            std::cout<<"离线主机：";
            for(int i=0;i<off_machine.size();i++){
                std::cout<<off_machine[i]<<" ";
            }
            std::cout<<std::endl;
        }

        void OffMachine(const int& id)
        {
            for(auto it=on_machine.begin();it!=on_machine.end();it++){
                if(*it==id){
                    machines[id].ResetLoad();
                    on_machine.erase(it);
                    off_machine.push_back(id);
                    break;
                }
            }
        }
        void OnMachine()
        {
            mtx.lock();
            on_machine.insert(on_machine.end(),off_machine.begin(),off_machine.end());
            off_machine.erase(off_machine.begin(),off_machine.end());

            mtx.unlock();
        }
    };

    class Control
    {
    private:
        Model _model;
        View _view;
        LoadBalance _load_balance;

    public:
        Control() {}
        ~Control() {}
        // 获取所有题目，并返回一个网络页面
        // html：输出型
        bool AllQuestions(std::string *html)
        {
            std::vector<struct Question> q;

            if (_model.GetAllQuestion(&q))
            {
                _view.AllQuestionHtml(q, html);
            }
            else
            {
                *html = "获取题目失败, 形成题目列表失败";
                return false;
            }

            return true;
        }
        // 获取指定题目，并返回一个网络页面
        // number：输入型，题目编号
        // html：输出型
        bool OneQuestion(std::string number, std::string *html)
        {
            struct Question q;
            if (_model.GetOneQuestion(number, &q))
            {
                _view.OneQuestionHtml(q, html);
            }
            else
            {
                return false;
            }

            return true;
        }
        void Judge(std::string& number,const std::string& in_json, std::string *out_json)
        {
            // 0.根据number拿到用户提交题目的测试用例
            struct Question q;
            _model.GetOneQuestion(number, &q);
            // 1.in_json的反序列化，用户提交的代码、input
            Json::Reader reader;
            Json::Value in_value;
            reader.parse(in_json, in_value);

            // 2.代码拼接（用户的代码和测试用例代码进行拼接）
            Json::Value compile_value;
            std::string code = in_value["code"].asString();
            code+="\n";
            code += q.tail;

            compile_value["code"] = code;
            compile_value["input"] = in_value["input"].asString();
            compile_value["cpu_limit"] = q.cpu_limit;
            compile_value["mem_limit"] = q.mem_limit;

            Json::FastWriter writer;
            std::string compile_json;
            compile_json = writer.write(compile_value);

            // 3.选择负载最小主机
            int id = 0;
            Machine * machine = nullptr;
            while (true)
            {
                if (!_load_balance.SmartChoice(&id, &machine))
                {
                    // 全部服务均已离线
                    break;
                }
                else
                {
                    // 4.通过http请求服务，得到结果
                    machine->IncLoad();
                    Client client(machine->ip, machine->port);
                    if (auto res = client.Post("/compile_run",compile_json,"application/json;charset=utf-8"))
                    {
                        LOG(INFO) << "选择主机成功id：" << id << "  " << machine->ip << "：" << machine->port  << std::endl;

                        // 5.对out_json赋值
                        if(res->status==200){
                            *out_json = res->body;
                            machine->DecLoad();
                            break;
                        }
                        machine->DecLoad();

                    }
                    else
                    {
                        LOG(ERROR) << "指定主机id：" << id << "  " << machine->ip << "：" << machine->port << "可能已经离线" << std::endl;
                        _load_balance.OffMachine(id);
                        _load_balance.ShowMashine();
                    }
                }
            }
        }
        void RecoverMachine(){
            _load_balance.OnMachine();
        }
    };
}