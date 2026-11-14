add_executable(reservation-service-tests
    reservation_service_test.cpp)
target_link_libraries(reservation-service-tests PRIVATE userver::utest)
rsoi_target_defaults(reservation-service-tests)

add_google_tests(reservation-service-tests)
