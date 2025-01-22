#include "scheme.h"

Interpreter::Interpreter() {
    RegisterFunctions();
}

std::string Interpreter::Run(const std::string& input) {
    std::stringstream ss(input);
    Tokenizer tokenizer(&ss);
    std::shared_ptr<Object> ast = Read(&tokenizer);
    if (!ast) {
        throw RuntimeError("empty ast");
    }
    std::shared_ptr<Object> res = ast->Evaluate();
    if (!res) {
        return "()";
    }
    return res->Serialize();
}
