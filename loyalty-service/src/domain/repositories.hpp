#pragma once

#include <optional>
#include <string>

#include "domain/models.hpp"

namespace loyalty {

class ILoyaltyRepository {
  public:
    virtual ~ILoyaltyRepository() = default;

    virtual std::optional<Loyalty> Find(const std::string& username) const = 0;
    virtual Loyalty ChangeReservationCount(const std::string& username, int delta) const = 0;
};

}
