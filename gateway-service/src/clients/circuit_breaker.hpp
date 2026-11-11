#pragma once

#include <chrono>
#include <cstdint>
#include <mutex>

namespace gateway {

// Circuit Breaker for one backend service; the statistics live in memory.
//  - Closed: requests go to the service. `failure_threshold` failures in a row open the breaker.
//  - Open: requests are rejected right away, the caller answers with a fallback.
//    After `open_duration` the breaker lets one trial request through (Half-Open).
//  - Half-Open: the trial request is in flight, the others are rejected.
//    Its success closes the breaker, its failure opens it for another `open_duration`.
// Every request that AllowRequest() let through must be reported with RecordSuccess()
// or RecordFailure().
class CircuitBreaker {
  public:
    using Clock = std::chrono::steady_clock;

    CircuitBreaker(int failure_threshold, Clock::duration open_duration);

    // Whether a request may go to the service now; false means "answer with a fallback".
    bool AllowRequest(Clock::time_point now = Clock::now());
    void RecordSuccess();
    void RecordFailure(Clock::time_point now = Clock::now());

  private:
    enum class State : std::uint8_t { kClosed, kOpen, kHalfOpen };

    const int failure_threshold_;
    const Clock::duration open_duration_;

    // Guards only a few assignments and never waits on I/O, so a plain mutex is enough.
    std::mutex mutex_;
    State state_ = State::kClosed;
    int failures_ = 0;
    Clock::time_point opened_at_;
};

}
