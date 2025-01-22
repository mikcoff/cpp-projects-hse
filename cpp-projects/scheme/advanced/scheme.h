#pragma once
#include <memory>
#include "object.h"

#include "functions.h"
#include "parser.h"
#include <sstream>

class Interpreter {
public:
    Interpreter();
    std::string Run(const std::string& input);

private:
    std::shared_ptr<Scope> scope_;
};
