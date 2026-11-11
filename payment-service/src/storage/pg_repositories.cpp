#include "storage/pg_repositories.hpp"

#include <utility>

#include <userver/storages/postgres/io/row_types.hpp>
#include <userver/storages/postgres/query.hpp>

namespace payment {

namespace {

namespace pg = userver::storages::postgres;

const pg::Query kCreatePayment{R"(
    CREATE TABLE IF NOT EXISTS payment
    (
        id          SERIAL PRIMARY KEY,
        payment_uid uuid        NOT NULL UNIQUE,
        status      VARCHAR(20) NOT NULL
            CHECK (status IN ('PAID', 'CANCELED')),
        price       INT         NOT NULL
    ))"};

const pg::Query kInsertPayment{R"(
    INSERT INTO payment (payment_uid, status, price)
    VALUES (gen_random_uuid(), 'PAID', $1)
    RETURNING payment_uid::text, status, price)"};

const pg::Query kFindPayment{R"(
    SELECT payment_uid::text, status, price
    FROM payment
    WHERE payment_uid = $1::uuid)"};

const pg::Query kSetStatus{R"(
    UPDATE payment
    SET status = $2
    WHERE payment_uid = $1::uuid)"};

}

void CreateSchema(pg::Cluster& cluster) {
    cluster.Execute(pg::ClusterHostType::kMaster, kCreatePayment);
}

PgPaymentRepository::PgPaymentRepository(pg::ClusterPtr cluster) : cluster_(std::move(cluster)) {}

Payment PgPaymentRepository::Create(int price) const {
    return cluster_->Execute(pg::ClusterHostType::kMaster, kInsertPayment, price)
        .AsSingleRow<Payment>(pg::kRowTag);
}

std::optional<Payment> PgPaymentRepository::Find(const std::string& payment_uid) const {
    return cluster_->Execute(pg::ClusterHostType::kMaster, kFindPayment, payment_uid)
        .AsOptionalSingleRow<Payment>(pg::kRowTag);
}

bool PgPaymentRepository::SetStatus(const std::string& payment_uid, std::string_view status) const {
    const auto result = cluster_->Execute(pg::ClusterHostType::kMaster, kSetStatus, payment_uid,
                                          std::string{status});
    return result.RowsAffected() > 0;
}

}
