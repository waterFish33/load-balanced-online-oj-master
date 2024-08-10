#include <string>

#include "compiler.hpp"
#include "runner.hpp"

using namespace ns_compiler;
using namespace ns_runner;

int main(){
    std::string filename="test";
    Compiler::compile(filename);
    Runner::Run(filename);
    return 0;
}