#include <string>

#include "compile_server.hpp"

using namespace ns_compile_server;

int main(){
    Json::Value in_value;
    Json::StyledWriter writer;

    in_value["code"]=R"(#include <iostream>
        int main(){
            std::cout<<"测试程序"<<std::endl;
            return 0;
        })";
    in_value["input"]="";
    in_value["cpu_limit"]=1;
    in_value["mem_limit"]=1024*200;
    std::string in_string=writer.write(in_value);

    std::string out_string;

    compile_server::Start(in_string,&out_string);

    std::cout<<out_string<<std::endl;
    return 0;
}