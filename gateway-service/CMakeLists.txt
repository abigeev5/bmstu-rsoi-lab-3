add_library(gateway-service-lib STATIC
    src/api/handlers.cpp
    src/booking_component.cpp
    src/clients/http_clients.cpp
    src/domain/booking_service.cpp)
target_include_directories(gateway-service-lib PUBLIC src)
target_link_libraries(gateway-service-lib PUBLIC userver::core rsoi-common)
rsoi_target_defaults(gateway-service-lib)

add_executable(gateway-service
    src/main.cpp)
target_link_libraries(gateway-service PRIVATE gateway-service-lib)
rsoi_target_defaults(gateway-service)

add_subdirectory(tests)
