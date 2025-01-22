#pragma once

#include <condition_variable>
#include <mutex>
#include <optional>
#include <stdexcept>

template <class T>
class UnbufferedChannel {
public:
    void Send(const T& value) {
        std::unique_lock<std::mutex> lc(mtx_);
        cv_send_.wait(lc, [this] { return wait_ || closed_; });
        if (closed_) {
            throw std::runtime_error("channel is closed");
        }
        value_ = value;
        wait_ = false;
        cv_recv_.notify_one();
    }

    std::optional<T> Recv() {
        std::unique_lock<std::mutex> lc(mtx_);
        wait_ = true;
        cv_send_.notify_one();
        cv_recv_.wait(lc, [this] { return value_.has_value() || closed_; });
        if (closed_ && !value_.has_value()) {
            return std::nullopt;
        }
        T cur = value_.value();
        value_.reset();
        return cur;
    }

    void Close() {
        std::unique_lock<std::mutex> lock(mtx_);
        closed_ = true;
        cv_send_.notify_all();
        cv_recv_.notify_all();
    }

private:
    std::optional<T> value_;
    bool closed_ = false;
    bool wait_ = false;
    std::mutex mtx_;
    std::condition_variable cv_send_;
    std::condition_variable cv_recv_;
};