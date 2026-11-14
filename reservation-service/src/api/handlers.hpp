#pragma once

#include <string>
#include <string_view>

#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/server/handlers/http_handler_base.hpp>

#include "domain/reservation_service.hpp"

namespace reservation {

// GET /api/v1/hotels?page=&size=
class HotelsHandler final : public userver::server::handlers::HttpHandlerBase {
  public:
    static constexpr std::string_view kName = "handler-hotels";

    HotelsHandler(const userver::components::ComponentConfig& config,
                  const userver::components::ComponentContext& context);

    std::string
    HandleRequestThrow(const userver::server::http::HttpRequest& request,
                       userver::server::request::RequestContext& context) const override;

  private:
    const ReservationService& service_;
};

// GET /api/v1/hotels/{hotelUid}
class HotelHandler final : public userver::server::handlers::HttpHandlerBase {
  public:
    static constexpr std::string_view kName = "handler-hotel";

    HotelHandler(const userver::components::ComponentConfig& config,
                 const userver::components::ComponentContext& context);

    std::string
    HandleRequestThrow(const userver::server::http::HttpRequest& request,
                       userver::server::request::RequestContext& context) const override;

  private:
    const ReservationService& service_;
};

// GET, POST /api/v1/reservations
class ReservationsHandler final : public userver::server::handlers::HttpHandlerBase {
  public:
    static constexpr std::string_view kName = "handler-reservations";

    ReservationsHandler(const userver::components::ComponentConfig& config,
                        const userver::components::ComponentContext& context);

    std::string
    HandleRequestThrow(const userver::server::http::HttpRequest& request,
                       userver::server::request::RequestContext& context) const override;

  private:
    const ReservationService& service_;
};

// GET, DELETE /api/v1/reservations/{reservationUid}
class ReservationHandler final : public userver::server::handlers::HttpHandlerBase {
  public:
    static constexpr std::string_view kName = "handler-reservation";

    ReservationHandler(const userver::components::ComponentConfig& config,
                       const userver::components::ComponentContext& context);

    std::string
    HandleRequestThrow(const userver::server::http::HttpRequest& request,
                       userver::server::request::RequestContext& context) const override;

  private:
    const ReservationService& service_;
};

}
