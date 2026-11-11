cmake_minimum_required(VERSION 3.28)
project(rsoi_labs LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# No C++20 modules here; scanning every source with clang-scan-deps only slows the build down.
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

# userver is built from source as a subproject; SYSTEM hides its headers from our warnings and clang-tidy.
include(cmake/get_cpm.cmake)
CPMAddPackage(
    NAME userver
    VERSION 3.2
    URL https://github.com/userver-framework/userver/archive/refs/tags/v3.2.tar.gz
    URL_HASH SHA256=9f53750cc6ed2889c70cfe3cafa849a3d13c53338f22cf4fb841ec463ca33999
    SYSTEM YES
    OPTIONS
        "USERVER_FEATURE_POSTGRESQL ON")
userver_setup_environment()

set(RSOI_SERVICES gateway-service reservation-service payment-service loyalty-service)

option(RSOI_CLANG_TIDY"Run clang-tidy during compilation" OFF)
if(RSOI_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE NAMES clang-tidy-20 clang-tidy REQUIRED)
endif()

function(rsoi_target_defaults target)
    target_compile_options(${target} PRIVATE
        -Wall -Wextra -Wpedantic
        -Wshadow -Wconversion -Wsign-conversion -Wnon-virtual-dtor -Wold-style-cast -Wimplicit-fallthrough)
    set_target_properties(${target} PROPERTIES COMPILE_WARNING_AS_ERROR ON)
    if(RSOI_CLANG_TIDY)
        set_target_properties(${target} PROPERTIES CXX_CLANG_TIDY ${CLANG_TIDY_EXE})
    endif()
endfunction()

# `cmake --build --preset debug --target format` / `format-check`
find_program(CLANG_FORMAT_EXE NAMES clang-format-20 clang-format)
if(CLANG_FORMAT_EXE)
    foreach(service IN LISTS RSOI_SERVICES)
        list(APPEND RSOI_FORMAT_GLOBS
            ${PROJECT_SOURCE_DIR}/${service}/*.cpp
            ${PROJECT_SOURCE_DIR}/${service}/*.hpp)
    endforeach()
    list(APPEND RSOI_FORMAT_GLOBS ${PROJECT_SOURCE_DIR}/common/*.cpp ${PROJECT_SOURCE_DIR}/common/*.hpp)
    file(GLOB_RECURSE RSOI_FORMAT_SOURCES CONFIGURE_DEPENDS ${RSOI_FORMAT_GLOBS})
    add_custom_target(format
        COMMAND ${CLANG_FORMAT_EXE} -i ${RSOI_FORMAT_SOURCES}
        COMMENT "Formatting sources")
    add_custom_target(format-check
        COMMAND ${CLANG_FORMAT_EXE} --dry-run --Werror ${RSOI_FORMAT_SOURCES}
        COMMENT "Checking formatting")
endif()

enable_testing()

add_subdirectory(common)
foreach(service IN LISTS RSOI_SERVICES)
    add_subdirectory(${service})
endforeach()
