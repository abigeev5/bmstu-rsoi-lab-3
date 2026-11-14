#include "clients/circuit_breaker.hpp"

namespace gateway {

CircuitBreaker::CircuitBreaker(std::size_t failure_threshold, Clock::duration failure_window,
                               Clock::duration open_duration)
    : failure_window_(failure_window), open_duration_(open_duration), failures_(failure_threshold) {
}

bool CircuitBreaker::AllowRequest(Clock::time_point now) {
    const std::lock_guard lock(mutex_);
    switch (state_) {
    case State::kClosed:
        return true;
    case State::kOpen:
        if (now - opened_at_ < open_duration_) {
            return false;
        }
        state_ = State::kHalfOpen;
        return true;
    case State::kHalfOpen:
        return false;
    }
    return false;
}

void CircuitBreaker::RecordSuccess() {
    const std::lock_guard lock(mutex_);
    state_ = State::kClosed;
}

void CircuitBreaker::RecordFailure(Clock::time_point now) {
    const std::lock_guard lock(mutex_);
    switch (state_) {
    case State::kClosed:
        failures_[oldest_] = now;
        oldest_ = (oldest_ + 1) % failures_.size();
        if (stored_ < failures_.size()) {
            ++stored_;
        }
        
        if (stored_ < failures_.size() || now - failures_[oldest_] >= failure_window_) {
            return;
        }
        break;
    case State::kHalfOpen:
        break;
    case State::kOpen:
        return;
    }
    state_ = State::kOpen;
    opened_at_ = now;
    stored_ = 0;
}

}
