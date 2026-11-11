#pragma once

#include <optional>
#include <string>
#include <string_view>

#include "domain/models.hpp"

namespace payment {

class IPaymentRepository {
  public:
    virtual ~IPaymentRepository() = default;

    virtual Payment Create(int price) const = 0;
    virtual std::optional<Payment> Find(const std::string& payment_uid) const = 0;
    virtual bool SetStatus(const std::string& payment_uid, std::string_view status) const = 0;
};

}
