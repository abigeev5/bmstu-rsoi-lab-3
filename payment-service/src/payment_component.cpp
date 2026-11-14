#include "payment_component.hpp"

#include <userver/storages/postgres/component.hpp>

namespace payment {

PaymentComponent::PaymentComponent(const userver::components::ComponentConfig& config,
                                   const userver::components::ComponentContext& context)
    : ComponentBase(config, context),
      cluster_(context.FindComponent<userver::components::Postgres>("postgres-db").GetCluster()),
      payments_(cluster_), service_(payments_) {
    CreateSchema(*cluster_);
}

}
