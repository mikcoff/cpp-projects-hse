#include <tokenizer.h>

bool SymbolToken::operator==(const SymbolToken& other) const {
    return name == other.name;
}

bool QuoteToken::operator==(const QuoteToken&) const {
    return true;
}

bool DotToken::operator==(const DotToken&) const {
    return true;
}

bool ConstantToken::operator==(const ConstantToken& other) const {
    return value == other.value;
}

Token& Tokenizer::GetToken() {
    return current_token_;
}

bool Tokenizer::IsEnd() {
    while (stream_->peek() == ' ' || stream_->peek() == '\n') {
        stream_->get();
    }
    if (stream_->peek() == EOF) {
        is_end_ = true;
    }
    return is_end_;
}

void Tokenizer::Next() {
    while (stream_->peek() == ' ' || stream_->peek() == '\n') {
        stream_->get();
    }
    if (stream_->peek() == EOF) {
        is_end_ = true;
        return;
    }
    char c = stream_->get();
    if (c == '+' && std::isdigit(stream_->peek())) {
        c = stream_->get();
    }
    if (c == '\'') {
        current_token_ = QuoteToken();
        return;
    }
    if (c == '.') {
        current_token_ = DotToken();
        return;
    }
    if (c == '(') {
        current_token_ = BracketToken::OPEN;
        return;
    }
    if (c == ')') {
        current_token_ = BracketToken::CLOSE;
        return;
    }
    if ((c == '-' && std::isdigit(stream_->peek())) || std::isdigit(c)) {
        std::string num;
        num += c;
        while (std::isdigit(stream_->peek())) {
            c = stream_->get();
            num += c;
        }
        current_token_ = ConstantToken{std::stoi(num)};
        return;
    }
    if (c == '+' || c == '-') {
        current_token_ = SymbolToken{std::string(1, c)};
        return;
    }
    if (std::regex_match(std::string(1, c), std::regex("[a-zA-Z<=>*/#]"))) {
        std::string s;
        s += c;
        while (
            std::regex_match(std::string(1, stream_->peek()), std::regex("[a-zA-Z<=>*/#0-9?!-]"))) {
            c = stream_->get();
            s += c;
        }
        current_token_ = SymbolToken{s};
        return;
    }
    throw NameError("invalid char in istream");
}

Tokenizer::Tokenizer(std::istream* in) : stream_(in), is_end_(false) {
    Next();
}
