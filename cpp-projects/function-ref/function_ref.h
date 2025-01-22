#pragma once

#include <type_traits>
#include <utility>
#include <functional>

template <typename Signature>
class FunctionRef;

template <typename ReturnType, typename... Args>
class FunctionRef<ReturnType(Args...)> {
public:
    template <typename Callback>
    FunctionRef(Callback&& callback)
        : funciton_(reinterpret_cast<std::function<ReturnType(Args...)>*>(&callback)) {
        if constexpr (!std::is_void_v<ReturnType>) {
            trampoline_ = [](Args... args,
                             std::function<ReturnType(Args...)>* funciton) -> ReturnType {
                return (*reinterpret_cast<std::remove_reference_t<Callback>*>(funciton))(
                    std::forward<Args>(args)...);
            };
        } else {
            trampoline_ = [](Args... args, std::function<ReturnType(Args...)>* funciton) -> void {
                (*reinterpret_cast<std::remove_reference_t<Callback>*>(funciton))(
                    std::forward<Args>(args)...);
            };
        }
    }

    template <typename Callback>
    FunctionRef(Callback* callback)
        : funciton_(reinterpret_cast<std::function<ReturnType(Args...)>*>(callback)) {
        if constexpr (!std::is_void_v<ReturnType>) {
            trampoline_ = [](Args... args,
                             std::function<ReturnType(Args...)>* funciton) -> ReturnType {
                return (*reinterpret_cast<std::remove_reference_t<Callback>*>(funciton))(
                    std::forward<Args>(args)...);
            };
        } else {
            trampoline_ = [](Args... args, std::function<ReturnType(Args...)>* funciton) -> void {
                (*reinterpret_cast<std::remove_reference_t<Callback>*>(funciton))(
                    std::forward<Args>(args)...);
            };
        }
    }

    ReturnType operator()(Args... args) {
        if constexpr (std::is_void_v<ReturnType>) {
            trampoline_(std::forward<Args>(args)..., funciton_);
            return;
        }
        return trampoline_(std::forward<Args>(args)..., funciton_);
    }

private:
    std::function<ReturnType(Args...)>* funciton_;
    ReturnType (*trampoline_)(Args..., std::function<ReturnType(Args...)>*);
};
