#include <chrono>
#include <stdexcept>
#include <string>
#include <vector>

#include <userver/utest/utest.hpp>

#include "clients/retry_queue.hpp"
#include "domain/clients.hpp"

namespace gateway {

namespace {

// The periodic run never comes during a test: the tests call RetryAll() themselves.
constexpr std::chrono::hours kNeverDuringTest{1};

using Runs = std::vector<std::string>;

}

UTEST(RetryQueue, RetriesInOrderUntilServiceIsBack) {
    RetryQueue queue{kNeverDuringTest};
    Runs runs;
    int failures_left = 2;
    queue.Push([&] {
        runs.emplace_back("first");
        if (failures_left-- > 0) {
            throw ServiceUnavailable("Loyalty");
        }
    });
    queue.Push([&] { runs.emplace_back("second"); });

    for (int round = 0; round < 4; ++round) {
        queue.RetryAll();
    }
    // The second job waits behind the failing first one; done jobs leave the queue.
    EXPECT_EQ(runs, (Runs{"first", "first", "first", "second"}));
}

UTEST(RetryQueue, DropsJobFailingWithOtherError) {
    RetryQueue queue{kNeverDuringTest};
    Runs runs;
    queue.Push([&] {
        runs.emplace_back("first");
        throw std::runtime_error("404 Not Found");
    });
    queue.Push([&] { runs.emplace_back("second"); });

    queue.RetryAll();
    queue.RetryAll();
    EXPECT_EQ(runs, (Runs{"first", "second"}));
}

}
