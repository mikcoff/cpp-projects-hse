#pragma once

#include <algorithm>
#include <limits>
#include <vector>
#include <string>

class Dictionary {
public:
    Dictionary();

    void Clear();

    void LoadFromFile(const std::string& path);

    bool IsLoaded() const;

    const std::string* FindClosest(const std::string& word, int& best_distance) const;

private:
    std::vector<std::string> words_;
    std::vector<std::string> words_lower_;
    mutable std::vector<int> dp_prev_;
    mutable std::vector<int> dp_cur_;
    bool loaded_;

    static std::string ToLowerCopy(const std::string& s) {
        std::string result = s;
        std::transform(result.begin(), result.end(), result.begin(),
                       [](const unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
        return result;
    }

    static bool CanBeatCurrentBest(const int word_len, const int cand_len, const int best) {
        if (best == std::numeric_limits<int>::max()) {
            return true;
        }
        const int len_diff = cand_len - word_len;
        const int abs_len_diff = len_diff >= 0 ? len_diff : -len_diff;
        return abs_len_diff < best;
    }

    int LevenshteinInternal(const std::string_view a, const std::string_view b) const;
};
