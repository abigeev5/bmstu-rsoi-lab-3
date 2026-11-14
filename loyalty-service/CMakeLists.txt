add_library(loyalty-service-lib STATIC
    src/api/handlers.cpp
    src/domain/level.cpp
    src/domain/loyalty_service.cpp
    src/loyalty_component.cpp
    src/storage/pg_repositories.cpp)
target_include_directories(loyalty-service-lib PUBLIC src)
target_link_libraries(loyalty-service-lib PUBLIC userver::postgresql rsoi-common)
rsoi_target_defaults(loyalty-service-lib)

add_executable(loyalty-service
    src/main.cpp)
target_link_libraries(loyalty-service PRIVATE loyalty-service-lib)
rsoi_target_defaults(loyalty-service)

add_subdirectory(tests)
