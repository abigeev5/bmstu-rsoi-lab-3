#include "booking_component.hpp"

#include <chrono>
#include <cstdint>
#include <string>

#include <userver/clients/http/component.hpp>
#include <userver/yaml_config/merge_schemas.hpp>

namespace gateway {

namespace {

userver::clients::http::Client&
GetHttpClient(const userver::components::ComponentContext& context) {
    return context.FindComponent<userver::components::HttpClient>().GetHttpClient();
}

std::chrono::milliseconds GetTimeout(const userver::components::ComponentConfig& config) {
    return std::chrono::milliseconds{config["request-timeout-ms"].As<std::int64_t>()};
}

}

BookingComponent::BookingComponent(const userver::components::ComponentConfig& config,
                                   const userver::components::ComponentContext& context)
    : ComponentBase(config, context),
      reservations_(GetHttpClient(context), config["reservation-url"].As<std::string>(),
                    GetTimeout(config)),
      payments_(GetHttpClient(context), config["payment-url"].As<std::string>(),
                GetTimeout(config)),
      loyalty_(GetHttpClient(context), config["loyalty-url"].As<std::string>(), GetTimeout(config)),
      service_(reservations_, payments_, loyalty_) {}

userver::yaml_config::Schema BookingComponent::GetStaticConfigSchema() {
    return userver::yaml_config::MergeSchemas<ComponentBase>(R"(
type: object
description: Booking orchestration over the backend services
additionalProperties: false
properties:
    reservation-url:
        type: string
        description: base URL of the Reservation Service
    payment-url:
        type: string
        description: base URL of the Payment Service
    loyalty-url:
        type: string
        description: base URL of the Loyalty Service
    request-timeout-ms:
        type: integer
        description: timeout of a request to a backend service
)");
}

}
