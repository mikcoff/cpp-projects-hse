#pragma once
#define SCHEME_FUZZING_1_PRINT_REQUESTS
#define SCHEME_FUZZING_2_PRINT_REQUESTS

#include "functions.h"
#include "parser.h"
#include <sstream>

class Interpreter {
public:
    Interpreter();
    std::string Run(const std::string& input);
};
