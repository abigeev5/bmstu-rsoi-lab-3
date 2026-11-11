#pragma once

#include <optional>
#include <string>
#include <vector>

#include "domain/repositories.hpp"

namespace reservation {

class ReservationService {
  public:
    ReservationService(const IHotelRepository& hotels, const IReservationRepository& reservations);

    HotelPage ListHotels(int page, int size) const;
    std::optional<Hotel> GetHotel(const std::string& hotel_uid) const;
    std::vector<Reservation> GetReservations(const std::string& username) const;
    std::optional<Reservation> GetReservation(const std::string& username,
                                              const std::string& reservation_uid) const;
    std::optional<Reservation> CreateReservation(const NewReservation& reservation) const;
    bool CancelReservation(const std::string& username, const std::string& reservation_uid) const;

  private:
    const IHotelRepository& hotels_;
    const IReservationRepository& reservations_;
};

}
