#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <vector>

namespace gateway {

class CircuitBreaker {
  public:
    using Clock = std::chrono::steady_clock;

    CircuitBreaker(std::size_t failure_threshold, Clock::duration failure_window,
                   Clock::duration open_duration);

    bool AllowRequest(Clock::time_point now = Clock::now());
    void RecordSuccess();
    void RecordFailure(Clock::time_point now = Clock::now());

  private:
    enum class State : std::uint8_t { kClosed, kOpen, kHalfOpen };

    const Clock::duration failure_window_;
    const Clock::duration open_duration_;

    std::mutex mutex_;
    State state_ = State::kClosed;

    std::vector<Clock::time_point> failures_;
    std::size_t oldest_ = 0;
    std::size_t stored_ = 0;
    Clock::time_point opened_at_;
};

}
