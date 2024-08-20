#pragma once

#include <unordered_map>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <assert.h>

#include "../comm/Log.hpp"
#include "../comm/util.hpp"

// 根据questions_list.cong文件，把题目列表加载到内存中
// model：跟数据进行交互，对外提供对数据的访问
namespace ns_model
{
    using namespace ns_log;
    using namespace ns_util;
    
    typedef struct Question
    {
        std::string number; // 题目编号
        std::string title;  // 题目标题
        std::string level;  // 题目难度：简单 中等 困难
        int cpu_limit;      // cpu资源上限（s）
        int mem_limit;      // 内存资源上限（kb）
        std::string desc;   // 题目描述
        std::string header; // 题目的代码框架
        std::string tail;   // 题目的测试用例
    } Question;

    std::string questions_list_path = "./questions/questions_list.conf";
    std::string question_path = "./questions/";
    
    class Model
    {
    private:
        std::unordered_map<std::string,struct Question> questions_list;

    public:
        Model()
        {
            assert(LoadAllQuestions(questions_list_path));
        }

      
        // 根据questions_list.conf文件挤在所有题目信息到内存中
        bool LoadAllQuestions(const std::string &questions_list_path)
        {
            std::fstream in(questions_list_path);
            if (!in.is_open())
            {
                LOG(FATAL) << "加载题库失败，请检查题库路径或题库是否存在" << "\n";
                return false;
            }
            std::string str;
            while (std::getline(in, str))
            {
                Question q;
                std::vector<std::string> tmp;
                StringUtil::SplitString(str, &tmp, " ");
                if (tmp.size() != 5)
                {
                    continue;
                }
                q.number = tmp[0];
                q.title = tmp[1];
                q.level = tmp[2];
                q.cpu_limit = atoi(tmp[3].c_str());
                q.mem_limit = atoi(tmp[4].c_str());

                std::string path = question_path + q.number;
                path += "/";
                FileUtil::ReadFile(path + "desc.txt", &q.desc, true);
                FileUtil::ReadFile(path + "header.cpp", &q.header, true);
                FileUtil::ReadFile(path + "tail.cpp", &q.tail, true);

                questions_list.insert({q.number, q});
            }
            LOG(INFO) << "加载题库成功,题目数量："<<questions_list.size() << "\n";
            in.close();

            return true;
        }

        bool GetAllQuestion(std::vector<struct Question> *vq)
        {
            if (questions_list.size() == 0)
            {
                // 按照题目编号对题目进行排序
                sort(vq->begin(),vq->end(),[](const struct Question& q1 ,const struct Question& q2){
                    return atoi(q1.number.c_str())<atoi(q2.number.c_str());
                });

                LOG(ERROR) << "用户加载题库失败" << "\n";
                return false;
            }
            for (const auto & q : questions_list)
            {
                vq->push_back(q.second);
            }
            return true;
        }

        bool GetOneQuestion(std::string& number, Question *q)
        {
            const auto &iter = questions_list.find(number);
            if (iter == questions_list.end())
            {
                LOG(ERROR) << "用户加载题目失败，题目编号：" << number << "\n";
                return false;
            }
            *q = iter->second;
            return true;
        }

        ~Model() {}
    };
}
