enable_testing()
include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG main
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
option(INSTALL_GMOCK "Install GMock" ON)
option(INSTALL_GTEST "Install GTest" ON)
 
FetchContent_MakeAvailable(googleTest)

include(GoogleTest)
#include(Coverage)
include(Memcheck)

macro(AddTests target)
    target_link_libraries(${target} PRIVATE gtest_main gmock)
    gtest_discover_tests(${target})
    #AddCoverage(${target})
    AddMemcheck(${target})
endmacro()
