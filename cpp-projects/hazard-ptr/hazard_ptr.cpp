#include "hazard_ptr.h"
#include <vector>

thread_local std::atomic<void*> hazard_ptr{nullptr};
std::mutex threads_lock;
std::atomic<RetiredPtr*> free_list{nullptr};
std::atomic<int> approximate_free_list_size{0};
std::mutex scan_lock;
std::unordered_set<ThreadState*> threads;

void ScanFreeList() {
    approximate_free_list_size.store(0);

    std::unique_lock scan_guard(scan_lock);
    if (!scan_guard.owns_lock()) {
        return;
    }

    RetiredPtr* retired = free_list.exchange(nullptr);

    std::vector<void*> hazard;
    {
        std::unique_lock guard(threads_lock);
        for (const auto& thread : threads) {
            if (auto ptr = thread->ptr->load(); ptr) {
                hazard.push_back(ptr);
            }
        }
    }

    std::sort(hazard.begin(), hazard.end());
    RetiredPtr* current = retired;
    RetiredPtr* next = nullptr;
    while (current != nullptr) {
        next = current->next;
        if (!std::binary_search(hazard.begin(), hazard.end(), current->value)) {
            current->deleter();
            delete current;
        } else {
            approximate_free_list_size.fetch_add(1);
            auto expected = free_list.load();
            do {
                current->next = expected;
            } while (!free_list.compare_exchange_weak(expected, current));
        }
        current = next;
    }
}
