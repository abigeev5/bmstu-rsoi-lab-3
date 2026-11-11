#include "api/handlers.hpp"

#include <algorithm>

#include <userver/formats/json/exception.hpp>
#include <userver/formats/json/serialize.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/formats/serialize/common_containers.hpp>

#include "common/http.hpp"
#include "reservation_component.hpp"

namespace reservation {

namespace {

using userver::formats::json::ValueBuilder;
using userver::server::http::HttpMethod;
using userver::server::http::HttpRequest;
using userver::server::http::HttpStatus;

constexpr int kMaxPageSize = 100;

const ReservationService& GetService(const userver::components::ComponentContext& context) {
    return context.FindComponent<ReservationComponent>().GetService();
}

ValueBuilder HotelToJson(const Hotel& hotel) {
    ValueBuilder json;
    json["hotelUid"] = hotel.hotel_uid;
    json["name"] = hotel.name;
    json["country"] = hotel.country;
    json["city"] = hotel.city;
    json["address"] = hotel.address;
    json["stars"] = hotel.stars;
    json["price"] = hotel.price;
    return json;
}

ValueBuilder ReservationToJson(const Reservation& reservation) {
    const auto& hotel = reservation.hotel;
    ValueBuilder json;
    json["reservationUid"] = reservation.reservation_uid;
    json["hotel"]["hotelUid"] = hotel.hotel_uid;
    json["hotel"]["name"] = hotel.name;
    json["hotel"]["fullAddress"] = hotel.country + ", " + hotel.city + ", " + hotel.address;
    json["hotel"]["stars"] = hotel.stars;
    json["startDate"] = reservation.start_date;
    json["endDate"] = reservation.end_date;
    json["status"] = reservation.status;
    json["paymentUid"] = reservation.payment_uid;
    return json;
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
    const auto size = rsoi::IntArg(request, "size", kMaxPageSize);
    if (!page || !size) {
        return rsoi::ErrorResponse(request, HttpStatus::kBadRequest,
                                   "page and size must be numbers");
    }
    const int page_number = std::max(*page, 1);
    const int page_size = std::clamp(*size, 1, kMaxPageSize);

    const auto result = service_.ListHotels(page_number, page_size);
    ValueBuilder json;
    json["page"] = page_number;
    json["pageSize"] = result.items.size();
    json["totalElements"] = result.total;
    json["items"] = ValueBuilder{userver::formats::common::Type::kArray};
    for (const auto& hotel : result.items) {
        json["items"].PushBack(HotelToJson(hotel));
    }
    return rsoi::JsonResponse(request, json.ExtractValue());
}

HotelHandler::HotelHandler(const userver::components::ComponentConfig& config,
                           const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context), service_(GetService(context)) {}

std::string HotelHandler::HandleRequestThrow(
    const HttpRequest& request,
    [[maybe_unused]] userver::server::request::RequestContext& context) const {
    const auto& hotel_uid = request.GetPathArg("hotelUid");
    const auto hotel =
        rsoi::IsUuid(hotel_uid) ? service_.GetHotel(hotel_uid) : std::optional<Hotel>{};
    if (!hotel) {
        return rsoi::ErrorResponse(request, HttpStatus::kNotFound, "Hotel not found");
    }
    return rsoi::JsonResponse(request, HotelToJson(*hotel).ExtractValue());
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
        ValueBuilder json{userver::formats::common::Type::kArray};
        for (const auto& reservation : service_.GetReservations(username)) {
            json.PushBack(ReservationToJson(reservation));
        }
        return rsoi::JsonResponse(request, json.ExtractValue());
    }

    NewReservation reservation;
    try {
        const auto body = userver::formats::json::FromString(request.RequestBody());
        reservation = NewReservation{
            .username = username,
            .hotel_uid = body["hotelUid"].As<std::string>(),
            .start_date = body["startDate"].As<std::string>(),
            .end_date = body["endDate"].As<std::string>(),
            .payment_uid = body["paymentUid"].As<std::string>(),
        };
    } catch (const userver::formats::json::Exception& e) {
        return rsoi::ErrorResponse(request, HttpStatus::kBadRequest, e.what());
    }
    if (!rsoi::IsUuid(reservation.hotel_uid) || !rsoi::IsUuid(reservation.payment_uid)) {
        return rsoi::ErrorResponse(request, HttpStatus::kBadRequest,
                                   "hotelUid and paymentUid must be UUIDs");
    }

    const auto created = service_.CreateReservation(reservation);
    if (!created) {
        return rsoi::ErrorResponse(request, HttpStatus::kNotFound, "Hotel not found");
    }
    return rsoi::JsonResponse(request, ReservationToJson(*created).ExtractValue(),
                              HttpStatus::kCreated);
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
    if (!rsoi::IsUuid(reservation_uid)) {
        return ReservationNotFound(request);
    }

    if (request.GetMethod() == HttpMethod::kDelete) {
        if (!service_.CancelReservation(username, reservation_uid)) {
            return ReservationNotFound(request);
        }
        return rsoi::NoContentResponse(request);
    }

    const auto reservation = service_.GetReservation(username, reservation_uid);
    if (!reservation) {
        return ReservationNotFound(request);
    }
    return rsoi::JsonResponse(request, ReservationToJson(*reservation).ExtractValue());
}

}
