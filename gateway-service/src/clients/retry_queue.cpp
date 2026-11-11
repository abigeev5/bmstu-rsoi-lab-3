#include "clients/retry_queue.hpp"

#include <exception>
#include <utility>

#include <userver/logging/log.hpp>

#include "domain/clients.hpp"

namespace gateway {

RetryQueue::RetryQueue(std::chrono::milliseconds interval)
    : task_("retry-queue", interval, [this] { RetryAll(); }) {}

void RetryQueue::Push(Job job) {
    const std::lock_guard lock(mutex_);
    jobs_.push_back(std::move(job));
}

void RetryQueue::RetryAll() {
    while (true) {
        Job job;
        {
            const std::lock_guard lock(mutex_);
            if (jobs_.empty()) {
                return;
            }
            job = jobs_.front();
        }
        try {
            job();
            LOG_INFO() << "Queued request succeeded";
        } catch (const ServiceUnavailable& e) {
            LOG_INFO() << e.what() << ", queued requests wait for the next retry";
            return;
        } catch (const std::exception& e) {
            LOG_ERROR() << "Queued request dropped: " << e.what();
        }
        // Only RetryAll() removes jobs and runs one at a time, so the front is still `job`.
        const std::lock_guard lock(mutex_);
        jobs_.pop_front();
    }
}

}
