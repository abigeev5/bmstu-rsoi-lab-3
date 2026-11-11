add_executable(loyalty-service-tests
    loyalty_service_test.cpp)
target_link_libraries(loyalty-service-tests PRIVATE loyalty-service-lib userver::utest)
rsoi_target_defaults(loyalty-service-tests)

add_google_tests(loyalty-service-tests)
