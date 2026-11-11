#pragma once

#include <optional>
#include <string>

#include "domain/repositories.hpp"

namespace payment {

class PaymentService {
  public:
    explicit PaymentService(const IPaymentRepository& payments);

    Payment CreatePayment(int price) const;
    std::optional<Payment> GetPayment(const std::string& payment_uid) const;
    bool CancelPayment(const std::string& payment_uid) const;

  private:
    const IPaymentRepository& payments_;
};

}
