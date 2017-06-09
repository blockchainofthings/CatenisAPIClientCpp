cmake_minimum_required(VERSION 3.7.1)

# Set compiler debug flags
#set(CMAKE_C_COMPILER "/usr/bin/clang")
set(CMAKE_C_FLAGS "-Wall -std=c99")

#set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
set(CMAKE_CXX_FLAGS "-Wall -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG "-g")

set(CMAKE_SHARED_LINKER_FLAGS "-w")

# Add hunter to manage external libs
include("cmake/HunterGate.cmake")
HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.19.7.tar.gz"
    SHA1 "e83786da74f703099fce44a4c021b5d3e5fc3b41"
    FILEPATH "cmake/config.cmake"
)

project(CatenisAPIClient)

# Add directories 
include_directories(src include)

# Add components needed for Boost.asio: system, regex
hunter_add_package(Boost COMPONENTS system regex)
find_package(Boost CONFIG REQUIRED system regex)

# Add OpenSSl components: openssl, crypto
hunter_add_package(OpenSSL)
find_package(OpenSSL REQUIRED)

# Link
add_executable(CatenisAPIClient src/CatenisApiClient.cpp include/CatenisApiClient.h)
target_link_libraries(CatenisAPIClient Boost::system Boost::regex OpenSSL::SSL OpenSSL::Crypto)
