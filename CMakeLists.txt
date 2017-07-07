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

# Add Lib utils for merging static libs
include("cmake/MergeStaticLibs.cmake")
# Set 3rd part lib build type to make final lib minimal
set(HUNTER_CONFIGURATION_TYPES Release)

# Add hunter to manage external libs
include("cmake/HunterGate.cmake")
HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.19.7.tar.gz"
    SHA1 "e83786da74f703099fce44a4c021b5d3e5fc3b41"
    FILEPATH "cmake/config.cmake"
)

# Final stand-alone lib file
project(CatenisAPIClient)
# Dependent temp lib
project(tempCatenis)

# Add compiler specific flags
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    # Clang
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    # GNU
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -ldl")
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

# Add components needed for Boost.asio: system, regex
hunter_add_package(Boost COMPONENTS system regex)
find_package(Boost CONFIG REQUIRED system regex)

# Add OpenSSl components: openssl, crypto
hunter_add_package(OpenSSL)
find_package(OpenSSL REQUIRED)


# Link and make lib
add_library(tempCatenis src/CatenisApiClient.cpp include/CatenisApiClient.h src/CatenisApiInternals.cpp include/CatenisApiInternals.h)
target_link_libraries(tempCatenis Boost::system Boost::regex OpenSSL::SSL OpenSSL::Crypto)

# fix cmake generator bug, kinda hacky since static, will fix later
if(NOT WIN32)
  set(tempCatenis_loc "${CMAKE_CURRENT_BINARY_DIR}/libtempCatenis.a")
  set(catenic_loc "${CMAKE_CURRENT_BINARY_DIR}/libCatenisAPIClient.a")
else()
  set(tempCatenis_loc "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/tempCatenis.lib")
endif()

# Merge all libs into one lib
merge_static_libs(CatenisAPIClient tempCatenis Boost::system Boost::regex OpenSSL::SSL OpenSSL::Crypto)

# Build samples if added flag added
message(STATUS "BUILD_SAMPLES : " ${BUILD_SAMPLES})
if (BUILD_SAMPLES)
  add_subdirectory(samples)
endif()

# Delete temp lib tempCatenis after build
if(WIN32)
  # have to use del command, and turn slash into backslash
  string(REGEX REPLACE "/" "\\\\" tempCatenis_loc ${tempCatenis_loc})
  add_custom_command(TARGET CatenisAPIClient POST_BUILD 
  COMMAND del ${tempCatenis_loc}
)
else()
  add_custom_command(TARGET CatenisAPIClient POST_BUILD
  COMMAND rm ${tempCatenis_loc}
)
endif()
