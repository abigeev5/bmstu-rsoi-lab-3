add_library(rsoi-common STATIC
    http.cpp)
target_include_directories(rsoi-common PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(rsoi-common PUBLIC userver::core)
rsoi_target_defaults(rsoi-common)
