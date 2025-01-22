#include "scheme.h"
#include <memory>
#include <iostream>
#include "object.h"

Interpreter::Interpreter() : scope_(std::make_shared<Scope>()) {
}

std::string Interpreter::Run(const std::string& input) {
    std::stringstream ss(input);
    Tokenizer tokenizer(&ss);
    std::shared_ptr<Object> ast = Read(&tokenizer);
    if (!ast) {
        throw RuntimeError("empty ast");
    }
    // std::cout << ast->Serialize() << std::endl;
    ast->SetScope(scope_);
    if (Is<Cell>(ast)) {
        As<Cell>(ast)->ShareScope();
    }
    std::shared_ptr<Object> res = ast->Evaluate();
    if (!res) {
        return "()";
    }
    return res->Serialize();
}
