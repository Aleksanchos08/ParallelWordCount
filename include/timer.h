#pragma once
#include <chrono>

namespace wc {

class Timer {
public:
    Timer() { reset(); }
    void reset() { start_ = std::chrono::steady_clock::now(); }
    double elapsed() const {  // seconds since construction / last reset
        return std::chrono::duration<double>(std::chrono::steady_clock::now() - start_).count();
    }
private:
    std::chrono::steady_clock::time_point start_;
};

}
