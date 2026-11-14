#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "domain/models.hpp"

namespace reservation {

class IHotelRepository {
  public:
    virtual ~IHotelRepository() = default;

    virtual HotelPage List(int page, int size) const = 0;
    virtual std::optional<Hotel> FindByUid(const std::string& hotel_uid) const = 0;
};

class IReservationRepository {
  public:
    virtual ~IReservationRepository() = default;

    virtual std::vector<Reservation> ListByUser(const std::string& username) const = 0;
    virtual std::optional<Reservation> Find(const std::string& username,
                                            const std::string& reservation_uid) const = 0;
    virtual std::optional<Reservation> Create(const NewReservation& reservation) const = 0;
    virtual bool SetStatus(const std::string& username, const std::string& reservation_uid,
                           std::string_view status) const = 0;
};

}
