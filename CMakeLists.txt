cmake_minimum_required(VERSION 3.7.1)

include("cmake/HunterGate.cmake")
HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.10.9.tar.gz"
    SHA1 "53b198e364dc7bc8360fc545f798563229bd7e20"
)

project(CatenisAPIClient)

# Add directories 
include_directories(src include)

# Add components needed for Boost.asio
hunter_add_package(Boost COMPONENTS system)
find_package(Boost CONFIG REQUIRED system)

# Add OpenSSl components
hunter_add_package(OpenSSL)
find_package(OpenSSL REQUIRED)
include_directories("${OPENSSL_INCLUDE_DIR}")

# Link
add_executable(CatenisAPIClient src/CatenisApiClient.cpp include/CatenisApiClient.h)
target_link_libraries(CatenisAPIClient Boost::system ${OPENSSL_CRYPTO_LIBRARY})