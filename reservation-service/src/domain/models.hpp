#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace reservation {

inline constexpr std::string_view kStatusPaid = "PAID";
inline constexpr std::string_view kStatusCanceled = "CANCELED";

struct Hotel {
    std::string hotel_uid;
    std::string name;
    std::string country;
    std::string city;
    std::string address;
    std::optional<int> stars;
    int price{};
};

struct HotelPage {
    std::vector<Hotel> items;
    std::int64_t total{};
};

struct Reservation {
    std::string reservation_uid;
    Hotel hotel;
    std::string start_date;
    std::string end_date;
    std::string status;
    std::string payment_uid;
};

struct NewReservation {
    std::string username;
    std::string hotel_uid;
    std::string start_date;
    std::string end_date;
    std::string payment_uid;
};

}
