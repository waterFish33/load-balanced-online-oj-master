#pragma once

#include <unordered_map>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <mysql/mysql.h>

#include "../comm/Log.hpp"
#include "../comm/util.hpp"

// 根据questions_list.cong文件，把题目列表加载到内存中
// model：跟数据进行交互，对外提供对数据的访问
namespace ns_model
{
    using namespace ns_log;
    using namespace ns_util;

    struct Question
    {
        std::string number; // 题目编号
        std::string title;  // 题目标题
        std::string level;  // 题目难度：简单 中等 困难
        int cpu_limit;      // cpu资源上限（s）
        int mem_limit;      // 内存资源上限（kb）
        std::string desc;   // 题目描述
        std::string header; // 题目的代码框架
        std::string tail;   // 题目的测试用例
    };

    const std::string table_name = "questions";
    const std::string host = "127.0.0.1";
    const std::string user = "oj_client";
    const std::string passwd = "Wangxijun111!";
    const std::string database = "oj";

    class Model
    {
    public:
        // 返回1则是失败
        bool QueryMySQL(const std::string &sql, std::vector<struct Question> *vq)
        {
            // 连接数据库
            MYSQL *mysql = nullptr;
            mysql = mysql_init(NULL);
            if (!mysql_real_connect(mysql, host.c_str(), user.c_str(), passwd.c_str(), database.c_str(), 0, NULL, 0))
            {
                LOG(FATAL) << "连接数据库失败" << std::endl;
                return 1;
            }
            mysql_set_character_set(mysql,"utf8");
            // 执行sql语句
            if (mysql_query(mysql, sql.c_str()))
            {
                LOG(ERROR) << "执行sql语句失败" << std::endl;
                return 1;
            }

            // 获取结果
            MYSQL_RES *res = mysql_store_result(mysql);
            int rows = mysql_num_rows(res); // 获取行数

            // 查询结果
            for (int i = 0; i < rows; i++)
            {
                MYSQL_ROW row = mysql_fetch_row(res);
                struct Question tmp;
                tmp.number = row[0];
                tmp.title = row[1];
                tmp.level = row[2];
                tmp.desc = row[3];
                tmp.header = row[4];
                tmp.tail = row[5];
                tmp.cpu_limit = atoi(row[6]);
                tmp.mem_limit = atoi(row[7]);
                vq->push_back(tmp);
            }
            // 释放结果
            mysql_free_result(res);
            // 关闭连接
            mysql_close(mysql);
            return true;
        }
        bool GetAllQuestion(std::vector<struct Question> *vq)
        {
            std::string sql = "select * from ";
            sql += table_name;
            sql += ";";
            return QueryMySQL(sql, vq);
        }

        bool GetOneQuestion(const std::string &number, Question *q)
        {
            std::string sql = "select * from ";
            sql += table_name;
            sql += " where number=";
            sql += number;
            sql += ";";
            std::vector<struct Question> vq;
            QueryMySQL(sql, &vq);
            if (vq.size() == 1)
            { // 因为number在MySQL是主键所以正常只有一个值
                *q = vq[0];
            }
            else
            {
                return false;
            }
            return true;
        }

        ~Model() {}
    };
}
