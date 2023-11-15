add_library(multithreading_header_only INTERFACE include/Locks.h)

target_include_directories(multithreading_header_only INTERFACE 
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

include(Doxygen)
Doxygen(multithreading_header_only include/Locks.h)

