#pragma once

#include <chrono>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include <userver/formats/json/value.hpp>

#include "domain/clients.hpp"

namespace gateway {

struct BookingRequest {
    std::string hotel_uid;
    std::string start_date;
    std::string end_date;
};

struct BookingResult {
    Reservation reservation;
    Payment payment;
    int discount{};
};

struct ReservationView {
    Reservation reservation;
    std::optional<Payment> payment;
};

struct UserInfo {
    std::vector<ReservationView> reservations;
    Loyalty loyalty;
};

class ValidationError : public std::runtime_error {
  public:
    ValidationError(std::string field, const std::string& message);

    const std::string& Field() const { return field_; }

  private:
    std::string field_;
};

std::optional<std::chrono::sys_days> ParseDate(std::string_view value);

std::int64_t CalcPrice(std::int64_t price_per_night, std::int64_t nights, int discount);

class BookingService {
  public:
    BookingService(const IReservationClient& reservations, const IPaymentClient& payments,
                   const ILoyaltyClient& loyalty);

    userver::formats::json::Value ListHotels(int page, int size) const;
    UserInfo GetUserInfo(const std::string& username) const;
    std::vector<ReservationView> GetReservations(const std::string& username) const;
    std::optional<ReservationView> GetReservation(const std::string& username,
                                                  const std::string& reservation_uid) const;
    BookingResult Book(const std::string& username, const BookingRequest& request) const;
    bool Cancel(const std::string& username, const std::string& reservation_uid) const;
    Loyalty GetLoyalty(const std::string& username) const;

  private:
    ReservationView WithPayment(Reservation reservation) const;

    const IReservationClient& reservations_;
    const IPaymentClient& payments_;
    const ILoyaltyClient& loyalty_;
};

}
