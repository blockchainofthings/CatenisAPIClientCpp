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
if ("${COM_SUPPORT_LIB}" STREQUAL "BOOST_ASIO")
    HunterGate(
            URL "https://github.com/ruslo/hunter/archive/v0.20.14.tar.gz"
            SHA1 "18cdbf5c47abdda437c73bf5437b7c3c65afe47c"
            FILEPATH "${CMAKE_SOURCE_DIR}/cmake/HunterConfig_BoostAsio.cmake"
    )
elseif ("${COM_SUPPORT_LIB}" STREQUAL "POCO")
    HunterGate(
            URL "https://github.com/ruslo/hunter/archive/v0.20.14.tar.gz"
            SHA1 "18cdbf5c47abdda437c73bf5437b7c3c65afe47c"
            FILEPATH "${CMAKE_SOURCE_DIR}/cmake/HunterConfig_Poco.cmake"
    )
endif()

# Final stand-alone lib file
project(CatenisAPIClient)

# Add compiler specific flags
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    # Clang
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    # GNU
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
    # intel
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    # MSVC
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
    add_definitions(-DBOOST_ALL_NO_LIB=1)
endif()

if ("${COM_SUPPORT_LIB}" STREQUAL "BOOST_ASIO")
    add_definitions(-DCOM_SUPPORT_LIB_BOOST_ASIO)
elseif ("${COM_SUPPORT_LIB}" STREQUAL "POCO")
    add_definitions(-DCOM_SUPPORT_LIB_POCO)
endif()

# build sample option
option(BUILD_SAMPLES "Build sample programs.")

# Add directories for including headers
include_directories(include)

if ("${COM_SUPPORT_LIB}" STREQUAL "BOOST_ASIO")
    # Add components needed for Boost.asio: system
    hunter_add_package(Boost COMPONENTS system)
    find_package(Boost CONFIG REQUIRED system)
elseif ("${COM_SUPPORT_LIB}" STREQUAL "POCO")
    # Add components needed for Poco: Foundation, Net, JSON <— needed for linking
    # XML, Util, Crypto <— needed for the stand-alone final lib
    hunter_add_package(PocoCpp)
    find_package(Poco REQUIRED Foundation Net JSON NetSSL XML Util Crypto CONFIG)
endif()

# Add OpenSSl components: openssl, crypto
hunter_add_package(OpenSSL)
find_package(OpenSSL REQUIRED)


# Link and make lib
add_library(tempCatenis src/CatenisApiClient.cpp include/CatenisApiClient.h src/CatenisApiInternals.cpp include/CatenisApiInternals.h include/CatenisApiException.h include/json-spirit/json_spirit_reader_template.h include/json-spirit/json_spirit_writer_template.h include/json-spirit/json_spirit_value.h include/json-spirit/json_spirit_writer_options.h include/json-spirit/json_spirit_error_position.h)

if ("${COM_SUPPORT_LIB}" STREQUAL "BOOST_ASIO")
    target_link_libraries(tempCatenis Boost::system OpenSSL::SSL OpenSSL::Crypto)
elseif ("${COM_SUPPORT_LIB}" STREQUAL "POCO")
    target_link_libraries(tempCatenis Poco::Foundation Poco::Net Poco::JSON Poco::NetSSL OpenSSL::SSL OpenSSL::Crypto)

    # Add os specific libs
    if(WIN32)
        target_link_libraries(tempCatenis crypt32)
    endif()
endif()

# Merge all libs into one lib (the first lib added has to be the lib created: tempCatenis)
if ("${COM_SUPPORT_LIB}" STREQUAL "BOOST_ASIO")
    merge_static_libs(CatenisAPIClient tempCatenis Boost::system OpenSSL::SSL OpenSSL::Crypto)
elseif ("${COM_SUPPORT_LIB}" STREQUAL "POCO")
    merge_static_libs(CatenisAPIClient tempCatenis Poco::Foundation Poco::Net Poco::JSON Poco::NetSSL Poco::XML Poco::Util Poco::Crypto OpenSSL::SSL OpenSSL::Crypto)
endif()

# Build samples if added flag added
message(STATUS "BUILD_SAMPLES : " ${BUILD_SAMPLES})
if (BUILD_SAMPLES)
  add_subdirectory(samples)
endif()

# Delete temp lib tempCatenis after build
if(WIN32)
  string(REGEX REPLACE "/" "\\\\" libOutDir "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/")
  add_custom_command(TARGET CatenisAPIClient POST_BUILD
    COMMAND del "${libOutDir}$<TARGET_FILE_NAME:tempCatenis>"
  )
else()
  add_custom_command(TARGET CatenisAPIClient POST_BUILD
    COMMAND rm $<TARGET_FILE:tempCatenis>
  )
endif()
