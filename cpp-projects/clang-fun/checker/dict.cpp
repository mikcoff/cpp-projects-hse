#include "dict.h"
#include <fstream>

Dictionary::Dictionary() : loaded_(false) {
}

void Dictionary::Clear() {
    words_.clear();
    words_lower_.clear();
    dp_prev_.clear();
    dp_cur_.clear();
    loaded_ = false;
}

void Dictionary::LoadFromFile(const std::string& path) {
    Clear();

    std::ifstream in(path);
    if (!in) {
        return;
    }

    std::string word;
    while (in >> word) {
        words_lower_.push_back(ToLowerCopy(word));
        words_.push_back(std::move(word));
    }

    loaded_ = true;
}

bool Dictionary::IsLoaded() const {
    return loaded_;
}

const std::string* Dictionary::FindClosest(const std::string& word, int& best_distance) const {
    if (!loaded_ || words_.empty()) {
        best_distance = -1;
        return nullptr;
    }

    const std::string word_lower = ToLowerCopy(word);
    const int word_len = static_cast<int>(word_lower.size());

    const std::string* best_word = nullptr;
    int best = std::numeric_limits<int>::max();

    const size_t dict_size = words_.size();

    for (size_t i = 0; i < dict_size; ++i) {
        const auto& candidate = words_[i];
        if (candidate.empty()) {
            continue;
        }

        const auto& candidate_lower = words_lower_[i];
        const int cand_len = static_cast<int>(candidate_lower.size());
        if (!CanBeatCurrentBest(word_len, cand_len, best)) {
            continue;
        }

        const int d = LevenshteinInternal(word_lower, candidate_lower);
        if (d < best) {
            best = d;
            best_word = &candidate;
        }
    }

    if (!best_word || best == std::numeric_limits<int>::max()) {
        best_distance = -1;
        return nullptr;
    }

    best_distance = best;
    return best_word;
}

int Dictionary::LevenshteinInternal(const std::string_view a, const std::string_view b) const {
    const size_t n = a.size();
    const size_t m = b.size();

    if (n == 0) {
        return static_cast<int>(m);
    }
    if (m == 0) {
        return static_cast<int>(n);
    }

    if (dp_prev_.size() < m + 1) {
        dp_prev_.resize(m + 1);
        dp_cur_.resize(m + 1);
    }

    int* prev = dp_prev_.data();
    int* cur = dp_cur_.data();

    for (size_t j = 0; j <= m; ++j) {
        prev[j] = static_cast<int>(j);
    }

    for (size_t i = 1; i <= n; ++i) {
        cur[0] = static_cast<int>(i);
        for (size_t j = 1; j <= m; ++j) {
            const int cost = (a[i - 1] == b[j - 1]) ? 0 : 1;
            const int del = prev[j] + 1;
            const int ins = cur[j - 1] + 1;
            const int sub = prev[j - 1] + cost;

            int best = del < ins ? del : ins;
            if (sub < best) {
                best = sub;
            }
            cur[j] = best;
        }
        std::swap(prev, cur);
    }

    return prev[m];
}
