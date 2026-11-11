#include <chrono>

#include <gtest/gtest.h>

#include "clients/circuit_breaker.hpp"

namespace gateway {

namespace {

using namespace std::chrono_literals;

constexpr int kThreshold = 3;
constexpr auto kOpenDuration = 10s;

class CircuitBreakerTest : public ::testing::Test {
  protected:
    // Opens the breaker at `now_` with kThreshold failures in a row.
    void Open() {
        for (int i = 0; i < kThreshold; ++i) {
            ASSERT_TRUE(breaker_.AllowRequest(now_));
            breaker_.RecordFailure(now_);
        }
    }

    CircuitBreaker::Clock::time_point now_;
    CircuitBreaker breaker_{kThreshold, kOpenDuration};
};

}

TEST_F(CircuitBreakerTest, ClosedLetsRequestsThroughBelowThreshold) {
    for (int i = 0; i < kThreshold - 1; ++i) {
        ASSERT_TRUE(breaker_.AllowRequest(now_));
        breaker_.RecordFailure(now_);
    }
    EXPECT_TRUE(breaker_.AllowRequest(now_));
}

TEST_F(CircuitBreakerTest, SuccessResetsFailureCount) {
    for (int i = 0; i < kThreshold - 1; ++i) {
        breaker_.RecordFailure(now_);
    }
    breaker_.RecordSuccess();
    for (int i = 0; i < kThreshold - 1; ++i) {
        breaker_.RecordFailure(now_);
    }
    EXPECT_TRUE(breaker_.AllowRequest(now_));
}

TEST_F(CircuitBreakerTest, OpensAfterThresholdFailures) {
    Open();
    EXPECT_FALSE(breaker_.AllowRequest(now_));
    EXPECT_FALSE(breaker_.AllowRequest(now_ + kOpenDuration - 1ms));
}

TEST_F(CircuitBreakerTest, LetsOneTrialRequestThroughAfterOpenDuration) {
    Open();
    const auto later = now_ + kOpenDuration;
    EXPECT_TRUE(breaker_.AllowRequest(later));
    EXPECT_FALSE(breaker_.AllowRequest(later));
}

TEST_F(CircuitBreakerTest, SuccessfulTrialClosesBreaker) {
    Open();
    const auto later = now_ + kOpenDuration;
    ASSERT_TRUE(breaker_.AllowRequest(later));
    breaker_.RecordSuccess();
    EXPECT_TRUE(breaker_.AllowRequest(later));
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
