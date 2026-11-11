#include "domain/reservation_service.hpp"

namespace reservation {

ReservationService::ReservationService(const IHotelRepository& hotels,
                                       const IReservationRepository& reservations)
    : hotels_(hotels), reservations_(reservations) {}

HotelPage ReservationService::ListHotels(int page, int size) const {
    return hotels_.List(page, size);
}

std::optional<Hotel> ReservationService::GetHotel(const std::string& hotel_uid) const {
    return hotels_.FindByUid(hotel_uid);
}

std::vector<Reservation> ReservationService::GetReservations(const std::string& username) const {
    return reservations_.ListByUser(username);
}

std::optional<Reservation>
ReservationService::GetReservation(const std::string& username,
                                   const std::string& reservation_uid) const {
    return reservations_.Find(username, reservation_uid);
}

std::optional<Reservation>
ReservationService::CreateReservation(const NewReservation& reservation) const {
    return reservations_.Create(reservation);
}

bool ReservationService::CancelReservation(const std::string& username,
                                           const std::string& reservation_uid) const {
    return reservations_.SetStatus(username, reservation_uid, kStatusCanceled);
}

}
