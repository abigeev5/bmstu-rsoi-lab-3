#pragma once

#include <userver/storages/postgres/cluster.hpp>

#include "domain/repositories.hpp"

namespace loyalty {

void CreateSchema(userver::storages::postgres::Cluster& cluster);

class PgLoyaltyRepository final : public ILoyaltyRepository {
  public:
    explicit PgLoyaltyRepository(userver::storages::postgres::ClusterPtr cluster);

    std::optional<Loyalty> Find(const std::string& username) const override;
    Loyalty ChangeReservationCount(const std::string& username, int delta) const override;

  private:
    userver::storages::postgres::ClusterPtr cluster_;
};

}
