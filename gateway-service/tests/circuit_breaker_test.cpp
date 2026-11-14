#include <chrono>
#include <cstddef>

#include <gtest/gtest.h>

#include "clients/circuit_breaker.hpp"

namespace gateway {

namespace {

using namespace std::chrono_literals;

constexpr std::size_t kThreshold = 3;
constexpr auto kFailureWindow = 10s;
constexpr auto kOpenDuration = 10s;

class CircuitBreakerTest : public ::testing::Test {
  protected:
    void Fail(CircuitBreaker::Clock::time_point at) {
        ASSERT_TRUE(breaker_.AllowRequest(at));
        breaker_.RecordFailure(at);
    }

    // Opens the breaker at `now_` with kThreshold failures at once.
    void Open() {
        for (std::size_t i = 0; i < kThreshold; ++i) {
            Fail(now_);
        }
    }

    CircuitBreaker::Clock::time_point now_;
    CircuitBreaker breaker_{kThreshold, kFailureWindow, kOpenDuration};
};

}

TEST_F(CircuitBreakerTest, ClosedLetsRequestsThroughBelowThreshold) {
    for (std::size_t i = 0; i < kThreshold - 1; ++i) {
        Fail(now_);
    }
    EXPECT_TRUE(breaker_.AllowRequest(now_));
}

TEST_F(CircuitBreakerTest, OpensAfterThresholdFailures) {
    Open();
    EXPECT_FALSE(breaker_.AllowRequest(now_));
    EXPECT_FALSE(breaker_.AllowRequest(now_ + kOpenDuration - 1ms));
}

TEST_F(CircuitBreakerTest, OpensWhenFailuresFitInWindow) {
    Fail(now_);
    Fail(now_ + 5s);
    Fail(now_ + kFailureWindow - 1ms);
    EXPECT_FALSE(breaker_.AllowRequest(now_ + kFailureWindow));
}

TEST_F(CircuitBreakerTest, FailuresOlderThanWindowDoNotCount) {
    Fail(now_);
    Fail(now_ + 5s);
    Fail(now_ + kFailureWindow);
    // The first failure has left the window: only two are in it.
    ASSERT_TRUE(breaker_.AllowRequest(now_ + kFailureWindow));
    // The ring buffer drops the first failure; the last three fit in the window.
    breaker_.RecordFailure(now_ + kFailureWindow + 1s);
    EXPECT_FALSE(breaker_.AllowRequest(now_ + kFailureWindow + 1s));
}

TEST_F(CircuitBreakerTest, SuccessDoesNotForgetFailuresInWindow) {
    Fail(now_);
    Fail(now_);
    breaker_.RecordSuccess();
    Fail(now_ + 1s);
    EXPECT_FALSE(breaker_.AllowRequest(now_ + 1s));
}

TEST_F(CircuitBreakerTest, LetsOneTrialRequestThroughAfterOpenDuration) {
    Open();
    const auto later = now_ + kOpenDuration;
    EXPECT_TRUE(breaker_.AllowRequest(later));
    EXPECT_FALSE(breaker_.AllowRequest(later));
}

TEST_F(CircuitBreakerTest, SuccessfulTrialClosesBreakerWithEmptyWindow) {
    Open();
    const auto later = now_ + kOpenDuration;
    ASSERT_TRUE(breaker_.AllowRequest(later));
    breaker_.RecordSuccess();
    // The failures that opened the breaker are not counted again.
    for (std::size_t i = 0; i < kThreshold - 1; ++i) {
        Fail(later);
    }
    EXPECT_TRUE(breaker_.AllowRequest(later));
}

TEST_F(CircuitBreakerTest, FailedTrialOpensBreakerAgain) {
    Open();
    const auto later = now_ + kOpenDuration;
    ASSERT_TRUE(breaker_.AllowRequest(later));
    breaker_.RecordFailure(later);
    EXPECT_FALSE(breaker_.AllowRequest(later + kOpenDuration - 1ms));
    EXPECT_TRUE(breaker_.AllowRequest(later + kOpenDuration));
}

}
