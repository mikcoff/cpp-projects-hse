#pragma once
#include <cstring>
#include <format>
#include <iterator>
#include <string_view>
#include <string>
#include <vector>
#include <sys/fcntl.h>
#include <cstdint>
#include <cstdio>
#include <cstddef>
#include <limits>
#include <cmath>
#include <type_traits>
#include <sys/unistd.h>
#include <unistd.h>

/*
`bool StartsWith(STR string, STR text)` — проверяет, что строка `string` начинается с `text`.

`bool EndsWith(STR string, STR text)` — проверяет, что строка `string` оканчивается на `text`.

`STR StripPrefix(STR string, STR prefix)` — возвращает `string` с убранным `prefix`,
если `string` не начинается на `prefix`, возвращает `string`.

`STR StripSuffix(STR string, STR suffix)` — тоже самое, но с суффиксом.

`STR ClippedSubstr(STR s, size_t pos, size_t n = STR::npos)` — тоже самое, что и `s.substr(pos, n)`,
но если `n` больше `s.size()`, то возвращается `s`.

`STR StripAsciiWhitespace(STR)` — `strip` строки, удаляем все символы с обоих концов
вида [isspace](https://en.cppreference.com/w/cpp/string/byte/isspace).

`std::vector<STR> StrSplit(STR text, STR delim)` — делаем `split` строки по `delim`. Подумайте,
прежде чем копипастить из уже имеющейся задачи. Обойдитесь одной аллокацией памяти.

`STR ReadN(STR filename, int n)` — открывает файл и читает `n` байт из filename. Используйте Linux
Syscalls `open`, `read`, `close`. Если открыть или прочитать файл нельзя, возвращает пустую строчку.

`STR AddSlash(STR path)` — добавляет к `path` файловой системы символ `/`, если его не было.

`STR RemoveSlash(STR path)` — убирает `/` из `path`, если это не сам путь `/` и путь заканчивается
на `/`.

`STR Dirname(STR path)` — известно, что `path` — корректный путь до файла без слеша на конце,
верните папку, в которой этот файл лежит без слеша на конце, если это не корень.

`STR Basename(STR path)` — известно, что `path` — корректный путь до файла, верните его название.

`STR CollapseSlashes(STR path)` — известно, что `path` — корректный путь, но `/` могут повторяться,
надо убрать все повторения.

`STR StrJoin(const std::vector<STR>& strings, STR delimiter)` — склеить все строки в одну через
`delimiter`. Обойдитесь одной аллокацией памяти.

`STR StrCat(Args...)` — склеить все аргументы в один в их строковом представлении.
Должны поддерживаться числа (`int, long, long long` и их `unsigned` версии), также все строковые
типы (`std::string, std::string_view, const char*`). Аргументов в `StrCat` не больше пяти.
Придумайте как это сделать за одну аллокацию памяти.
*/

bool StartsWith(std::string_view string, std::string_view text);

bool EndsWith(std::string_view string, std::string_view text);

std::string_view StripPrefix(std::string_view string, std::string_view prefix);

std::string_view StripSuffix(std::string_view string, std::string_view suffix);

std::string_view ClippedSubstr(std::string_view s, size_t pos, size_t n = std::string_view::npos);

std::string_view StripAsciiWhitespace(std::string_view string);

std::vector<std::string_view> StrSplit(std::string_view text, std::string_view delim);

std::string ReadN(const std::string& filename, size_t n);

std::string AddSlash(std::string_view path);

std::string_view RemoveSlash(std::string_view path);

std::string_view Dirname(std::string_view path);

std::string_view Basename(std::string_view path);

std::string CollapseSlashes(std::string_view path);

std::string StrJoin(const std::vector<std::string_view>& strings, std::string_view delimiter);

template <typename... Args>
std::string StrCat(const Args&... args) {
    size_t size = 0;
    (
        [&] {
            if constexpr (std::is_integral_v<std::decay_t<Args>> ||
                          std::is_floating_point_v<std::decay_t<Args>>) {
                size += static_cast<size_t>(log10(std::numeric_limits<uint64_t>::max())) + 1;
            } else if constexpr (std::is_same_v<std::decay_t<Args>, std::string> ||
                                 std::is_same_v<std::decay_t<Args>, std::string_view>) {
                size += args.length();
            } else if constexpr (std::is_same_v<std::decay_t<Args>, const char*>) {
                size += std::strlen(args);
            }
        }(),
        ...);
    std::string res;
    res.reserve(size + 1);
    (
        [&] {
            if constexpr (std::is_integral_v<std::decay_t<Args>> ||
                          std::is_floating_point_v<std::decay_t<Args>>) {
                std::format_to(std::back_inserter(res), "{}", args);
            } else {
                res += args;
            }
        }(),
        ...);
    return res;
}
