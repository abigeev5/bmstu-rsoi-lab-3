#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace gateway {

inline constexpr std::string_view kStatusCanceled = "CANCELED";

struct HotelInfo {
    std::string hotel_uid;
    std::string name;
    std::string full_address;
    std::optional<int> stars;
};

struct Hotel {
    std::string hotel_uid;
    std::int64_t price{};
};

struct Reservation {
    std::string reservation_uid;
    HotelInfo hotel;
    std::string start_date;
    std::string end_date;
    std::string status;
    std::string payment_uid;
};

struct NewReservation {
    std::string hotel_uid;
    std::string start_date;
    std::string end_date;
    std::string payment_uid;
};

struct Payment {
    std::string payment_uid;
    std::string status;
    std::int64_t price{};
};

struct Loyalty {
    std::string status;
    int discount{};
    int reservation_count{};
};

}
