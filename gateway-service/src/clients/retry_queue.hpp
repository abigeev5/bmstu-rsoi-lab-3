#pragma once

#include <chrono>
#include <deque>
#include <mutex>

#include <userver/utils/periodic_task.hpp>

#include "domain/retry_queue.hpp"

namespace gateway {

// In-memory FIFO queue of the gateway: every `interval` RetryAll() runs the queued jobs in order.
class RetryQueue final : public IRetryQueue {
  public:
    explicit RetryQueue(std::chrono::milliseconds interval);

    void Push(Job job) override;

    // Runs the jobs from the front until one fails with ServiceUnavailable: that one and
    // the rest wait for the next run. A job failing with any other error is dropped.
    void RetryAll();

  private:
    // Guards only the deque; jobs run outside of it.
    std::mutex mutex_;
    std::deque<Job> jobs_;
    // The last member: it is stopped before the queue it works on is destroyed.
    userver::utils::PeriodicTask task_;
};

}
