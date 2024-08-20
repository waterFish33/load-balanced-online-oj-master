#include <iostream>
#include <string>
#include <signal.h>

#include "../comm/httplib.h"
#include "./oj_control.hpp"

using namespace httplib;
using namespace ns_control;

static Control* cntl_ptr=nullptr;
void Recovery(int signo)
{
    cntl_ptr->RecoverMachine();
}
int main()
{
  
  // 用户请求的路由功能
  Server server;
  Control cntl;
  cntl_ptr=&cntl;
  signal(SIGQUIT,Recovery);
  // 获取所有题目
  server.Get("/questions/all", [&cntl](const Request &res, Response &resp)
              { 
                std::string html;
                cntl.AllQuestions(&html);
                resp.set_content(html, "text/html;charset=utf-8"); });

  // 获取指定题目编号的题目
  server.Get(R"(/questions/(\d+))", [&cntl](const Request &res, Response &resp)
              {
                 std::string html;
                 std::string number = res.matches[1];
                 cntl.OneQuestion(number, &html);

                 resp.set_content(html, "text/html;charset=utf-8"); });

  // 对指定的题目进行判题（1.测试用例 2.compile_run）
  server.Post(R"(/judge/(\d+))", [&cntl](const Request &res, Response &resp)
              {
                    std::string number=res.matches[1];
                    std::string result_json;
                    cntl.Judge(number,res.body,&result_json);
                    
                  resp.set_content(result_json,"application/json;charset=utf-8"); });

  server.set_base_dir("./wwwroot");

  server.listen("0.0.0.0", 10086);

  return 0;
}