include(FetchContent)

FetchContent_Declare(
    benchmark
    GIT_REPOSITORY  https://github.com/google/benchmark.git
    GIT_TAG         main
)
set(benchmark_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(benchmark)


macro(add_bechmark target)
    target_link_libraries(${target} PRIVATE benchmark::benchmark_main)
endmacro()