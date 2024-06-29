### Detailed Explanation
Instead of doing hard work, I have created this explanation using chatGPT.

#### Concepts

```cpp
template<typename T>
concept MoveConstructible = std::is_move_constructible_v<T>;
```

We define a concept `MoveConstructible` to ensure that the type `T` used with `threadsafe_stack` is move constructible. This provides a compile-time check and better expresses the requirements for the type.

#### Struct `empty_stack`

```cpp
struct empty_stack : std::exception {
    const char* what() const noexcept override {
        return "Stack is empty";
    }
};
```

This struct defines a custom exception to be thrown when an operation is attempted on an empty stack.

#### Class Template `threadsafe_stack`

The `threadsafe_stack` class template provides a thread-safe stack implementation for any `MoveConstructible` type `T`.

#### Private Members

```cpp
private:
    std::stack<T> data;
    mutable std::shared_mutex m;
```

- `data`: The actual stack storing elements.
- `m`: A `shared_mutex` allowing multiple readers or one writer.

#### Constructors and Assignment Operator

```cpp
public:
    threadsafe_stack() noexcept = default;

    threadsafe_stack(const threadsafe_stack& other) {
        std::shared_lock lock(other.m);
        data = other.data;
    }

    threadsafe_stack& operator=(const threadsafe_stack&) = delete;
```

- **Default Constructor**: Initializes the stack. The `noexcept` specifier indicates it does not throw exceptions.
- **Copy Constructor**: Uses `std::shared_lock` to allow multiple threads to read from `other` concurrently while copying its data.
- **Deleted Assignment Operator**: Prevents assignment, ensuring each stack instance is unique.

#### `push` Method

```cpp
void push(T new_value) {
    std::scoped_lock lock(m);
    data.push(std::move(new_value));
}
```

Locks the mutex using `std::scoped_lock` (C++17) to ensure thread safety and moves the new value onto the stack.

#### `emplace` Method

```cpp
void emplace(T&& new_value) {
    std::scoped_lock lock(m);
    data.emplace(std::forward<T>(new_value));
}
```

Constructs an element in place at the top of the stack using perfect forwarding. `std::scoped_lock` ensures thread safety.

#### `pop` Methods

1. **Returning `std::shared_ptr`**

```cpp
[[nodiscard]] std::optional<std::shared_ptr<T>> pop() {
    std::scoped_lock lock(m);
    if (data.empty()) {
        return std::nullopt;
    }
    auto res = std::make_shared<T>(std::move(data.top()));
    data.pop();
    return res;
}
```

Locks the mutex using `std::scoped_lock`, checks if the stack is empty, and returns an `std::optional` containing a `std::shared_ptr` to the top element. If the stack is empty, returns `std::nullopt`.

2. **Returning Value via Reference**

```cpp
std::optional<T> pop(T& value) {
    std::scoped_lock lock(m);
    if (data.empty()) {
        return std::nullopt;
    }
    value = std::move(data.top());
    data.pop();
    return value;
}
```

Locks the mutex, checks if the stack is empty, and assigns the top element to the provided reference. Returns `std::optional` to indicate success or failure.

#### `empty` Method

```cpp
bool empty() const noexcept {
    std::shared_lock lock(m);
    return data.empty();
}
```

Locks the mutex using `std::shared_lock` (allows multiple readers) and checks if the stack is empty. Marked as `noexcept` because it does not throw exceptions.

### Summary

This updated `threadsafe_stack` implementation leverages C++20 features to enhance safety and efficiency:

1. **Concepts**: Ensure types meet certain requirements at compile time.
2. **Mutex Types**: Use `std::shared_mutex` for shared access, allowing multiple readers and a single writer.
3. **Scoped Locks**: Use `std::scoped_lock` for better lock management.
4. **Optional Return Values**: Use `std::optional` to avoid exceptions for empty stack operations.
5. **Move Semantics and Perfect Forwarding**: Optimize element handling and insertion.
