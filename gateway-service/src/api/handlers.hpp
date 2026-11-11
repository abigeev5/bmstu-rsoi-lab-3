#pragma once

#include <string>
#include <string_view>

#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/server/handlers/http_handler_base.hpp>

#include "domain/booking_service.hpp"

namespace gateway {

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
    const BookingService& service_;
};

// GET /api/v1/me
class MeHandler final : public userver::server::handlers::HttpHandlerBase {
  public:
    static constexpr std::string_view kName = "handler-me";

    MeHandler(const userver::components::ComponentConfig& config,
              const userver::components::ComponentContext& context);

    std::string
    HandleRequestThrow(const userver::server::http::HttpRequest& request,
                       userver::server::request::RequestContext& context) const override;

  private:
    const BookingService& service_;
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
    const BookingService& service_;
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
    const BookingService& service_;
};

// GET /api/v1/loyalty
class LoyaltyHandler final : public userver::server::handlers::HttpHandlerBase {
  public:
    static constexpr std::string_view kName = "handler-loyalty";

    LoyaltyHandler(const userver::components::ComponentConfig& config,
                   const userver::components::ComponentContext& context);

    std::string
    HandleRequestThrow(const userver::server::http::HttpRequest& request,
                       userver::server::request::RequestContext& context) const override;

  private:
    const BookingService& service_;
};

}
