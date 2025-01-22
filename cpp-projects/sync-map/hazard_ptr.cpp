#include "hazard_ptr.h"
#include <vector>

thread_local std::atomic<void*> hazard_ptr{nullptr};
std::mutex threads_lock;
std::atomic<RetiredPtr*> free_list{nullptr};
std::atomic<int> approximate_free_list_size{0};
std::mutex scan_lock;
std::unordered_set<ThreadState*> threads;

void ScanFreeList() {
    // (0) Обнуляем approximate_free_list_size, чтобы другие потоки не пытались зайти в ScanFreeList
    // вместе с нами.
    approximate_free_list_size.store(0);

    // (1) С помощью мьютекса убеждаемся, что не больше одного потока занимается сканированием.
    // В реальном коде не забудьте использовать guard.
    std::unique_lock scan_guard(scan_lock);
    if (!scan_guard.owns_lock()) {
        return;
    }

    // (2) Забираем все указатели из free_list
    RetiredPtr* retired = free_list.exchange(nullptr);

    // (3) Читаем множество защищённых указателей обойдя все ThreadState.
    std::vector<void*> hazard;
    {
        std::unique_lock guard(threads_lock);
        for (const auto& thread : threads) {
            if (auto ptr = thread->ptr->load(); ptr) {
                hazard.push_back(ptr);
            }
        }
    }

    // (4) Сканируем все retired указатели.
    std::sort(hazard.begin(), hazard.end(),
              [](void* lhs, void* rhs) { return std::less<void*>{}(lhs, rhs); });
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
    //    (a) Для тех, что не находятся в hazard, вызываем деструктор и освобождаем память под
    //    RetiredPtr. (b) Те, что еще находятся в hazard, кладём назад в free_list.
    //
    // Для ускорения, hazard нужно посортировать и использовать бинарный поиск.
}
