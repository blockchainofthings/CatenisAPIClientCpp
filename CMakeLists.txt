cmake_minimum_required(VERSION 3.5.0)

# Set compiler debug flags
#set(CMAKE_C_FLAGS "-g -Wall")
#set(CMAKE_CXX_FLAGS "-g -Wall")

# Set c++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  # add clang specific flags
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  # add GNU specific flags
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
  # add Intel specific flags
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  set(CMAKE_CXX_FLAGS "/EHsc")
  add_definitions(-DBOOST_ALL_NO_LIB=1)
endif()

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

# build sample option
option(build_samples "Build sample programs." ON)

# Add directories 
include_directories(src include)

# Add components needed for Boost.asio: system, regex
hunter_add_package(Boost COMPONENTS system regex)
find_package(Boost CONFIG REQUIRED system regex)

# Add OpenSSl components: openssl, crypto
hunter_add_package(OpenSSL)
find_package(OpenSSL REQUIRED)

# Link and make exe
#add_executable(CatenisAPIClient src/CatenisApiClient.cpp include/CatenisApiClient.h)
#target_link_libraries(CatenisAPIClient Boost::system Boost::regex OpenSSL::SSL OpenSSL::Crypto)

# Link and make lib
add_library(CatenisAPIClient src/CatenisApiClient.cpp include/CatenisApiClient.h)
target_link_libraries(CatenisAPIClient Boost::system Boost::regex OpenSSL::SSL OpenSSL::Crypto)

# Build samples after the service apis
# but keep them under src
if (build_samples)
  add_subdirectory(samples)
endif()
