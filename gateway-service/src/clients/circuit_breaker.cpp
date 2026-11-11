#include "clients/circuit_breaker.hpp"

namespace gateway {

CircuitBreaker::CircuitBreaker(int failure_threshold, Clock::duration open_duration)
    : failure_threshold_(failure_threshold), open_duration_(open_duration) {}

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
    failures_ = 0;
}

void CircuitBreaker::RecordFailure(Clock::time_point now) {
    const std::lock_guard lock(mutex_);
    switch (state_) {
    case State::kClosed:
        if (++failures_ < failure_threshold_) {
            return;
        }
        break;
    case State::kHalfOpen:
        break;
    case State::kOpen:
        // A request let through before the breaker opened: it is already open.
        return;
    }
    state_ = State::kOpen;
    opened_at_ = now;
    failures_ = 0;
}

}
