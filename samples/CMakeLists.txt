project(CmdSample)

# Add directories 
include_directories(../include ../_builds)

# Link and Make exe
add_executable(CmdSample CmdSample.cpp)
target_link_libraries(CmdSample CatenisAPIClient)
