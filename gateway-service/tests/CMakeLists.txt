add_executable(gateway-service-tests
    circuit_breaker_test.cpp
    gateway_service_test.cpp
    retry_queue_test.cpp)
target_link_libraries(gateway-service-tests PRIVATE gateway-service-lib userver::utest)
rsoi_target_defaults(gateway-service-tests)

add_google_tests(gateway-service-tests)
