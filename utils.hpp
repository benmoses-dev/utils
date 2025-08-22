#pragma once

#include <chrono>
#include <cmath>
#include <iostream>
#include <variant>
#include <version>

template <class... Ts> struct overloaded : Ts... {
    using Ts::operator()...; // Inherit and bring into set
};
template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>; // Help CTAD as we have no constructor

template <class Variant, class... Fs> auto match(Variant &&v, Fs &&...fs) {
    return std::visit(
        overloaded{std::forward<Fs>(fs)...},
        std::forward<Variant>(v)); // Match the variant against the combined callable
}

#define CONCAT_IMPL(x, y) x##y
#define CONCAT(x, y) CONCAT_IMPL(x, y)

#if defined(__cpp_lib_scope) && __cpp_lib_scope >= 202207L
// C++23
#include <scope>
#define defer auto CONCAT(_defer_, __COUNTER__) = std::scope_exit([&]()
#define defer_fail auto CONCAT(_defer_fail_, __COUNTER__) = std::scope_fail([&]()
#define defer_success auto CONCAT(_defer_success_, __COUNTER__) = std::scope_success([&]()
#else
template <typename F> class scope_exit {
    F f;
    bool active = true;

  public:
    explicit scope_exit(F &&fun) : f(std::forward<F>(fun)) {}
    scope_exit(const scope_exit &) = delete;
    scope_exit &
    operator=(const scope_exit &) = delete; // Don't allow copying (would call f twice)
    scope_exit(scope_exit &&other) noexcept
        : f(std::move(other.f)), active(other.active) {
        other.active = false;
    }
    scope_exit &operator=(scope_exit &&other) noexcept {
        if (this != &other) {
            if (active)
                f(); // If we are intentionally overwriting, trigger the cleanup
            f = std::move(other.f);
            active = other.active;
            other.active = false;
        }
        return *this;
    }
    ~scope_exit() { // Use f as the cleanup function on the destructor once scope exits
        if (active)
            f();
    }
    void release() { active = false; } // Possible undo if needed
};
template <typename F> scope_exit(F &&) -> scope_exit<std::decay_t<F>>;
#define defer auto CONCAT(_defer_, __COUNTER__) = scope_exit([&]()
#endif

#define defer_end )

// Simple class for timing until destruction
class scope_timer {
    const char *name;
    std::chrono::steady_clock::time_point start;

  public:
    explicit scope_timer(const char *n)
        : name(n), start(std::chrono::steady_clock::now()) {}
    ~scope_timer() {
        auto end = std::chrono::steady_clock::now();
        std::cout
            << name << " took "
            << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
            << " ms" << std::endl;
    }
};

#define time_scope auto CONCAT(_time_scope_, __COUNTER__) = scope_timer

template <typename T, typename E> class Result {
    std::variant<T, E>
        data; // Result will essentially be a variant with an Ok type and an Err type

  public:
    template <typename U> static Result Ok(U &&val) {
        return Result(std::in_place_index<0>, std::forward<U>(val));
    }
    template <typename U> static Result Err(U &&val) {
        return Result(std::in_place_index<1>,
                      std::forward<U>(val)); // Avoid double move for large rvalues
    }

    // The types need to be distinct to use match, otherwise the lambdas won't overload...
    template <typename FOk, typename FErr> auto match(FOk &&ok, FErr &&err) const {
        return std::visit(overloaded{std::forward<FOk>(ok), std::forward<FErr>(err)},
                          data);
    }

  private:
    template <size_t I, typename U>
    explicit Result(std::in_place_index_t<I>, U &&val)
        : data(std::in_place_index<I>, std::forward<U>(val)) {}
};
