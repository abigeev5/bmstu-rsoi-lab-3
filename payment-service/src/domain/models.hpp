#pragma once

#include <string>
#include <string_view>

namespace payment {

inline constexpr std::string_view kStatusPaid = "PAID";
inline constexpr std::string_view kStatusCanceled = "CANCELED";

struct Payment {
    std::string payment_uid;
    std::string status;
    int price{};
};

}
