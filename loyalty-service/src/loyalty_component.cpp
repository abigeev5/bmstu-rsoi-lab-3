#include "loyalty_component.hpp"

#include <userver/storages/postgres/component.hpp>

namespace loyalty {

LoyaltyComponent::LoyaltyComponent(const userver::components::ComponentConfig& config,
                                   const userver::components::ComponentContext& context)
    : ComponentBase(config, context),
      cluster_(context.FindComponent<userver::components::Postgres>("postgres-db").GetCluster()),
      loyalties_(cluster_), service_(loyalties_) {
    CreateSchema(*cluster_);
}

}
