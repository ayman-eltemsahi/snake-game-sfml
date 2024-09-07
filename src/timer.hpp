#pragma once
#include <chrono>

class Timer {
 public:
  Timer() : m_start(std::chrono::high_resolution_clock::now()) {}

  float elapsed_millis() const {
    auto now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now - m_start).count();
  }

  float elapsed_micros() const {
    auto now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(now - m_start).count();
  }

  void reset() { m_start = std::chrono::high_resolution_clock::now(); }

 private:
  std::chrono::time_point<std::chrono::high_resolution_clock> m_start;
};
