#include "domain/loyalty_service.hpp"

#include "domain/level.hpp"

namespace loyalty {

LoyaltyService::LoyaltyService(const ILoyaltyRepository& loyalties) : loyalties_(loyalties) {}

Loyalty LoyaltyService::GetLoyalty(const std::string& username) const {
    if (auto loyalty = loyalties_.Find(username)) {
        return *std::move(loyalty);
    }
    const auto level = LevelFor(0);
    return Loyalty{
        .reservation_count = 0,
        .status = std::string{level.status},
        .discount = level.discount,
    };
}

Loyalty LoyaltyService::Increment(const std::string& username) const {
    return loyalties_.ChangeReservationCount(username, 1);
}

Loyalty LoyaltyService::Decrement(const std::string& username) const {
    return loyalties_.ChangeReservationCount(username, -1);
}

}
