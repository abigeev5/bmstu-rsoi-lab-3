#pragma once

#include <string_view>

#include <userver/components/component_base.hpp>
#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/storages/postgres/cluster.hpp>

#include "domain/payment_service.hpp"
#include "storage/pg_repositories.hpp"

namespace payment {

class PaymentComponent final : public userver::components::ComponentBase {
  public:
    static constexpr std::string_view kName = "payment-service";

    PaymentComponent(const userver::components::ComponentConfig& config,
                     const userver::components::ComponentContext& context);

    const PaymentService& GetService() const { return service_; }

  private:
    userver::storages::postgres::ClusterPtr cluster_;
    PgPaymentRepository payments_;
    PaymentService service_;
};

}
