#include "clients/http_clients.hpp"

#include <chrono>
#include <memory>
#include <utility>

#include <userver/clients/http/request.hpp>
#include <userver/clients/http/response.hpp>
#include <userver/formats/json/serialize.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/formats/parse/common_containers.hpp>
#include <userver/formats/serialize/common_containers.hpp>
#include <userver/http/common_headers.hpp>

#include "common/http.hpp"

namespace gateway {

namespace {

using userver::clients::http::HttpMethod;
using userver::clients::http::Response;
using userver::formats::json::Value;

std::shared_ptr<Response> Perform(userver::clients::http::Client& http,
                                  std::chrono::milliseconds timeout, HttpMethod method,
                                  std::string url, const std::string& username = {},
                                  std::string body = {}) {
    userver::clients::http::Headers headers;
    if (!username.empty()) {
        headers.insert_or_assign(std::string{rsoi::kUserNameHeader}, username);
    }
    if (!body.empty()) {
        headers.insert_or_assign(userver::http::headers::kContentType, "application/json");
    }
    auto request = http.CreateRequest();
    request.method(method).url(std::move(url)).headers(headers).timeout(timeout);
    if (!body.empty()) {
        request.data(std::move(body));
    }
    return request.perform();
}

bool IsNotFound(const Response& response) {
    return response.status_code() == userver::http::StatusCode::kNotFound;
}

Value JsonBody(const Response& response) {
    response.raise_for_status();
    return userver::formats::json::FromString(response.body_view());
}

Hotel ParseHotel(const Value& json) {
    return Hotel{
        .hotel_uid = json["hotelUid"].As<std::string>(),
        .price = json["price"].As<std::int64_t>(),
    };
}

Reservation ParseReservation(const Value& json) {
    const auto hotel = json["hotel"];
    return Reservation{
        .reservation_uid = json["reservationUid"].As<std::string>(),
        .hotel =
            HotelInfo{
                .hotel_uid = hotel["hotelUid"].As<std::string>(),
                .name = hotel["name"].As<std::string>(),
                .full_address = hotel["fullAddress"].As<std::string>(),
                .stars = hotel["stars"].As<std::optional<int>>(),
            },
        .start_date = json["startDate"].As<std::string>(),
        .end_date = json["endDate"].As<std::string>(),
        .status = json["status"].As<std::string>(),
        .payment_uid = json["paymentUid"].As<std::string>(),
    };
}

Payment ParsePayment(const Value& json) {
    return Payment{
        .payment_uid = json["paymentUid"].As<std::string>(),
        .status = json["status"].As<std::string>(),
        .price = json["price"].As<std::int64_t>(),
    };
}

Loyalty ParseLoyalty(const Value& json) {
    return Loyalty{
        .status = json["status"].As<std::string>(),
        .discount = json["discount"].As<int>(),
        .reservation_count = json["reservationCount"].As<int>(),
    };
}

}

ReservationClient::ReservationClient(userver::clients::http::Client& http, std::string base_url,
                                     std::chrono::milliseconds timeout)
    : http_(http), base_url_(std::move(base_url)), timeout_(timeout) {}

Value ReservationClient::ListHotels(int page, int size) const {
    const auto url =
        base_url_ + "/api/v1/hotels?page=" + std::to_string(page) + "&size=" + std::to_string(size);
    return JsonBody(*Perform(http_, timeout_, HttpMethod::kGet, url));
}

std::optional<Hotel> ReservationClient::GetHotel(const std::string& hotel_uid) const {
    const auto response =
        Perform(http_, timeout_, HttpMethod::kGet, base_url_ + "/api/v1/hotels/" + hotel_uid);
    if (IsNotFound(*response)) {
        return std::nullopt;
    }
    return ParseHotel(JsonBody(*response));
}

std::vector<Reservation> ReservationClient::GetReservations(const std::string& username) const {
    const auto json = JsonBody(
        *Perform(http_, timeout_, HttpMethod::kGet, base_url_ + "/api/v1/reservations", username));
    std::vector<Reservation> reservations;
    for (const auto& item : json) {
        reservations.push_back(ParseReservation(item));
    }
    return reservations;
}

std::optional<Reservation>
ReservationClient::GetReservation(const std::string& username,
                                  const std::string& reservation_uid) const {
    const auto response = Perform(http_, timeout_, HttpMethod::kGet,
                                  base_url_ + "/api/v1/reservations/" + reservation_uid, username);
    if (IsNotFound(*response)) {
        return std::nullopt;
    }
    return ParseReservation(JsonBody(*response));
}

Reservation ReservationClient::CreateReservation(const std::string& username,
                                                 const NewReservation& reservation) const {
    userver::formats::json::ValueBuilder body;
    body["hotelUid"] = reservation.hotel_uid;
    body["startDate"] = reservation.start_date;
    body["endDate"] = reservation.end_date;
    body["paymentUid"] = reservation.payment_uid;
    const auto response =
        Perform(http_, timeout_, HttpMethod::kPost, base_url_ + "/api/v1/reservations", username,
                userver::formats::json::ToString(body.ExtractValue()));
    return ParseReservation(JsonBody(*response));
}

void ReservationClient::CancelReservation(const std::string& username,
                                          const std::string& reservation_uid) const {
    Perform(http_, timeout_, HttpMethod::kDelete,
            base_url_ + "/api/v1/reservations/" + reservation_uid, username)
        ->raise_for_status();
}

PaymentClient::PaymentClient(userver::clients::http::Client& http, std::string base_url,
                             std::chrono::milliseconds timeout)
    : http_(http), base_url_(std::move(base_url)), timeout_(timeout) {}

Payment PaymentClient::CreatePayment(std::int64_t price) const {
    userver::formats::json::ValueBuilder body;
    body["price"] = price;
    const auto response =
        Perform(http_, timeout_, HttpMethod::kPost, base_url_ + "/api/v1/payments", {},
                userver::formats::json::ToString(body.ExtractValue()));
    return ParsePayment(JsonBody(*response));
}

std::optional<Payment> PaymentClient::GetPayment(const std::string& payment_uid) const {
    const auto response =
        Perform(http_, timeout_, HttpMethod::kGet, base_url_ + "/api/v1/payments/" + payment_uid);
    if (IsNotFound(*response)) {
        return std::nullopt;
    }
    return ParsePayment(JsonBody(*response));
}

void PaymentClient::CancelPayment(const std::string& payment_uid) const {
    Perform(http_, timeout_, HttpMethod::kDelete, base_url_ + "/api/v1/payments/" + payment_uid)
        ->raise_for_status();
}

LoyaltyClient::LoyaltyClient(userver::clients::http::Client& http, std::string base_url,
                             std::chrono::milliseconds timeout)
    : http_(http), base_url_(std::move(base_url)), timeout_(timeout) {}

Loyalty LoyaltyClient::GetLoyalty(const std::string& username) const {
    return ParseLoyalty(JsonBody(
        *Perform(http_, timeout_, HttpMethod::kGet, base_url_ + "/api/v1/loyalty", username)));
}

void LoyaltyClient::Increment(const std::string& username) const {
    Perform(http_, timeout_, HttpMethod::kPost, base_url_ + "/api/v1/loyalty/increment", username)
        ->raise_for_status();
}

void LoyaltyClient::Decrement(const std::string& username) const {
    Perform(http_, timeout_, HttpMethod::kPost, base_url_ + "/api/v1/loyalty/decrement", username)
        ->raise_for_status();
}

}
