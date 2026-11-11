#include "api/handlers.hpp"

#include <algorithm>

#include <userver/formats/json/exception.hpp>
#include <userver/formats/json/serialize.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/formats/serialize/common_containers.hpp>

#include "booking_component.hpp"
#include "common/http.hpp"

namespace gateway {

namespace {

using userver::formats::json::ValueBuilder;
using userver::server::http::HttpMethod;
using userver::server::http::HttpRequest;
using userver::server::http::HttpStatus;

constexpr int kDefaultPageSize = 10;
constexpr int kMaxPageSize = 100;

const BookingService& GetService(const userver::components::ComponentContext& context) {
    return context.FindComponent<BookingComponent>().GetService();
}

ValueBuilder PaymentToJson(const Payment& payment) {
    ValueBuilder json;
    json["status"] = payment.status;
    json["price"] = payment.price;
    return json;
}

ValueBuilder ReservationToJson(const ReservationView& view) {
    const auto& reservation = view.reservation;
    ValueBuilder json;
    json["reservationUid"] = reservation.reservation_uid;
    json["hotel"]["hotelUid"] = reservation.hotel.hotel_uid;
    json["hotel"]["name"] = reservation.hotel.name;
    json["hotel"]["fullAddress"] = reservation.hotel.full_address;
    json["hotel"]["stars"] = reservation.hotel.stars;
    json["startDate"] = reservation.start_date;
    json["endDate"] = reservation.end_date;
    json["status"] = reservation.status;
    if (view.payment) {
        json["payment"] = PaymentToJson(*view.payment);
    }
    return json;
}

ValueBuilder ReservationsToJson(const std::vector<ReservationView>& views) {
    ValueBuilder json{userver::formats::common::Type::kArray};
    for (const auto& view : views) {
        json.PushBack(ReservationToJson(view));
    }
    return json;
}

ValueBuilder LoyaltyToJson(const Loyalty& loyalty) {
    ValueBuilder json;
    json["status"] = loyalty.status;
    json["discount"] = loyalty.discount;
    json["reservationCount"] = loyalty.reservation_count;
    return json;
}

ValueBuilder BookingToJson(const BookingResult& booking) {
    ValueBuilder json;
    json["reservationUid"] = booking.reservation.reservation_uid;
    json["hotelUid"] = booking.reservation.hotel.hotel_uid;
    json["startDate"] = booking.reservation.start_date;
    json["endDate"] = booking.reservation.end_date;
    json["discount"] = booking.discount;
    json["status"] = booking.reservation.status;
    json["payment"] = PaymentToJson(booking.payment);
    return json;
}

std::string ValidationErrorResponse(const HttpRequest& request, const std::string& field,
                                    const std::string& error) {
    ValueBuilder json;
    json["message"] = "Validation failed";
    ValueBuilder description;
    description["field"] = field;
    description["error"] = error;
    json["errors"] = ValueBuilder{userver::formats::common::Type::kArray};
    json["errors"].PushBack(std::move(description));
    return rsoi::JsonResponse(request, json.ExtractValue(), HttpStatus::kBadRequest);
}

std::string MissingUserName(const HttpRequest& request) {
    return rsoi::ErrorResponse(request, HttpStatus::kBadRequest, "X-User-Name header is required");
}

std::string ReservationNotFound(const HttpRequest& request) {
    return rsoi::ErrorResponse(request, HttpStatus::kNotFound, "Reservation not found");
}

}

HotelsHandler::HotelsHandler(const userver::components::ComponentConfig& config,
                             const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context), service_(GetService(context)) {}

std::string HotelsHandler::HandleRequestThrow(
    const HttpRequest& request,
    [[maybe_unused]] userver::server::request::RequestContext& context) const {
    const auto page = rsoi::IntArg(request, "page", 1);
    if (!page || *page < 0) {
        return ValidationErrorResponse(request, "page", "must be a non-negative number");
    }
    const auto size = rsoi::IntArg(request, "size", kDefaultPageSize);
    if (!size || *size < 1 || *size > kMaxPageSize) {
        return ValidationErrorResponse(request, "size", "must be a number from 1 to 100");
    }
    return rsoi::JsonResponse(request, service_.ListHotels(std::max(*page, 1), *size));
}

MeHandler::MeHandler(const userver::components::ComponentConfig& config,
                     const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context), service_(GetService(context)) {}

std::string MeHandler::HandleRequestThrow(
    const HttpRequest& request,
    [[maybe_unused]] userver::server::request::RequestContext& context) const {
    const auto& username = request.GetHeader(rsoi::kUserNameHeader);
    if (username.empty()) {
        return MissingUserName(request);
    }
    const auto info = service_.GetUserInfo(username);
    ValueBuilder json;
    json["reservations"] = ReservationsToJson(info.reservations);
    json["loyalty"] = LoyaltyToJson(info.loyalty);
    return rsoi::JsonResponse(request, json.ExtractValue());
}

ReservationsHandler::ReservationsHandler(const userver::components::ComponentConfig& config,
                                         const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context), service_(GetService(context)) {}

std::string ReservationsHandler::HandleRequestThrow(
    const HttpRequest& request,
    [[maybe_unused]] userver::server::request::RequestContext& context) const {
    const auto& username = request.GetHeader(rsoi::kUserNameHeader);
    if (username.empty()) {
        return MissingUserName(request);
    }

    if (request.GetMethod() == HttpMethod::kGet) {
        return rsoi::JsonResponse(
            request, ReservationsToJson(service_.GetReservations(username)).ExtractValue());
    }

    BookingRequest booking;
    try {
        const auto body = userver::formats::json::FromString(request.RequestBody());
        booking = BookingRequest{
            .hotel_uid = body["hotelUid"].As<std::string>(),
            .start_date = body["startDate"].As<std::string>(),
            .end_date = body["endDate"].As<std::string>(),
        };
    } catch (const userver::formats::json::Exception& e) {
        return ValidationErrorResponse(request, "body", e.what());
    }

    try {
        return rsoi::JsonResponse(request,
                                  BookingToJson(service_.Book(username, booking)).ExtractValue());
    } catch (const ValidationError& e) {
        return ValidationErrorResponse(request, e.Field(), e.what());
    }
}

ReservationHandler::ReservationHandler(const userver::components::ComponentConfig& config,
                                       const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context), service_(GetService(context)) {}

std::string ReservationHandler::HandleRequestThrow(
    const HttpRequest& request,
    [[maybe_unused]] userver::server::request::RequestContext& context) const {
    const auto& username = request.GetHeader(rsoi::kUserNameHeader);
    if (username.empty()) {
        return MissingUserName(request);
    }
    const auto& reservation_uid = request.GetPathArg("reservationUid");

    if (request.GetMethod() == HttpMethod::kDelete) {
        if (!service_.Cancel(username, reservation_uid)) {
            return ReservationNotFound(request);
        }
        return rsoi::NoContentResponse(request);
    }

    const auto view = service_.GetReservation(username, reservation_uid);
    if (!view) {
        return ReservationNotFound(request);
    }
    return rsoi::JsonResponse(request, ReservationToJson(*view).ExtractValue());
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
    return rsoi::JsonResponse(request, LoyaltyToJson(service_.GetLoyalty(username)).ExtractValue());
}

}
