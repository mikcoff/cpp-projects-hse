#pragma once
#define SCHEME_FUZZING_1_PRINT_REQUESTS
#include <memory>

#include "object.h"
#include <tokenizer.h>
#include "error.h"

template <typename... Ts>
struct Overloaded : Ts... {
    using Ts::operator()...;
};
template <typename... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;

std::shared_ptr<Object> Read(Tokenizer* tokenizer);

std::shared_ptr<Object> ReadList(Tokenizer* tokenizer);