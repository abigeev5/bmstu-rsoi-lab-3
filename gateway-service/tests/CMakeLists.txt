add_executable(gateway-service-tests
    gateway_service_test.cpp)
target_link_libraries(gateway-service-tests PRIVATE gateway-service-lib userver::utest)
rsoi_target_defaults(gateway-service-tests)

add_google_tests(gateway-service-tests)
