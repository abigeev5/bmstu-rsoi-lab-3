#include "storage/pg_repositories.hpp"

#include <utility>

#include <userver/storages/postgres/io/row_types.hpp>
#include <userver/storages/postgres/query.hpp>
#include <userver/storages/postgres/transaction.hpp>

#include "domain/level.hpp"

namespace loyalty {

namespace {

namespace pg = userver::storages::postgres;

const pg::Query kCreateLoyalty{R"(
    CREATE TABLE IF NOT EXISTS loyalty
    (
        id                SERIAL PRIMARY KEY,
        username          VARCHAR(80) NOT NULL UNIQUE,
        reservation_count INT         NOT NULL DEFAULT 0,
        status            VARCHAR(80) NOT NULL DEFAULT 'BRONZE'
            CHECK (status IN ('BRONZE', 'SILVER', 'GOLD')),
        discount          INT         NOT NULL
    ))"};

const pg::Query kInsertTestUser{R"(
    INSERT INTO loyalty (username, reservation_count, status, discount)
    VALUES ('Test Max', 25, 'GOLD', 10)
    ON CONFLICT (username) DO NOTHING)"};

const pg::Query kFindLoyalty{R"(
    SELECT reservation_count, status, discount
    FROM loyalty
    WHERE username = $1)"};

const pg::Query kInsertUser{R"(
    INSERT INTO loyalty (username, reservation_count, status, discount)
    VALUES ($1, 0, $2, $3)
    ON CONFLICT (username) DO NOTHING)"};

const pg::Query kChangeCount{R"(
    UPDATE loyalty
    SET reservation_count = GREATEST(reservation_count + $2, 0)
    WHERE username = $1
    RETURNING reservation_count)"};

const pg::Query kSetLevel{R"(
    UPDATE loyalty
    SET status = $2, discount = $3
    WHERE username = $1)"};

}

void CreateSchema(pg::Cluster& cluster) {
    cluster.Execute(pg::ClusterHostType::kMaster, kCreateLoyalty);
    cluster.Execute(pg::ClusterHostType::kMaster, kInsertTestUser);
}

PgLoyaltyRepository::PgLoyaltyRepository(pg::ClusterPtr cluster) : cluster_(std::move(cluster)) {}

std::optional<Loyalty> PgLoyaltyRepository::Find(const std::string& username) const {
    return cluster_->Execute(pg::ClusterHostType::kMaster, kFindLoyalty, username)
        .AsOptionalSingleRow<Loyalty>(pg::kRowTag);
}

Loyalty PgLoyaltyRepository::ChangeReservationCount(const std::string& username, int delta) const {
    auto transaction = cluster_->Begin(pg::ClusterHostType::kMaster, pg::TransactionOptions{});

    const auto initial = LevelFor(0);
    transaction.Execute(kInsertUser, username, std::string{initial.status}, initial.discount);
    const auto count = transaction.Execute(kChangeCount, username, delta).AsSingleRow<int>();
    const auto level = LevelFor(count);
    transaction.Execute(kSetLevel, username, std::string{level.status}, level.discount);
    transaction.Commit();

    return Loyalty{
        .reservation_count = count,
        .status = std::string{level.status},
        .discount = level.discount,
    };
}

}
