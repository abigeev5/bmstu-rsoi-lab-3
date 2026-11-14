#include "storage/pg_repositories.hpp"

#include <cstdint>
#include <utility>

#include <userver/storages/postgres/io/row_types.hpp>
#include <userver/storages/postgres/query.hpp>

namespace reservation {

namespace {

namespace pg = userver::storages::postgres;

const pg::Query kCreateHotels{R"(
    CREATE TABLE IF NOT EXISTS hotels
    (
        id        SERIAL PRIMARY KEY,
        hotel_uid uuid         NOT NULL UNIQUE,
        name      VARCHAR(255) NOT NULL,
        country   VARCHAR(80)  NOT NULL,
        city      VARCHAR(80)  NOT NULL,
        address   VARCHAR(255) NOT NULL,
        stars     INT,
        price     INT          NOT NULL
    ))"};

const pg::Query kCreateReservation{R"(
    CREATE TABLE IF NOT EXISTS reservation
    (
        id              SERIAL PRIMARY KEY,
        reservation_uid uuid UNIQUE NOT NULL,
        username        VARCHAR(80) NOT NULL,
        payment_uid     uuid        NOT NULL,
        hotel_id        INT REFERENCES hotels (id),
        status          VARCHAR(20) NOT NULL
            CHECK (status IN ('PAID', 'CANCELED')),
        start_date      TIMESTAMP WITH TIME ZONE,
        end_date        TIMESTAMP WITH TIME ZONE
    ))"};

const pg::Query kInsertTestHotel{R"(
    INSERT INTO hotels (hotel_uid, name, country, city, address, stars, price)
    VALUES ('049161bb-badd-4fa8-9d90-87c9a82b0668', 'Ararat Park Hyatt Moscow', 'Россия', 'Москва',
            'Неглинная ул., 4', 5, 10000)
    ON CONFLICT (hotel_uid) DO NOTHING)"};

const pg::Query kListHotels{R"(
    SELECT hotel_uid::text, name, country, city, address, stars, price
    FROM hotels
    ORDER BY id
    LIMIT $1 OFFSET $2)"};

const pg::Query kCountHotels{"SELECT COUNT(*) FROM hotels"};

const pg::Query kFindHotel{R"(
    SELECT hotel_uid::text, name, country, city, address, stars, price
    FROM hotels
    WHERE hotel_uid = $1::uuid)"};

const pg::Query kListReservations{R"(
    SELECT r.reservation_uid::text,
           h.hotel_uid::text, h.name, h.country, h.city, h.address, h.stars, h.price,
           to_char(r.start_date AT TIME ZONE 'UTC', 'YYYY-MM-DD'),
           to_char(r.end_date AT TIME ZONE 'UTC', 'YYYY-MM-DD'),
           r.status, r.payment_uid::text
    FROM reservation r
    JOIN hotels h ON h.id = r.hotel_id
    WHERE r.username = $1
    ORDER BY r.id)"};

const pg::Query kFindReservation{R"(
    SELECT r.reservation_uid::text,
           h.hotel_uid::text, h.name, h.country, h.city, h.address, h.stars, h.price,
           to_char(r.start_date AT TIME ZONE 'UTC', 'YYYY-MM-DD'),
           to_char(r.end_date AT TIME ZONE 'UTC', 'YYYY-MM-DD'),
           r.status, r.payment_uid::text
    FROM reservation r
    JOIN hotels h ON h.id = r.hotel_id
    WHERE r.username = $1 AND r.reservation_uid = $2::uuid)"};

const pg::Query kInsertReservation{R"(
    INSERT INTO reservation (reservation_uid, username, payment_uid, hotel_id, status, start_date,
                             end_date)
    SELECT gen_random_uuid(), $1, $2::uuid, id, 'PAID',
           $4::date::timestamp AT TIME ZONE 'UTC', $5::date::timestamp AT TIME ZONE 'UTC'
    FROM hotels
    WHERE hotel_uid = $3::uuid
    RETURNING reservation_uid::text)"};

const pg::Query kSetStatus{R"(
    UPDATE reservation
    SET status = $3
    WHERE username = $1 AND reservation_uid = $2::uuid)"};

struct ReservationRow {
    std::string reservation_uid;
    std::string hotel_uid;
    std::string name;
    std::string country;
    std::string city;
    std::string address;
    std::optional<int> stars;
    int price{};
    std::string start_date;
    std::string end_date;
    std::string status;
    std::string payment_uid;
};

Reservation ToReservation(ReservationRow row) {
    return Reservation{
        .reservation_uid = std::move(row.reservation_uid),
        .hotel =
            Hotel{
                .hotel_uid = std::move(row.hotel_uid),
                .name = std::move(row.name),
                .country = std::move(row.country),
                .city = std::move(row.city),
                .address = std::move(row.address),
                .stars = row.stars,
                .price = row.price,
            },
        .start_date = std::move(row.start_date),
        .end_date = std::move(row.end_date),
        .status = std::move(row.status),
        .payment_uid = std::move(row.payment_uid),
    };
}

}

void CreateSchema(pg::Cluster& cluster) {
    cluster.Execute(pg::ClusterHostType::kMaster, kCreateHotels);
    cluster.Execute(pg::ClusterHostType::kMaster, kCreateReservation);
    cluster.Execute(pg::ClusterHostType::kMaster, kInsertTestHotel);
}

PgHotelRepository::PgHotelRepository(pg::ClusterPtr cluster) : cluster_(std::move(cluster)) {}

HotelPage PgHotelRepository::List(int page, int size) const {
    const std::int64_t offset = static_cast<std::int64_t>(page - 1) * size;
    auto items = cluster_->Execute(pg::ClusterHostType::kMaster, kListHotels, size, offset)
                     .AsContainer<std::vector<Hotel>>(pg::kRowTag);
    const auto total =
        cluster_->Execute(pg::ClusterHostType::kMaster, kCountHotels).AsSingleRow<std::int64_t>();
    return HotelPage{.items = std::move(items), .total = total};
}

std::optional<Hotel> PgHotelRepository::FindByUid(const std::string& hotel_uid) const {
    return cluster_->Execute(pg::ClusterHostType::kMaster, kFindHotel, hotel_uid)
        .AsOptionalSingleRow<Hotel>(pg::kRowTag);
}

PgReservationRepository::PgReservationRepository(pg::ClusterPtr cluster)
    : cluster_(std::move(cluster)) {}

std::vector<Reservation> PgReservationRepository::ListByUser(const std::string& username) const {
    auto rows = cluster_->Execute(pg::ClusterHostType::kMaster, kListReservations, username)
                    .AsContainer<std::vector<ReservationRow>>(pg::kRowTag);
    std::vector<Reservation> reservations;
    reservations.reserve(rows.size());
    for (auto& row : rows) {
        reservations.push_back(ToReservation(std::move(row)));
    }
    return reservations;
}

std::optional<Reservation> PgReservationRepository::Find(const std::string& username,
                                                         const std::string& reservation_uid) const {
    auto row =
        cluster_->Execute(pg::ClusterHostType::kMaster, kFindReservation, username, reservation_uid)
            .AsOptionalSingleRow<ReservationRow>(pg::kRowTag);
    if (!row) {
        return std::nullopt;
    }
    return ToReservation(std::move(*row));
}

std::optional<Reservation>
PgReservationRepository::Create(const NewReservation& reservation) const {
    const auto reservation_uid =
        cluster_
            ->Execute(pg::ClusterHostType::kMaster, kInsertReservation, reservation.username,
                      reservation.payment_uid, reservation.hotel_uid, reservation.start_date,
                      reservation.end_date)
            .AsOptionalSingleRow<std::string>();
    if (!reservation_uid) {
        return std::nullopt;
    }
    return Find(reservation.username, *reservation_uid);
}

bool PgReservationRepository::SetStatus(const std::string& username,
                                        const std::string& reservation_uid,
                                        std::string_view status) const {
    const auto result = cluster_->Execute(pg::ClusterHostType::kMaster, kSetStatus, username,
                                          reservation_uid, std::string{status});
    return result.RowsAffected() > 0;
}

}
