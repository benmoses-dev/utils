#pragma once

#include <chrono>
#include <cmath>
#include <functional>
#include <iostream>
#include <variant>

/**
 * Overloaded helper - use inheritence and operator overloading
 */
template <class... Ts> struct overloaded : Ts... {
    using Ts::operator()...;
};
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

/**
 * Pattern matching for variant (exhaustive)
 */
template <class Variant, class... Fs> auto vmatch(Variant &&v, Fs &&...fs) {
    return std::visit(overloaded{std::forward<Fs>(fs)...}, std::forward<Variant>(v));
}

/**
 * scope_exit / defer
 */
class scope_exit {
    std::function<void()> f;
    bool active = true;

  public:
    explicit scope_exit(std::function<void()> f) : f(std::move(f)) {}
    ~scope_exit() {
        if (active)
            f();
    }
    void release() { active = false; }
};

#define CONCAT_IMPL(x, y) x##y
#define CONCAT(x, y) CONCAT_IMPL(x, y)
#define defer auto CONCAT(_defer_, __LINE__) = scope_exit

/**
 * scoped_timer
 */
class scoped_timer {
    const char *name;
    std::chrono::steady_clock::time_point start;

  public:
    explicit scoped_timer(const char *n)
        : name(n), start(std::chrono::steady_clock::now()) {}
    ~scoped_timer() {
        auto end = std::chrono::steady_clock::now();
        std::cout
            << name << " took "
            << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
            << " ms\n";
    }
};

/**
 * Math helpers
 */
template <typename T> constexpr T clamp(T val, T lo, T hi) {
    return val < lo ? lo : (val > hi ? hi : val);
}

template <typename T> constexpr T lerp(T a, T b, double t) { return a + (b - a) * t; }

/**
 * Result<T,E> type with enforced pattern matching
 */
template <typename T, typename E> class Result {
    std::variant<T, E> data;

  public:
    static Result Ok(T val) { return Result(std::in_place_index<0>, std::move(val)); }
    static Result Err(E val) { return Result(std::in_place_index<1>, std::move(val)); }

    template <typename FOk, typename FErr> auto match(FOk &&ok, FErr &&err) const {
        return std::visit(overloaded{std::forward<FOk>(ok), std::forward<FErr>(err)},
                          data);
    }

  private:
    template <size_t I, typename U>
    explicit Result(std::in_place_index_t<I>, U &&val)
        : data(std::in_place_index<I>, std::forward<U>(val)) {}
};
