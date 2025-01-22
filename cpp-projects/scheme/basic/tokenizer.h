#pragma once

#include <variant>
#include <optional>
#include <istream>
#include <cctype>
#include <cstdio>
#include <string>
#include <regex>

struct SymbolToken {
    std::string name;

    bool operator==(const SymbolToken& other) const;
};

struct QuoteToken {
    bool operator==(const QuoteToken&) const;
};

struct DotToken {
    bool operator==(const DotToken&) const;
};

enum class BracketToken { OPEN, CLOSE };

struct ConstantToken {
    int value;

    bool operator==(const ConstantToken& other) const;
};

using Token = std::variant<ConstantToken, BracketToken, SymbolToken, QuoteToken, DotToken>;

class Tokenizer {
public:
    Tokenizer(std::istream* in);

    bool IsEnd();

    void Next();

    Token GetToken();

private:
    std::istream* stream_;
    Token current_token_;
    bool is_end_;
    std::regex symbol_start_ = std::regex("[a-zA-Z<=>*/#]");
    std::regex symbol_reg_ = std::regex("[a-zA-Z<=>*/#0-9?!-]");
};