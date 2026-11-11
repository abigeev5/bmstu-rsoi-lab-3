#include <chrono>
#include <optional>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "domain/booking_service.hpp"

namespace gateway {

namespace {

constexpr std::string_view kUser = "Test Max";
constexpr std::string_view kHotelUid = "049161bb-badd-4fa8-9d90-87c9a82b0668";
constexpr std::string_view kReservationUid = "9b4ba1f7-e5ac-465b-ace4-7b54dec20f9a";
constexpr std::string_view kPaymentUid = "5c4a6c2a-6d3c-4b8a-9d0e-0b1f3c2d4e5f";

using CallLog = std::vector<std::string>;

class FakeReservationClient final : public IReservationClient {
  public:
    explicit FakeReservationClient(CallLog& log) : log_(log) {}

    userver::formats::json::Value ListHotels([[maybe_unused]] int page,
                                             [[maybe_unused]] int size) const override {
        return {};
    }

    std::optional<Hotel> GetHotel([[maybe_unused]] const std::string& hotel_uid) const override {
        log_.emplace_back("GetHotel");
        return hotel;
    }

    std::vector<Reservation>
    GetReservations([[maybe_unused]] const std::string& username) const override {
        return {};
    }

    std::optional<Reservation>
    GetReservation([[maybe_unused]] const std::string& username,
                   [[maybe_unused]] const std::string& reservation_uid) const override {
        log_.emplace_back("GetReservation");
        return reservation;
    }

    Reservation CreateReservation([[maybe_unused]] const std::string& username,
                                  const NewReservation& new_reservation) const override {
        log_.emplace_back("CreateReservation");
        created = new_reservation;
        Reservation result;
        result.reservation_uid = kReservationUid;
        result.hotel.hotel_uid = new_reservation.hotel_uid;
        result.start_date = new_reservation.start_date;
        result.end_date = new_reservation.end_date;
        result.status = "PAID";
        result.payment_uid = new_reservation.payment_uid;
        return result;
    }

    void CancelReservation([[maybe_unused]] const std::string& username,
                           [[maybe_unused]] const std::string& reservation_uid) const override {
        log_.emplace_back("CancelReservation");
    }

    std::optional<Hotel> hotel = Hotel{.hotel_uid = std::string{kHotelUid}, .price = 10000};
    std::optional<Reservation> reservation;
    mutable std::optional<NewReservation> created;

  private:
    CallLog& log_;
};

class FakePaymentClient final : public IPaymentClient {
  public:
    explicit FakePaymentClient(CallLog& log) : log_(log) {}

    Payment CreatePayment(std::int64_t price) const override {
        log_.emplace_back("CreatePayment");
        return Payment{.payment_uid = std::string{kPaymentUid}, .status = "PAID", .price = price};
    }

    std::optional<Payment>
    GetPayment([[maybe_unused]] const std::string& payment_uid) const override {
        return std::nullopt;
    }

    void CancelPayment(const std::string& payment_uid) const override {
        log_.push_back("CancelPayment " + payment_uid);
    }

  private:
    CallLog& log_;
};

class FakeLoyaltyClient final : public ILoyaltyClient {
  public:
    explicit FakeLoyaltyClient(CallLog& log) : log_(log) {}

    Loyalty GetLoyalty([[maybe_unused]] const std::string& username) const override {
        log_.emplace_back("GetLoyalty");
        return Loyalty{.status = "GOLD", .discount = 10, .reservation_count = 25};
    }

    void Increment([[maybe_unused]] const std::string& username) const override {
        log_.emplace_back("Increment");
    }

    void Decrement([[maybe_unused]] const std::string& username) const override {
        log_.emplace_back("Decrement");
    }

  private:
    CallLog& log_;
};

class BookingServiceTest : public ::testing::Test {
  protected:
    static BookingRequest Request(std::string start_date, std::string end_date) {
        return BookingRequest{
            .hotel_uid = std::string{kHotelUid},
            .start_date = std::move(start_date),
            .end_date = std::move(end_date),
        };
    }

    static Reservation ExistingReservation(std::string status) {
        Reservation reservation;
        reservation.reservation_uid = kReservationUid;
        reservation.status = std::move(status);
        reservation.payment_uid = kPaymentUid;
        return reservation;
    }

