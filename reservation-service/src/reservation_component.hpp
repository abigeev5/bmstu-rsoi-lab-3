#pragma once

#include <string_view>

#include <userver/components/component_base.hpp>
#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/storages/postgres/cluster.hpp>

#include "domain/reservation_service.hpp"
#include "storage/pg_repositories.hpp"

namespace reservation {

class ReservationComponent final : public userver::components::ComponentBase {
  public:
    static constexpr std::string_view kName = "reservation-service";

    ReservationComponent(const userver::components::ComponentConfig& config,
                         const userver::components::ComponentContext& context);

    const ReservationService& GetService() const { return service_; }

  private:
    userver::storages::postgres::ClusterPtr cluster_;
    PgHotelRepository hotels_;
    PgReservationRepository reservations_;
    ReservationService service_;
};

}
