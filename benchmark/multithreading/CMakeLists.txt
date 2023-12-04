add_executable(multithreadingBenckmark mulithreadingBenchmark.cpp)

target_link_libraries(multithreadingBenckmark PRIVATE multithreading_header_only)
add_bechmark(multithreadingBenckmark)

add_custom_target(runBenchmark 
                  COMMAND multithreadingBenckmark --benchmark_repetitions=10 --benchmark_format=json --benchmark_out=out.json && mv out.json ${CMAKE_SOURCE_DIR}/out.json
                  DEPENDS multithreadingBenckmark)
