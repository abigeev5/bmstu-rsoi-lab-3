#pragma once

#include <userver/storages/postgres/cluster.hpp>

#include "domain/repositories.hpp"

namespace payment {

void CreateSchema(userver::storages::postgres::Cluster& cluster);

class PgPaymentRepository final : public IPaymentRepository {
  public:
    explicit PgPaymentRepository(userver::storages::postgres::ClusterPtr cluster);

    Payment Create(int price) const override;
    std::optional<Payment> Find(const std::string& payment_uid) const override;
    bool SetStatus(const std::string& payment_uid, std::string_view status) const override;

  private:
    userver::storages::postgres::ClusterPtr cluster_;
};

}