    CallLog log_;
    FakeReservationClient reservations_{log_};
    FakePaymentClient payments_{log_};
    FakeLoyaltyClient loyalty_{log_};
    BookingService service_{reservations_, payments_, loyalty_};
};

}

TEST(CalcPrice, AppliesDiscountToAllNights) {
    EXPECT_EQ(CalcPrice(10000, 3, 10), 27000);
    EXPECT_EQ(CalcPrice(10000, 3, 7), 27900);
    EXPECT_EQ(CalcPrice(10000, 1, 0), 10000);
}

TEST(ParseDate, AcceptsIsoDates) {
    using namespace std::chrono;
    EXPECT_EQ(ParseDate("2021-10-08"), sys_days{year{2021} / October / day{8}});
    EXPECT_EQ(ParseDate("2024-02-29"), sys_days{year{2024} / February / day{29}});
}

TEST(ParseDate, RejectsMalformedDates) {
    EXPECT_EQ(ParseDate(""), std::nullopt);
    EXPECT_EQ(ParseDate("2021-1-08"), std::nullopt);
    EXPECT_EQ(ParseDate("2021/10/08"), std::nullopt);
    EXPECT_EQ(ParseDate("2021-13-01"), std::nullopt);
    EXPECT_EQ(ParseDate("2023-02-29"), std::nullopt);
    EXPECT_EQ(ParseDate("20x1-10-08"), std::nullopt);
}

TEST_F(BookingServiceTest, BookPaysWithDiscountThenReservesThenCounts) {
    const auto result = service_.Book(std::string{kUser}, Request("2021-10-08", "2021-10-11"));

    EXPECT_EQ(log_, (CallLog{"GetHotel", "GetLoyalty", "CreatePayment", "CreateReservation",
                             "Increment"}));
    EXPECT_EQ(result.payment.price, 27000);
    EXPECT_EQ(result.discount, 10);
    EXPECT_EQ(result.reservation.reservation_uid, kReservationUid);
    EXPECT_EQ(reservations_.created.value_or(NewReservation{}).payment_uid, kPaymentUid);
}

TEST_F(BookingServiceTest, BookRejectsEndDateNotAfterStart) {
    try {
        service_.Book(std::string{kUser}, Request("2021-10-11", "2021-10-11"));
        FAIL() << "ValidationError expected";
    } catch (const ValidationError& e) {
        EXPECT_EQ(e.Field(), "endDate");
    }
    EXPECT_TRUE(log_.empty());
}

TEST_F(BookingServiceTest, BookRejectsUnknownHotelWithoutPaying) {
    reservations_.hotel = std::nullopt;
    try {
        service_.Book(std::string{kUser}, Request("2021-10-08", "2021-10-11"));
        FAIL() << "ValidationError expected";
    } catch (const ValidationError& e) {
        EXPECT_EQ(e.Field(), "hotelUid");
    }
    EXPECT_EQ(log_, CallLog{"GetHotel"});
}

TEST_F(BookingServiceTest, CancelCancelsReservationPaymentAndCounter) {
    reservations_.reservation = ExistingReservation("PAID");

    EXPECT_TRUE(service_.Cancel(std::string{kUser}, std::string{kReservationUid}));
    EXPECT_EQ(log_, (CallLog{"GetReservation", "CancelReservation",
                             "CancelPayment " + std::string{kPaymentUid}, "Decrement"}));
}

TEST_F(BookingServiceTest, CancelOfCanceledReservationChangesNothing) {
    reservations_.reservation = ExistingReservation("CANCELED");

    EXPECT_TRUE(service_.Cancel(std::string{kUser}, std::string{kReservationUid}));
    EXPECT_EQ(log_, CallLog{"GetReservation"});
}

TEST_F(BookingServiceTest, CancelOfMissingReservationIsNotFound) {
    EXPECT_FALSE(service_.Cancel(std::string{kUser}, std::string{kReservationUid}));
    EXPECT_FALSE(service_.Cancel(std::string{kUser}, "not-a-uuid"));
    EXPECT_EQ(log_, CallLog{"GetReservation"});
}

}
