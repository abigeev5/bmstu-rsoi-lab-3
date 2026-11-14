add_executable(payment-service-tests
    payment_service_test.cpp)
target_link_libraries(payment-service-tests PRIVATE userver::utest)
rsoi_target_defaults(payment-service-tests)

add_google_tests(payment-service-tests)
