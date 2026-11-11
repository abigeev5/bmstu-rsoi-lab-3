#pragma once

#include <optional>
#include <string>
#include <string_view>

#include <userver/formats/json/value.hpp>
#include <userver/server/http/http_request.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/utils/zstring_view.hpp>

namespace rsoi {

inline constexpr userver::utils::zstring_view kUserNameHeader = "X-User-Name";

std::string
JsonResponse(const userver::server::http::HttpRequest& request,
             const userver::formats::json::Value& body,
             userver::server::http::HttpStatus status = userver::server::http::HttpStatus::kOk);

std::string ErrorResponse(const userver::server::http::HttpRequest& request,
                          userver::server::http::HttpStatus status, std::string_view message);

std::string NoContentResponse(const userver::server::http::HttpRequest& request);

bool IsUuid(std::string_view value);

std::optional<int> IntArg(const userver::server::http::HttpRequest& request, std::string_view name,
                          int default_value);

}
