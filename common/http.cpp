#include "common/http.hpp"

#include <cctype>
#include <cstddef>

#include <userver/formats/json/serialize.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/http/content_type.hpp>
#include <userver/utils/from_string.hpp>

namespace rsoi {

std::string JsonResponse(const userver::server::http::HttpRequest& request,
                         const userver::formats::json::Value& body,
                         userver::server::http::HttpStatus status) {
    request.SetResponseStatus(status);
    request.GetHttpResponse().SetContentType(userver::http::content_type::kApplicationJson);
    return userver::formats::json::ToString(body);
}

std::string ErrorResponse(const userver::server::http::HttpRequest& request,
                          userver::server::http::HttpStatus status, std::string_view message) {
    userver::formats::json::ValueBuilder body;
    body["message"] = std::string{message};
    return JsonResponse(request, body.ExtractValue(), status);
}

std::string NoContentResponse(const userver::server::http::HttpRequest& request) {
    request.SetResponseStatus(userver::server::http::HttpStatus::kNoContent);
    return {};
}

bool IsUuid(std::string_view value) {
    constexpr std::size_t kLength = 36;
    if (value.size() != kLength) {
        return false;
    }
    for (std::size_t i = 0; i < value.size(); ++i) {
        const bool is_dash_position = i == 8 || i == 13 || i == 18 || i == 23;
        const bool ok = is_dash_position ? value[i] == '-'
                                         : std::isxdigit(static_cast<unsigned char>(value[i])) != 0;
        if (!ok) {
            return false;
        }
    }
    return true;
}

std::optional<int> IntArg(const userver::server::http::HttpRequest& request, std::string_view name,
                          int default_value) {
    const auto& raw = request.GetArg(name);
    if (raw.empty()) {
        return default_value;
    }
    try {
        return userver::utils::FromString<int>(raw);
    } catch (const userver::utils::FromStringException&) {
        return std::nullopt;
    }
}

}
