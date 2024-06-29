#include <exception>
#include <memory>
#include <mutex>
#include <stack>
#include <optional>
#include <shared_mutex>
#include <utility>

struct empty_stack : std::exception {
    const char* what() const noexcept override {
        return "Stack is empty";
    }
};

template<typename T>
concept MoveConstructible = std::is_move_constructible_v<T>;

template<MoveConstructible T>
class threadsafe_stack {
private:
    std::stack<T> data;
    mutable std::shared_mutex m;

public:
    threadsafe_stack() noexcept = default;

    threadsafe_stack(const threadsafe_stack& other) {
        std::shared_lock lock(other.m);
        data = other.data;
    }

    threadsafe_stack& operator=(const threadsafe_stack&) = delete;

    void push(T new_value) {
        std::scoped_lock lock(m);
        data.push(std::move(new_value));
    }

    void emplace(T&& new_value) {
        std::scoped_lock lock(m);
        data.emplace(std::forward<T>(new_value));
    }

    [[nodiscard]] std::optional<std::shared_ptr<T>> pop() {
        std::scoped_lock lock(m);
        if (data.empty()) {
            return std::nullopt;
        }
        auto res = std::make_shared<T>(std::move(data.top()));
        data.pop();
        return res;
    }

    std::optional<T> pop(T& value) {
        std::scoped_lock lock(m);
        if (data.empty()) {
            return std::nullopt;
        }
        value = std::move(data.top());
        data.pop();
        return value;
    }

    bool empty() const noexcept {
        std::shared_lock lock(m);
        return data.empty();
    }
};
