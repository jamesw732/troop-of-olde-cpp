#pragma once
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

#include "bitsery/bitsery.h"
#include "bitsery/adapter/buffer.h"
#include "bitsery/traits/vector.h"
#include "bitsery/traits/string.h"
#include "bitsery/traits/array.h"


using Buffer = std::vector<uint8_t>;
using OutputAdapter = bitsery::OutputBufferAdapter<Buffer>;
using InputAdapter = bitsery::InputBufferAdapter<uint8_t*>;

inline float round(float a, int places) {
    return std::round(a * pow(10, places)) / pow(10, places);
}

inline bool is_close(float a, float b, float tol=1e-5f) {
    return std::abs(a - b) < tol;
}

inline void print_buffer(const std::vector<uint8_t>& buffer, const size_t& size) {
    int i = 0;
    for (uint8_t byte: buffer) {
        if (i >= size) {
            break;
        }
        i++;
        std::cout << std::hex
            << std::setw(2)
            << std::setfill('0')
            << static_cast<int>(byte) << ' ';
    }
    std::cout << std::dec << std::endl;
}

inline float angle_slerp(float a0, float a1, float t) {
    assert(a1 - a0 + 540 > 0);
    float diff = fmodf(a1 - a0 + 540, 360) - 180;
    return fmodf(a0 + diff * t, 360);
}

// Logging util
#define LOG_ERROR   0
#define LOG_WARNING 1
#define LOG_INFO    2
#define LOG_DEBUG   3
#define LOG_TRACE   4

enum class LogLevel: uint8_t {
    Error = LOG_ERROR,
    Warning = LOG_WARNING,
    Info = LOG_INFO,
    Debug = LOG_DEBUG,
    Trace = LOG_TRACE
};

#ifndef LOG_LEVEL
#define LOG_LEVEL 3
#endif

inline float get_duration_ms(auto duration) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

inline uint64_t get_timestamp_us() {
    using namespace std::chrono;
    return duration_cast<microseconds>(
        system_clock::now().time_since_epoch()
    ).count();
}

inline void log(std::ofstream& log_file, const Buffer& buffer, const size_t& size) {
    uint64_t timestamp = get_timestamp_us();
    log_file.write(reinterpret_cast<const char*>(&timestamp), sizeof(timestamp));
    uint16_t short_size = (uint16_t) size;
    log_file.write(reinterpret_cast<char*>(&short_size), sizeof(uint16_t));
    log_file.write(reinterpret_cast<const char*>(buffer.data()), size);
}

template <typename... Args>
inline void log(LogLevel level, Args&&... args) {
    auto timestamp = get_timestamp_us();
    std::chrono::microseconds us(timestamp);
    std::chrono::system_clock::time_point tp(us);
    std::time_t time = std::chrono::system_clock::to_time_t(tp);
    std::tm* tm = std::localtime(&time);
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(
            tp.time_since_epoch()
            ).count() % 1000;

    std::cout << std::put_time(tm, "%Y-%m-%d %H:%M:%S")
        << '.' << std::setfill('0') << std::setw(3) << millis
        << ": "; 
    (std::cout << ... << args) << "\n";
}

#define LOG(level, ...) \
    do { \
        if constexpr (static_cast<uint8_t>(level) <= LOG_LEVEL) \
            log(level, __VA_ARGS__); \
    } while (0)


inline int indent_index() {
    static int index = std::ios_base::xalloc();
    return index;
}

struct Indent {
    int delta;
};

inline std::ostream& operator<<(std::ostream& os, Indent i) {
    os.iword(indent_index()) += i.delta;
    return os;
}

inline Indent indent() {return {1};}
inline Indent unindent() {return {-1};}

inline void print_indent(std::ostream& os) {
    int level = os.iword(indent_index());
    for (int i = 0; i < level; i++) {
        os << "  ";
    }
}

template <typename T>
inline void print_vector(std::ostream& os, std::vector<T> container) {
    os << "{\n";
    os << indent();
    for (auto it = container.begin(); it != container.end(); it++) {
        print_indent(os);
        os << *it;
        if (it != std::prev(container.end())) {
            os << ",\n";
        }
    }
    os << "\n";
    os << unindent();
    print_indent(os);
    os <<"}\n";
}

template <typename T, size_t n>
inline void print_array(std::ostream& os, std::array<T, n> array, size_t size = 0) {
    if (size == 0) {
        size = array.size();
    }
    os << "{\n";
    os << indent();
    for (int i = 0; i < size; i++) {
        print_indent(os);
        os << array[i];
        if (i < size - 1) {
            os << ",\n";
        }
    }
    os << "\n";
    os << unindent();
    print_indent(os);
    os <<"}\n";
}
