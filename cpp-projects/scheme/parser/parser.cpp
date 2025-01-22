#include <parser.h>

Number::Number(int v) : value_(v) {
}

int Number::GetValue() const {
    return value_;
}

Symbol::Symbol(std::string_view s) : name_(s) {
}

const std::string& Symbol::GetName() const {
    return name_;
}

Cell::Cell(const std::shared_ptr<Object>& f, const std::shared_ptr<Object>& s)
    : head_(f), tail_(s) {
}

std::shared_ptr<Object> Cell::GetFirst() const {
    return head_;
}

std::shared_ptr<Object> Cell::GetSecond() const {
    return tail_;
}

std::shared_ptr<Object> Read(Tokenizer* tokenizer) {
    if (tokenizer->IsEnd()) {
        throw SyntaxError("empty line");
    }
    const Token& current = tokenizer->GetToken();
    tokenizer->Next();
    if (std::holds_alternative<BracketToken>(current)) {
        if (current == Token{BracketToken::OPEN}) {
            return ReadList(tokenizer);
        }
        throw SyntaxError("closing bracket doesn't match opening bracket");
    }
    if (std::holds_alternative<ConstantToken>(current)) {
        return std::make_shared<Number>(std::get<ConstantToken>(current).value);
    }
    if (std::holds_alternative<SymbolToken>(current)) {
        return std::make_shared<Symbol>(std::get<SymbolToken>(current).name);
    }
    throw SyntaxError("wrong token");
}
std::shared_ptr<Object> ReadList(Tokenizer* tokenizer) {
    if (tokenizer->GetToken() == Token{BracketToken::CLOSE}) {
        tokenizer->Next();
        return nullptr;
    }
    std::shared_ptr<Object> first = nullptr;
    std::shared_ptr<Object> second = nullptr;
    if (std::holds_alternative<DotToken>(tokenizer->GetToken())) {
        throw SyntaxError("empty first element in pair");
    }
    first = Read(tokenizer);
    if (tokenizer->IsEnd()) {
        throw SyntaxError("invalid list");
    }
    if (std::holds_alternative<DotToken>(tokenizer->GetToken())) {
        if (tokenizer->IsEnd()) {
            throw SyntaxError("invalid syntax");
        }
        tokenizer->Next();
        second = Read(tokenizer);
        if (tokenizer->IsEnd()) {
            throw SyntaxError("invalid list");
        }
        if (tokenizer->GetToken() != Token{BracketToken::CLOSE}) {
            throw SyntaxError("invalid list1");
        }
        if (!tokenizer->IsEnd()) {
            tokenizer->Next();
        }
        return std::make_shared<Cell>(first, second);
    }
    if (std::holds_alternative<SymbolToken>(tokenizer->GetToken()) ||
        std::holds_alternative<ConstantToken>(tokenizer->GetToken())) {
        second = ReadList(tokenizer);
    } else if (tokenizer->GetToken() == Token{BracketToken::CLOSE}) {
        if (!tokenizer->IsEnd()) {
            tokenizer->Next();
        }
    } else {
        second = Read(tokenizer);
    }
    tokenizer->Next();
    if (!tokenizer->IsEnd()) {
        tokenizer->Next();
    }
    return std::make_shared<Cell>(first, second);
}