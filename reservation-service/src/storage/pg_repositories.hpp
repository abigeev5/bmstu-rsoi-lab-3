#pragma once

#include <userver/storages/postgres/cluster.hpp>

#include "domain/repositories.hpp"

namespace reservation {

void CreateSchema(userver::storages::postgres::Cluster& cluster);

class PgHotelRepository final : public IHotelRepository {
  public:
    explicit PgHotelRepository(userver::storages::postgres::ClusterPtr cluster);

    HotelPage List(int page, int size) const override;
    std::optional<Hotel> FindByUid(const std::string& hotel_uid) const override;

  private:
    userver::storages::postgres::ClusterPtr cluster_;
};

class PgReservationRepository final : public IReservationRepository {
  public:
    explicit PgReservationRepository(userver::storages::postgres::ClusterPtr cluster);

    std::vector<Reservation> ListByUser(const std::string& username) const override;
    std::optional<Reservation> Find(const std::string& username,
                                    const std::string& reservation_uid) const override;
    std::optional<Reservation> Create(const NewReservation& reservation) const override;
    bool SetStatus(const std::string& username, const std::string& reservation_uid,
                   std::string_view status) const override;

  private:
    userver::storages::postgres::ClusterPtr cluster_;
};

}
