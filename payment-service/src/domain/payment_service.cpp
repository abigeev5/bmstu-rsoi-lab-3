#include "domain/payment_service.hpp"

namespace payment {

PaymentService::PaymentService(const IPaymentRepository& payments) : payments_(payments) {}

Payment PaymentService::CreatePayment(int price) const {
    return payments_.Create(price);
}

std::optional<Payment> PaymentService::GetPayment(const std::string& payment_uid) const {
    return payments_.Find(payment_uid);
}

bool PaymentService::CancelPayment(const std::string& payment_uid) const {
    return payments_.SetStatus(payment_uid, kStatusCanceled);
}

}
