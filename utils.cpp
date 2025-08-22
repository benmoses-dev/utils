#include "utils.hpp"
#include <cmath>
#include <string>

using namespace std;

struct Circle {
    double r;
};
struct Rect {
    double w, h;
};
using Shape = variant<Circle, Rect>;

Result<int, string> divide(int a, int b) {
    if (b == 0) {
        return Result<int, string>::Err("Division by zero");
    }
    return Result<int, string>::Ok(a / b);
};

/**
 * This is just me playing around turning C++ into Rust/Go...
 */
int main() {
    time_scope("My entire main function");
    defer { cout << "Leaving main scope" << endl; }
    defer_end;

#if defined(__cpp_lib_scope) && __cpp_lib_scope >= 202207L
    defer_fail { cout << "Exception occurred!" << endl; }
    defer_end;
    defer_success { cout << "Leaving scope gracefully..." << endl; }
    defer_end;
#endif

    /**
     * Variant pattern matching.
     * I tried this with Optional and it was horrible, but this works nicely...
     */
    Shape s = Rect{3.0, 4.0};
    double area = match(
        s, [](Circle c) { return 3.141592653589793 * c.r * c.r; },
        [](Rect r) { return r.h * r.w; });
    cout << "area is: " << area << endl;

    /**
     * Using my custom Result type.
     * You must check both the success and error results.
     * The only downside is that the compiler isn't too useful...
     */
    auto r = divide(10, 2);
    r.match([](int v) { cout << "Result: " << v << endl; },
            [](const string &e) { cout << "Error: " << e << endl; });

    auto r2 = divide(5, 0);
    r2.match([](int v) { cout << "Result: " << v << endl; },
             [](const string &e) { cout << "Error: " << e << endl; });
}
