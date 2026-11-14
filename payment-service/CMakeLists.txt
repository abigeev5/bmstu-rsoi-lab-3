add_library(payment-service-lib STATIC
    src/api/handlers.cpp
    src/domain/payment_service.cpp
    src/payment_component.cpp
    src/storage/pg_repositories.cpp)
target_include_directories(payment-service-lib PUBLIC src)
target_link_libraries(payment-service-lib PUBLIC userver::postgresql rsoi-common)
rsoi_target_defaults(payment-service-lib)

add_executable(payment-service
    src/main.cpp)
target_link_libraries(payment-service PRIVATE payment-service-lib)
rsoi_target_defaults(payment-service)

add_subdirectory(tests)
