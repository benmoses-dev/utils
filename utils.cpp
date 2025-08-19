#include "utils.hpp"
#include <cmath>
#include <string>

struct Circle {
    double r;
};
struct Rect {
    double w, h;
};
using Shape = std::variant<Circle, Rect>;

int main() {
    /**
     * Variant pattern matching
     */
    Shape s = Rect{3.0, 4.0};
    double area = vmatch(
        s, [](Circle c) { return 3.141592653589793 * c.r * c.r; },
        [](Rect r) { return r.h * r.w; });
    std::cout << "area is: " << area << std::endl;

    /**
     * scope_exit / defer
     */
    defer([] { std::cout << "Leaving main scope\n"; });
    std::cout << "Doing some work...\n";

    /**
     * scoped_timer
     */
    {
        scoped_timer timer("Starting computation...");
        volatile int sum = 0;
        for (int i = 0; i < 1000000; ++i)
            sum += i;
    }

    /**
     * clamp / lerp
     */
    std::cout << "clamp(10,0,5) = " << clamp(10, 0, 5) << "\n";             // 5
    std::cout << "lerp(0.0,10.0,0.25) = " << lerp(0.0, 10.0, 0.25) << "\n"; // 2.5

    /**
     * Result<T,E>
     */
    auto divide = [](int a, int b) -> Result<int, std::string> {
        if (b == 0) {
            return Result<int, std::string>::Err("Divide by zero");
        }
        return Result<int, std::string>::Ok(a / b);
    };

    auto r = divide(10, 2);
    r.match([](int v) { std::cout << "Result: " << v << "\n"; },
            [](const std::string &e) { std::cout << "Error: " << e << "\n"; });

    auto r2 = divide(5, 0);
    r2.match([](int v) { std::cout << "Result: " << v << "\n"; },
             [](const std::string &e) { std::cout << "Error: " << e << "\n"; });
}
