include(FetchContent)

FetchContent_Declare(
    benchmark
    GIT_REPOSITORY  https://github.com/google/benchmark.git
    GIT_TAG         main
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

macro(LinkBechmark target)
    target_link_directories(target benchmark::benchmark)
endmacro()