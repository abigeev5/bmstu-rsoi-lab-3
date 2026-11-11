#pragma once

#include <functional>

namespace gateway {

// Requests that must eventually reach a service. A job is a call to a client; while it throws
// ServiceUnavailable it stays in the queue and is retried later.
class IRetryQueue {
  public:
    using Job = std::function<void()>;

    virtual ~IRetryQueue() = default;

    virtual void Push(Job job) = 0;
};

}
