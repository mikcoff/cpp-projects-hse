#include "string_operations.h"
#include <string>
#include <string_view>

bool StartsWith(std::string_view string, std::string_view text) {
    if (!text.empty() || (text.empty() && string.empty())) {
        return string.substr(0, text.size()) == text;
    }
    return false;
}

bool EndsWith(std::string_view string, std::string_view text) {
    if (!text.empty() || (text.empty() && string.empty())) {
        if (text.size() > string.size()) {
            return false;
        }
        return string.substr(string.size() - text.size(), text.size()) == text;
    }
    return false;
}

std::string_view StripPrefix(std::string_view string, std::string_view prefix) {
    if (!StartsWith(string, prefix)) {
        return string;
    }
    return string.substr(prefix.size(), string.size() - prefix.size());
}

std::string_view StripSuffix(std::string_view string, std::string_view suffix) {
    if (!EndsWith(string, suffix)) {
        return string;
    }
    return string.substr(0, string.size() - suffix.size());
}

std::string_view ClippedSubstr(std::string_view s, size_t pos, size_t n) {
    if (n > s.size()) {
        return s;
    }
    return s.substr(pos, n);
}

std::string_view StripAsciiWhitespace(std::string_view string) {
    size_t i = 0;
    size_t j = string.size() - 1;
    while (i < string.size() && std::isspace(string[i])) {
        ++i;
    }
    while (j > i && std::isspace(string[j - 1])) {
        --j;
    }
    return string.substr(i, j - i);
}

std::vector<std::string_view> StrSplit(std::string_view text, std::string_view delim) {
    size_t n = 0;
    size_t pos = 0;
    while ((pos = text.find(delim, pos)) != std::string::npos) {
        ++n;
        pos += delim.size();
    }
    std::vector<std::string_view> res(n + 1);
    size_t i = 0;
    pos = 0;
    size_t counter = 0;
    while ((pos = text.find(delim, pos)) != std::string_view::npos) {
        res[counter] = text.substr(i, pos - i);
        ++counter;
        pos += delim.size();
        i = pos;
    }
    res[counter] = text.substr(i, pos - i);
    return res;
}

std::string ReadN(const std::string& filename, size_t n) {
    std::string buf(n, '\0');
    int fd = open(filename.c_str(), O_RDONLY);
    if (fd == -1) {
        return "";
    }
    read(fd, &buf[0], n);
    close(fd);
    return buf;
}

std::string AddSlash(std::string_view path) {
    if (path.empty()) {
        return std::string(path);
    }
    if (path.back() == '/') {
        return std::string(path);
    }
    std::string res;
    res.reserve(path.size() + 1);
    res += path;
    res += "/";
    return res;
}

std::string_view RemoveSlash(std::string_view path) {
    if (path.empty()) {
        return path;
    }
    if (path.back() != '/' || path == "/") {
        return path;
    }
    return path.substr(0, path.size() - 1);
}

std::string_view Dirname(std::string_view path) {
    size_t ind = path.rfind('/');
    if (ind == 0) {
        return path.substr(0, 1);
    }
    return path.substr(0, ind);
}

std::string_view Basename(std::string_view path) {
    size_t ind = path.rfind('/');
    return path.substr(ind + 1, path.size());
}

std::string CollapseSlashes(std::string_view path) {
    if (path.empty()) {
        return "";
    }
    size_t count = 0;
    bool slash = false;
    for (const auto& ch : path) {
        if (ch == '/') {
            if (!slash) {
                slash = true;
                ++count;
            }
        } else {
            slash = false;
        }
    }
    if (count == 1) {
        return "/";
    }
    std::string res;
    res.reserve(path.size());
    slash = false;
    for (const auto& ch : path) {
        if (ch == '/') {
            if (!slash) {
                slash = true;
                res += ch;
            }
        } else {
            slash = false;
            res += ch;
        }
    }
    return res;
}

std::string StrJoin(const std::vector<std::string_view>& strings, std::string_view delimiter) {
    if (strings.empty()) {
        return "";
    }
    size_t size = 0;
    for (const auto& el : strings) {
        size += el.size() + delimiter.size();
    }
    std::string res;
    res.reserve(size);
    for (size_t i = 0; i < strings.size() - 1; ++i) {
        res += strings[i];
        res += delimiter;
    }
    res += strings[strings.size() - 1];
    return res;
}
