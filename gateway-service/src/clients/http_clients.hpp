#pragma once

#include <chrono>
#include <string>

#include <userver/clients/http/client.hpp>

#include "domain/clients.hpp"

namespace gateway {

class ReservationClient final : public IReservationClient {
  public:
    ReservationClient(userver::clients::http::Client& http, std::string base_url,
                      std::chrono::milliseconds timeout);

    userver::formats::json::Value ListHotels(int page, int size) const override;
    std::optional<Hotel> GetHotel(const std::string& hotel_uid) const override;
    std::vector<Reservation> GetReservations(const std::string& username) const override;
    std::optional<Reservation> GetReservation(const std::string& username,
                                              const std::string& reservation_uid) const override;
    Reservation CreateReservation(const std::string& username,
                                  const NewReservation& reservation) const override;
    void CancelReservation(const std::string& username,
                           const std::string& reservation_uid) const override;

  private:
    userver::clients::http::Client& http_;
    std::string base_url_;
    std::chrono::milliseconds timeout_;
};

class PaymentClient final : public IPaymentClient {
  public:
    PaymentClient(userver::clients::http::Client& http, std::string base_url,
                  std::chrono::milliseconds timeout);

    Payment CreatePayment(std::int64_t price) const override;
    std::optional<Payment> GetPayment(const std::string& payment_uid) const override;
    void CancelPayment(const std::string& payment_uid) const override;

  private:
    userver::clients::http::Client& http_;
    std::string base_url_;
    std::chrono::milliseconds timeout_;
};

class LoyaltyClient final : public ILoyaltyClient {
  public:
    LoyaltyClient(userver::clients::http::Client& http, std::string base_url,
                  std::chrono::milliseconds timeout);

    Loyalty GetLoyalty(const std::string& username) const override;
    void Increment(const std::string& username) const override;
    void Decrement(const std::string& username) const override;

  private:
    userver::clients::http::Client& http_;
    std::string base_url_;
    std::chrono::milliseconds timeout_;
};

}
