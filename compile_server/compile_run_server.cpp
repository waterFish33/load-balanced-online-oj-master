#include <string>

#include "../comm/httplib.h"
#include "compile_server.hpp"

using namespace ns_compile_server;
using namespace httplib;

void Usage(){
    std::cerr<<"please enter: ./compile_run_server\tport"<<std::endl;
}
int main(int argc,char** argv){
    if(argc!=2){
        Usage();
    }
    Server server;

    // server.Get("/hello",[](const Request& res,Response& resp){
    //     resp.set_content("你好哦,httplib","content-type:text/plain;charset:utf-8");
    // });

    server.Post("/compile_run",[](const Request& res,Response& resp){
        std::string in_josn=res.body;
        std::string out_json;
        if(!in_josn.empty()){
            compile_server::Start(in_josn,&out_json);
        }
        resp.set_content(out_json,"application/json;charset=utf-8");
    });
    server.listen("0.0.0.0",atoi(argv[1]));



    return 0;
}