#pragma once

#include <string>
#include <string_view>

#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/server/handlers/http_handler_base.hpp>

#include "domain/loyalty_service.hpp"

namespace loyalty {

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
    const LoyaltyService& service_;
};

// POST /api/v1/loyalty/{action}
class LoyaltyCounterHandler final : public userver::server::handlers::HttpHandlerBase {
  public:
    static constexpr std::string_view kName = "handler-loyalty-counter";

    LoyaltyCounterHandler(const userver::components::ComponentConfig& config,
                          const userver::components::ComponentContext& context);

    std::string
    HandleRequestThrow(const userver::server::http::HttpRequest& request,
                       userver::server::request::RequestContext& context) const override;

  private:
    const LoyaltyService& service_;
};

}
