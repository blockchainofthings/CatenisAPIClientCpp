project(CmdSample)

# Link and Make exe
add_executable(CmdSample CmdSample.cpp)
if(UNIX AND NOT APPLE)
    # Linux - need to link against libdl too
    target_link_libraries(CmdSample CatenisAPIClient dl)
else()
    target_link_libraries(CmdSample CatenisAPIClient)
endif()
