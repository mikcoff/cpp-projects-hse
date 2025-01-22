#include <parser.h>

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
        return std::make_shared<Symbol>(std::string(std::get<SymbolToken>(current).name));
    }
    if (std::holds_alternative<QuoteToken>(current)) {
        return std::make_shared<Cell>(std::make_shared<Symbol>(std::string("quote")),
                                      Read(tokenizer));
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
    if (tokenizer->GetToken() == Token{BracketToken::CLOSE}) {
        tokenizer->Next();
        return std::make_shared<Cell>(first, nullptr);
    }
    second = ReadList(tokenizer);
    return std::make_shared<Cell>(first, second);
}