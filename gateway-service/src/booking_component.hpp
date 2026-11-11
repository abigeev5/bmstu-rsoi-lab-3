#pragma once

#include <string_view>

#include <userver/components/component_base.hpp>
#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/yaml_config/schema.hpp>

#include "clients/http_clients.hpp"
#include "domain/booking_service.hpp"

namespace gateway {

class BookingComponent final : public userver::components::ComponentBase {
  public:
    static constexpr std::string_view kName = "booking-service";

    BookingComponent(const userver::components::ComponentConfig& config,
                     const userver::components::ComponentContext& context);

    const BookingService& GetService() const { return service_; }

    static userver::yaml_config::Schema GetStaticConfigSchema();

  private:
    ReservationClient reservations_;
    PaymentClient payments_;
    LoyaltyClient loyalty_;
    BookingService service_;
};

}

template <>
inline constexpr bool userver::components::kHasValidate<gateway::BookingComponent> = true;
