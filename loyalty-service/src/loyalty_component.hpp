#pragma once

#include <string_view>

#include <userver/components/component_base.hpp>
#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/storages/postgres/cluster.hpp>

#include "domain/loyalty_service.hpp"
#include "storage/pg_repositories.hpp"

namespace loyalty {

class LoyaltyComponent final : public userver::components::ComponentBase {
  public:
    static constexpr std::string_view kName = "loyalty-service";

    LoyaltyComponent(const userver::components::ComponentConfig& config,
                     const userver::components::ComponentContext& context);

    const LoyaltyService& GetService() const { return service_; }

  private:
    userver::storages::postgres::ClusterPtr cluster_;
    PgLoyaltyRepository loyalties_;
    LoyaltyService service_;
};

}
