#include "api/handlers.hpp"

#include <userver/formats/json/value_builder.hpp>

#include "common/http.hpp"
#include "loyalty_component.hpp"

namespace loyalty {

namespace {

using userver::server::http::HttpRequest;
using userver::server::http::HttpStatus;

const LoyaltyService& GetService(const userver::components::ComponentContext& context) {
    return context.FindComponent<LoyaltyComponent>().GetService();
}

std::string LoyaltyResponse(const HttpRequest& request, const Loyalty& loyalty) {
    userver::formats::json::ValueBuilder json;
    json["status"] = loyalty.status;
    json["discount"] = loyalty.discount;
    json["reservationCount"] = loyalty.reservation_count;
    return rsoi::JsonResponse(request, json.ExtractValue());
}

std::string MissingUserName(const HttpRequest& request) {
    return rsoi::ErrorResponse(request, HttpStatus::kBadRequest, "X-User-Name header is required");
}

}

LoyaltyHandler::LoyaltyHandler(const userver::components::ComponentConfig& config,
                               const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context), service_(GetService(context)) {}

std::string LoyaltyHandler::HandleRequestThrow(
    const HttpRequest& request,
    [[maybe_unused]] userver::server::request::RequestContext& context) const {
    const auto& username = request.GetHeader(rsoi::kUserNameHeader);
    if (username.empty()) {
        return MissingUserName(request);
    }
    return LoyaltyResponse(request, service_.GetLoyalty(username));
}

LoyaltyCounterHandler::LoyaltyCounterHandler(const userver::components::ComponentConfig& config,
                                             const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context), service_(GetService(context)) {}

std::string LoyaltyCounterHandler::HandleRequestThrow(
    const HttpRequest& request,
    [[maybe_unused]] userver::server::request::RequestContext& context) const {
    const auto& username = request.GetHeader(rsoi::kUserNameHeader);
    if (username.empty()) {
        return MissingUserName(request);
    }
    const auto& action = request.GetPathArg("action");
    if (action == "increment") {
        return LoyaltyResponse(request, service_.Increment(username));
    }
    if (action == "decrement") {
        return LoyaltyResponse(request, service_.Decrement(username));
    }
    return rsoi::ErrorResponse(request, HttpStatus::kNotFound, "Unknown action");
}

}
