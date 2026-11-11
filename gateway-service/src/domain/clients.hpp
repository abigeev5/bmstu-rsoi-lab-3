#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include <userver/formats/json/value.hpp>

#include "domain/models.hpp"

namespace gateway {

class IReservationClient {
  public:
    virtual ~IReservationClient() = default;

    virtual userver::formats::json::Value ListHotels(int page, int size) const = 0;
    virtual std::optional<Hotel> GetHotel(const std::string& hotel_uid) const = 0;
    virtual std::vector<Reservation> GetReservations(const std::string& username) const = 0;
    virtual std::optional<Reservation> GetReservation(const std::string& username,
                                                      const std::string& reservation_uid) const = 0;
    virtual Reservation CreateReservation(const std::string& username,
                                          const NewReservation& reservation) const = 0;
    virtual void CancelReservation(const std::string& username,
                                   const std::string& reservation_uid) const = 0;
};

class IPaymentClient {
  public:
    virtual ~IPaymentClient() = default;

    virtual Payment CreatePayment(std::int64_t price) const = 0;
    virtual std::optional<Payment> GetPayment(const std::string& payment_uid) const = 0;
    virtual void CancelPayment(const std::string& payment_uid) const = 0;
};

class ILoyaltyClient {
  public:
    virtual ~ILoyaltyClient() = default;

    virtual Loyalty GetLoyalty(const std::string& username) const = 0;
    virtual void Increment(const std::string& username) const = 0;
    virtual void Decrement(const std::string& username) const = 0;
};

}
