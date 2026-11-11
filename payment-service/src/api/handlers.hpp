#pragma once

#include <string>
#include <string_view>

#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/server/handlers/http_handler_base.hpp>

#include "domain/payment_service.hpp"

namespace payment {

// POST /api/v1/payments
class PaymentsHandler final : public userver::server::handlers::HttpHandlerBase {
  public:
    static constexpr std::string_view kName = "handler-payments";

    PaymentsHandler(const userver::components::ComponentConfig& config,
                    const userver::components::ComponentContext& context);

    std::string
    HandleRequestThrow(const userver::server::http::HttpRequest& request,
                       userver::server::request::RequestContext& context) const override;

  private:
    const PaymentService& service_;
};

// GET, DELETE /api/v1/payments/{paymentUid}
class PaymentHandler final : public userver::server::handlers::HttpHandlerBase {
  public:
    static constexpr std::string_view kName = "handler-payment";

    PaymentHandler(const userver::components::ComponentConfig& config,
                   const userver::components::ComponentContext& context);

    std::string
    HandleRequestThrow(const userver::server::http::HttpRequest& request,
                       userver::server::request::RequestContext& context) const override;

  private:
    const PaymentService& service_;
};

}
