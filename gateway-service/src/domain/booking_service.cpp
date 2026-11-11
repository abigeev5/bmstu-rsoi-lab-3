#include "domain/booking_service.hpp"

#include <cstddef>
#include <utility>

#include "common/http.hpp"

namespace gateway {

namespace {

std::optional<int> ParseNumber(std::string_view value, std::size_t pos, std::size_t length) {
    int result = 0;
    for (const char c : value.substr(pos, length)) {
        if (c < '0' || c > '9') {
            return std::nullopt;
        }
        result = result * 10 + (c - '0');
    }
    return result;
}

}

ValidationError::ValidationError(std::string field, const std::string& message)
    : std::runtime_error(message), field_(std::move(field)) {}

std::optional<std::chrono::sys_days> ParseDate(std::string_view value) {
    if (value.size() != 10 || value[4] != '-' || value[7] != '-') {
        return std::nullopt;
    }
    const auto year = ParseNumber(value, 0, 4);
    const auto month = ParseNumber(value, 5, 2);
    const auto day = ParseNumber(value, 8, 2);
    if (!year || !month || !day) {
        return std::nullopt;
    }
    const std::chrono::year_month_day date{std::chrono::year{*year},
                                           std::chrono::month{static_cast<unsigned>(*month)},
                                           std::chrono::day{static_cast<unsigned>(*day)}};
    if (!date.ok()) {
        return std::nullopt;
    }
    return std::chrono::sys_days{date};
}

std::int64_t CalcPrice(std::int64_t price_per_night, std::int64_t nights, int discount) {
    return price_per_night * nights * (100 - discount) / 100;
}

BookingService::BookingService(const IReservationClient& reservations,
                               const IPaymentClient& payments, const ILoyaltyClient& loyalty)
    : reservations_(reservations), payments_(payments), loyalty_(loyalty) {}

userver::formats::json::Value BookingService::ListHotels(int page, int size) const {
    return reservations_.ListHotels(page, size);
}

UserInfo BookingService::GetUserInfo(const std::string& username) const {
    return UserInfo{
        .reservations = GetReservations(username),
        .loyalty = loyalty_.GetLoyalty(username),
    };
}

std::vector<ReservationView> BookingService::GetReservations(const std::string& username) const {
    std::vector<ReservationView> views;
    for (auto& reservation : reservations_.GetReservations(username)) {
        views.push_back(WithPayment(std::move(reservation)));
    }
    return views;
}

std::optional<ReservationView>
BookingService::GetReservation(const std::string& username,
                               const std::string& reservation_uid) const {
    if (!rsoi::IsUuid(reservation_uid)) {
        return std::nullopt;
    }
    auto reservation = reservations_.GetReservation(username, reservation_uid);
    if (!reservation) {
        return std::nullopt;
    }
    return WithPayment(*std::move(reservation));
}

BookingResult BookingService::Book(const std::string& username,
                                   const BookingRequest& request) const {
    if (!rsoi::IsUuid(request.hotel_uid)) {
        throw ValidationError("hotelUid", "must be a UUID");
    }
    const auto start = ParseDate(request.start_date);
    if (!start) {
        throw ValidationError("startDate", "must be a date in the YYYY-MM-DD format");
    }
    const auto end = ParseDate(request.end_date);
    if (!end) {
        throw ValidationError("endDate", "must be a date in the YYYY-MM-DD format");
    }
    if (*end <= *start) {
        throw ValidationError("endDate", "must be after startDate");
    }
    const auto hotel = reservations_.GetHotel(request.hotel_uid);
    if (!hotel) {
        throw ValidationError("hotelUid", "hotel not found");
    }

    const auto nights = (*end - *start).count();
    const auto discount = loyalty_.GetLoyalty(username).discount;
    auto payment = payments_.CreatePayment(CalcPrice(hotel->price, nights, discount));
    auto reservation =
        reservations_.CreateReservation(username, NewReservation{
                                                      .hotel_uid = request.hotel_uid,
                                                      .start_date = request.start_date,
                                                      .end_date = request.end_date,
                                                      .payment_uid = payment.payment_uid,
                                                  });
    loyalty_.Increment(username);

    return BookingResult{
        .reservation = std::move(reservation),
        .payment = std::move(payment),
        .discount = discount,
    };
}

bool BookingService::Cancel(const std::string& username, const std::string& reservation_uid) const {
    if (!rsoi::IsUuid(reservation_uid)) {
        return false;
    }
    const auto reservation = reservations_.GetReservation(username, reservation_uid);
    if (!reservation) {
        return false;
    }
    if (reservation->status == kStatusCanceled) {
        return true;
    }
    reservations_.CancelReservation(username, reservation_uid);
    payments_.CancelPayment(reservation->payment_uid);
    loyalty_.Decrement(username);
    return true;
}

Loyalty BookingService::GetLoyalty(const std::string& username) const {
    return loyalty_.GetLoyalty(username);
}

ReservationView BookingService::WithPayment(Reservation reservation) const {
    auto payment = payments_.GetPayment(reservation.payment_uid);
    return ReservationView{
        .reservation = std::move(reservation),
        .payment = std::move(payment),
    };
}

}
