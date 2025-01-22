#pragma once

#include <atomic>
#include <unordered_map>
#include <memory>
#include <mutex>

#include "hazard_ptr.h"

const int kMAX = 100;

template <class K, class V>
class SyncMap {
public:
    bool Lookup(const K& key, V* value) {
        auto hzrd_ptr = Acquire(&snapshot_);
        if (!hzrd_ptr->dirty) {
            std::lock_guard<std::mutex> lc(lock_);
            if (!mutable_map_->contains(key)) {
                Release();
                return false;
            }
            ++operation_count_;
            *value = (*mutable_map_)[key];
            if (operation_count_ > kMAX) {
                Retire(snapshot_.exchange(new Snapshot{mutable_map_, true}));
                operation_count_ = 0;
            }
            Release();
            return true;
        }
        if (!hzrd_ptr->read_only->contains(key)) {
            Release();
            return false;
        }
        *value = (*(hzrd_ptr->read_only))[key];
        Release();
        return true;
    }

    bool Insert(const K& key, const V& value) {
        std::lock_guard<std::mutex> lc(lock_);
        if (mutable_map_->contains(key) && (*mutable_map_)[key] == value) {
            return false;
        }

        auto hzrd_ptr = Acquire(&snapshot_);
        Retire(snapshot_.exchange(new Snapshot{hzrd_ptr->read_only, false}));
        Release();
        (*mutable_map_)[key] = value;
        return true;
    }

    SyncMap()
        : snapshot_(new Snapshot{std::make_shared<std::unordered_map<K, V>>()}),
          mutable_map_(std::make_shared<std::unordered_map<K, V>>()) {
    }

    ~SyncMap() {
        Retire(snapshot_.load());
        ScanFreeList();
    }

private:
    struct Snapshot {
        std::shared_ptr<std::unordered_map<K, V>> read_only;

        // Indicates that read_only snapshot may be incomplete and lookup should take lock.
        bool dirty = false;
    };

    std::atomic<Snapshot*> snapshot_;

    std::mutex lock_;
    std::shared_ptr<std::unordered_map<K, V>> mutable_map_;
    int operation_count_ = 0;
};
