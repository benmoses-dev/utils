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

#ifdef __cpp_lib_scope
// C++23
#include <scope>
using my_scope_exit = std::scope_exit;
#define defer_fail auto CONCAT(_defer_fail_, __LINE__) = std::scope_fail
#define defer_success auto CONCAT(_defer_success_, __LINE__) = std::scope_success
#else
#include <functional>
class scope_exit {
    std::function<void()> f;
    bool active = true;

  public:
    explicit scope_exit(std::function<void()> f) : f(std::move(f)) {}
    ~scope_exit() { // Use f as the cleanup function on the destructor once scope exits
        if (active)
            f();
    }
    void release() { active = false; } // Possible undo if needed
};
using my_scope_exit = scope_exit;
#endif

// Use _defer_linenumber to release the guard (or just use scope_exit directly...)
#define defer auto CONCAT(_defer_, __LINE__) = my_scope_exit

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

#define time_scope auto CONCAT(_time_scope_, __LINE__) = scope_timer

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
