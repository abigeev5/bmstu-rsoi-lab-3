#pragma once

#include <string>

#include "domain/repositories.hpp"

namespace loyalty {

class LoyaltyService {
  public:
    explicit LoyaltyService(const ILoyaltyRepository& loyalties);

    Loyalty GetLoyalty(const std::string& username) const;
    Loyalty Increment(const std::string& username) const;
    Loyalty Decrement(const std::string& username) const;

  private:
    const ILoyaltyRepository& loyalties_;
};

}
