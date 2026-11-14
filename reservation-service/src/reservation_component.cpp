#include "reservation_component.hpp"

#include <userver/storages/postgres/component.hpp>

namespace reservation {

ReservationComponent::ReservationComponent(const userver::components::ComponentConfig& config,
                                           const userver::components::ComponentContext& context)
    : ComponentBase(config, context),
      cluster_(context.FindComponent<userver::components::Postgres>("postgres-db").GetCluster()),
      hotels_(cluster_), reservations_(cluster_), service_(hotels_, reservations_) {
    CreateSchema(*cluster_);
}

}
