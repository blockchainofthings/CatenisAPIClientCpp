cmake_minimum_required(VERSION 3.0.0)

# Set compiler debug flags
set(CMAKE_C_FLAGS "-g -Wall")
set(CMAKE_CXX_FLAGS "-g -Wall")

# Set c++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Silence linkage warnings
set(CMAKE_SHARED_LINKER_FLAGS "-w")

# Add hunter to manage external libs
include("cmake/HunterGate.cmake")
HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.19.7.tar.gz"
    SHA1 "e83786da74f703099fce44a4c021b5d3e5fc3b41"
    FILEPATH "cmake/config.cmake"
)

project(CatenisAPIClient)

# Add compiler specific flags
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    # Clang
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    # GNU
    # -pthread option needed for boost
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
    # intel
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    # MSVC
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
    add_definitions(-DBOOST_ALL_NO_LIB=1)
endif()

# build sample option
option(BUILD_SAMPLES "Build sample programs.")

# Add directories for including headers
include_directories(include)

# Add components needed for Poco: Foundation, Net, JSON
hunter_add_package(PocoCpp)
find_package(Poco REQUIRED Foundation Net JSON CONFIG)

# Add OpenSSl components: openssl, crypto
hunter_add_package(OpenSSL)
find_package(OpenSSL REQUIRED)

# Link and make exe
#add_executable(CatenisAPIClient CatenisApiClient.cpp CatenisApiClient.h CatenisApiInternals.cpp CatenisApiInternals.h)
#target_link_libraries(CatenisAPIClient Boost::system Boost::regex OpenSSL::SSL OpenSSL::Crypto)

# Link and make lib
add_library(CatenisAPIClient src/CatenisApiClient.cpp include/CatenisApiClient.h src/CatenisApiInternals.cpp include/CatenisApiInternals.h)
target_link_libraries(CatenisAPIClient Poco::Foundation Poco::Net Poco::JSON OpenSSL::SSL OpenSSL::Crypto)

# Build samples if added flag added
message(STATUS "BUILD_SAMPLES : " ${BUILD_SAMPLES})
if (BUILD_SAMPLES)
  add_subdirectory(samples)
endif()
