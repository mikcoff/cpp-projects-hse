#pragma once

#include <algorithm>
#include <cstddef>
#include <list>
#include <stdexcept>
#include <mutex>
#include <functional>
#include <utility>
#include <vector>
#include <atomic>

template <class K, class V, class Hash = std::hash<K>>
class ConcurrentHashMap {
public:
    ConcurrentHashMap(const Hash& hasher = Hash()) : ConcurrentHashMap(kUndefinedSize, hasher) {
    }

    explicit ConcurrentHashMap(size_t expected_size, const Hash& hasher = Hash())
        : ConcurrentHashMap(expected_size, kDefaultConcurrencyLevel, hasher) {
    }

    ConcurrentHashMap(size_t expected_size, size_t expected_threads_count,
                      const Hash& hasher = Hash())
        : hasher_(hasher), size_(0), table_(expected_size), mutexes_(expected_size) {
        (void)expected_threads_count;
    }

    bool Insert(const K& key, const V& value) {
        std::unique_lock<std::mutex> lock(mutexes_[hasher_(key) % mutexes_.size()]);
        size_t pos = hasher_(key) % table_.size();
        for (const auto& [k, v] : table_[pos]) {
            if (k == key) {
                return false;
            }
        }
        table_[pos].push_back(std::make_pair(key, value));
        size_.fetch_add(1);
        if (size_.load() / table_.size() > 4) {
            lock.unlock();
            {
                std::lock_guard<std::mutex> lock_in(insert_mtx_);
                Rehash();
            }
        }
        return true;
    }

    bool Erase(const K& key) {
        std::lock_guard<std::mutex> lock(mutexes_[hasher_(key) % mutexes_.size()]);
        size_t pos = hasher_(key) % table_.size();
        for (const auto& [k, v] : table_[pos]) {
            if (k == key) {
                table_[pos].erase(
                    std::find(table_[pos].begin(), table_[pos].end(), std::pair<K, V>{k, v}));
                size_.fetch_sub(1);
                return true;
            }
        }
        return false;
    }

    void Clear() {
        std::vector<std::unique_lock<std::mutex>> locked;
        for (auto& mtx : mutexes_) {
            locked.emplace_back(mtx);
        }
        for (auto& bucket : table_) {
            bucket.clear();
        }
        size_.store(0);
    }

    std::pair<bool, V> Find(const K& key) const {
        std::lock_guard<std::mutex> lock(mutexes_[hasher_(key) % mutexes_.size()]);
        size_t pos = hasher_(key) % table_.size();
        for (const auto& [k, v] : table_[pos]) {
            if (k == key) {
                return std::make_pair(true, v);
            }
        }
        return std::make_pair(false, V());
    }

    const V At(const K& key) const {
        std::lock_guard<std::mutex> lock(mutexes_[hasher_(key) % mutexes_.size()]);
        size_t pos = hasher_(key) % table_.size();
        for (const auto& [k, v] : table_[pos]) {
            if (k == key) {
                return v;
            }
        }
        throw std::out_of_range("key not in map");
    }

    size_t Size() const {
        return size_.load();
    }

    static const size_t kDefaultConcurrencyLevel;
    static const size_t kUndefinedSize;

private:
    Hash hasher_;
    std::atomic<size_t> size_;
    std::vector<std::list<std::pair<K, V>>> table_;
    mutable std::vector<std::mutex> mutexes_;
    mutable std::mutex insert_mtx_;

    void Rehash() {
        size_t expected = table_.size();
        std::vector<std::unique_lock<std::mutex>> locked;
        for (auto& mtx : mutexes_) {
            locked.emplace_back(mtx);
        }
        if (expected != table_.size()) {
            return;
        }
        size_t new_table_size = 3 * expected;
        std::vector<std::list<std::pair<K, V>>> new_table(new_table_size);
        for (const auto& bucket : table_) {
            for (const auto& [k, v] : bucket) {
                new_table[hasher_(k) % new_table_size].push_back(std::make_pair(k, v));
            }
        }
        std::swap(table_, new_table);
    }
};

template <class K, class V, class Hash>
const size_t ConcurrentHashMap<K, V, Hash>::kDefaultConcurrencyLevel = 8;

template <class K, class V, class Hash>
const size_t ConcurrentHashMap<K, V, Hash>::kUndefinedSize = 17;
