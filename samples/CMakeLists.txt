project(CmdSample)

# Link and Make exe
add_executable(CmdSample CmdSample.cpp)
target_link_libraries(CmdSample CatenisAPIClient)
