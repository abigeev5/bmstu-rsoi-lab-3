add_library(reservation-service-lib STATIC
    src/api/handlers.cpp
    src/domain/reservation_service.cpp
    src/reservation_component.cpp
    src/storage/pg_repositories.cpp)
target_include_directories(reservation-service-lib PUBLIC src)
target_link_libraries(reservation-service-lib PUBLIC userver::postgresql rsoi-common)
rsoi_target_defaults(reservation-service-lib)

add_executable(reservation-service
    src/main.cpp)
target_link_libraries(reservation-service PRIVATE reservation-service-lib)
rsoi_target_defaults(reservation-service)

add_subdirectory(tests)
