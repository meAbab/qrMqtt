cmake_minimum_required(VERSION 3.16)
project(qrMqtt VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenCV REQUIRED COMPONENTS core imgproc videoio)
find_package(OpenSSL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(MOSQUITTO REQUIRED IMPORTED_TARGET libmosquitto)
pkg_check_modules(ZBAR REQUIRED IMPORTED_TARGET zbar)

add_library(qrmqtt_core
    src/config.cpp
    src/duplicate_filter.cpp
    src/event.cpp
    src/mqtt_client.cpp
    src/qr_scanner.cpp
    src/token_validator.cpp
)
target_include_directories(qrmqtt_core PUBLIC include ${OpenCV_INCLUDE_DIRS})
target_link_libraries(qrmqtt_core
    PUBLIC
        ${OpenCV_LIBS}
        OpenSSL::Crypto
        OpenSSL::SSL
        PkgConfig::MOSQUITTO
        PkgConfig::ZBAR
)
target_compile_options(qrmqtt_core PRIVATE -Wall -Wextra -Werror)

add_executable(qrMqtt src/main.cpp)
target_link_libraries(qrMqtt PRIVATE qrmqtt_core)
target_compile_options(qrMqtt PRIVATE -Wall -Wextra -Werror)

include(CTest)
if(BUILD_TESTING)
    add_executable(qrmqtt_tests tests/core_tests.cpp)
    target_link_libraries(qrmqtt_tests PRIVATE qrmqtt_core)
    target_compile_options(qrmqtt_tests PRIVATE -Wall -Wextra -Werror)
    add_test(NAME qrmqtt_core_tests COMMAND qrmqtt_tests)
endif()

install(TARGETS qrMqtt RUNTIME DESTINATION bin)
