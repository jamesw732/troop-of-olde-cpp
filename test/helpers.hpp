#include <iostream>
#include <source_location>
#include <cmath>

// Macro version of assertEquals:
// template<typename T, typename U>
// void _assertEquals(
//     const T& expected, const U& actual,
//     const char* expectedStr, const char* actualStr,
//     const char* file, const int line) {
//     if (!(expected == actual)) {
//         std::cerr << file << ":" << line
//             << "Assertion failed: " << expectedStr << " == " << actualStr << std::endl;
//         std::terminate();
//     }
// }

template<typename T, typename U>
void assertEquals(const T& expected, const U& actual,
    const std::source_location& location = std::source_location::current()) {

    if (expected != actual) {
        std::cerr << location.file_name() << ":" << location.line()
            << " Assertion failed: " << expected << " == " << actual << std::endl;
        exit(1);
    }
}

template<typename T, typename U>
void assertClose(const T& expected, const U& actual, float atol=0.1,
    const std::source_location& location = std::source_location::current()) {

    if (abs(expected - actual) > atol) {
        std::cerr << location.file_name() << ":" << location.line()
            << " Assertion failed: " << actual << " within " << atol << " of " << expected << std::endl;
        exit(1);
    }
}
