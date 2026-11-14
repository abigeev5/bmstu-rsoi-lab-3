#include "api/handlers.hpp"

#include <userver/formats/json/exception.hpp>
#include <userver/formats/json/serialize.hpp>
#include <userver/formats/json/value_builder.hpp>

#include "common/http.hpp"
#include "payment_component.hpp"

namespace payment {

namespace {

using userver::formats::json::ValueBuilder;
using userver::server::http::HttpMethod;
using userver::server::http::HttpRequest;
using userver::server::http::HttpStatus;

const PaymentService& GetService(const userver::components::ComponentContext& context) {
    return context.FindComponent<PaymentComponent>().GetService();
}

userver::formats::json::Value PaymentToJson(const Payment& payment) {
    ValueBuilder json;
    json["paymentUid"] = payment.payment_uid;
    json["status"] = payment.status;
    json["price"] = payment.price;
    return json.ExtractValue();
}

std::string PaymentNotFound(const HttpRequest& request) {
    return rsoi::ErrorResponse(request, HttpStatus::kNotFound, "Payment not found");
}

}

PaymentsHandler::PaymentsHandler(const userver::components::ComponentConfig& config,
                                 const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context), service_(GetService(context)) {}

std::string PaymentsHandler::HandleRequestThrow(
    const HttpRequest& request,
    [[maybe_unused]] userver::server::request::RequestContext& context) const {
    int price = 0;
    try {
        price = userver::formats::json::FromString(request.RequestBody())["price"].As<int>();
    } catch (const userver::formats::json::Exception& e) {
        return rsoi::ErrorResponse(request, HttpStatus::kBadRequest, e.what());
    }
    if (price < 0) {
        return rsoi::ErrorResponse(request, HttpStatus::kBadRequest, "price must not be negative");
    }
    return rsoi::JsonResponse(request, PaymentToJson(service_.CreatePayment(price)),
                              HttpStatus::kCreated);
}

PaymentHandler::PaymentHandler(const userver::components::ComponentConfig& config,
                               const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context), service_(GetService(context)) {}

std::string PaymentHandler::HandleRequestThrow(
    const HttpRequest& request,
    [[maybe_unused]] userver::server::request::RequestContext& context) const {
    const auto& payment_uid = request.GetPathArg("paymentUid");
    if (!rsoi::IsUuid(payment_uid)) {
        return PaymentNotFound(request);
    }

    if (request.GetMethod() == HttpMethod::kDelete) {
        if (!service_.CancelPayment(payment_uid)) {
            return PaymentNotFound(request);
        }
        return rsoi::NoContentResponse(request);
    }

    const auto payment = service_.GetPayment(payment_uid);
    if (!payment) {
        return PaymentNotFound(request);
    }
    return rsoi::JsonResponse(request, PaymentToJson(*payment));
}

}
