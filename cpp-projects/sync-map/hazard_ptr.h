#pragma once

#include <algorithm>
#include <atomic>
#include <functional>
#include <mutex>
#include <memory>
#include <unordered_set>

extern thread_local std::atomic<void*> hazard_ptr;

struct ThreadState {
    std::atomic<void*>* ptr;
};

extern std::mutex threads_lock;
extern std::unordered_set<ThreadState*> threads;

template <class T>
T* Acquire(std::atomic<T*>* ptr) {

    auto value = ptr->load();
    do {
        hazard_ptr.store(value);
        auto new_value = ptr->load();
        if (new_value == value) {
            return value;
        }
        value = new_value;
    } while (true);
}

inline void Release() {
    hazard_ptr.store(nullptr);
}
struct RetiredPtr {
    void* value;
    std::function<void()> deleter;
    RetiredPtr* next;
};

extern std::atomic<RetiredPtr*> free_list;
extern std::atomic<int> approximate_free_list_size;
const int kN = 1000;

extern std::mutex scan_lock;

void ScanFreeList();

template <class T, class Deleter = std::default_delete<T>>
void Retire(T* value, Deleter deleter = {}) {
    // 1) Add ptr to free list.
    RetiredPtr* retired = new RetiredPtr{value, [deleter, value]() { deleter(value); }, nullptr};
    auto expected = free_list.load();
    do {
        retired->next = expected;
    } while (!free_list.compare_exchange_weak(expected, retired));
    // 2) Increment free list size.
    approximate_free_list_size.fetch_add(1);
    // 3) Scan free list, if size > K.
    if (approximate_free_list_size.load() > kN) {
        ScanFreeList();
    }
}

inline void RegisterThread() {
    std::lock_guard<std::mutex> lock(threads_lock);
    threads.insert(new ThreadState{&hazard_ptr});
}

inline void UnregisterThread() {
    std::lock_guard<std::mutex> lock(threads_lock);
    hazard_ptr.store(nullptr);
    auto it = std::find_if(threads.begin(), threads.end(),
                           [](ThreadState* state) { return state->ptr == &hazard_ptr; });
    if (it != threads.end()) {
        ThreadState* state = *it;
        threads.erase(it);
        delete state;
    }
}
