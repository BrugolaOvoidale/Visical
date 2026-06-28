#pragma once
#include <memory>
#include <utility>
#include <type_traits>

template<typename T>
class MoveOnlyFunction; 

template<typename Ret, typename... Args>
class MoveOnlyFunction<Ret(Args...)> {
    struct Concept {
        virtual ~Concept() = default;
        virtual Ret execute(Args... args) = 0;
    };

    template<typename F>
    struct Model : Concept {
        F func_;
        Model(F&& f) : func_(std::move(f)) {}
        
        Ret execute(Args... args) override {
            if constexpr (std::is_void_v<Ret>) {
                func_(std::forward<Args>(args)...);
            } else {
                return func_(std::forward<Args>(args)...);
            }
        }
    };

    std::unique_ptr<Concept> impl_;

public:
    MoveOnlyFunction() = default;

    template<typename F, typename = std::enable_if_t<!std::is_same_v<std::decay_t<F>, MoveOnlyFunction>>>
    MoveOnlyFunction(F&& f) : impl_(std::make_unique<Model<std::decay_t<F>>>(std::forward<F>(f))) {}

    MoveOnlyFunction(MoveOnlyFunction&&) = default;
    MoveOnlyFunction& operator=(MoveOnlyFunction&&) = default;
    MoveOnlyFunction(const MoveOnlyFunction&) = delete;
    MoveOnlyFunction& operator=(const MoveOnlyFunction&) = delete;

    Ret operator()(Args... args) const {
        if (impl_) {
            if constexpr (std::is_void_v<Ret>) {
                impl_->execute(std::forward<Args>(args)...);
            } else {
                return impl_->execute(std::forward<Args>(args)...);
            }
        } else {
            if constexpr (!std::is_void_v<Ret>) {
                return Ret{}; 
            }
        }
    }
    
    explicit operator bool() const { return impl_ != nullptr; }
};