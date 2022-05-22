cmake_minimum_required(VERSION 3.15.0)
project(simdsp C CXX)

add_subdirectory(third_party/catch2)

# Apparently we have to do this globally because CMake won't let us set a define for a subdirectory. Without it
# google/benchmark wants gtest.
set(BENCHMARK_ENABLE_TESTING OFF)
add_subdirectory(third_party/benchmark)


add_library(simdsp STATIC
  src/convolution/generic_block_convolution.cpp
)
target_include_directories(simdsp PUBLIC include)
set_property(TARGET simdsp PROPERTY CXX_STANDARD 17)

add_executable(benches
  bench/convolution_engine.cpp
)
target_link_libraries(benches simdsp benchmark::benchmark benchmark::benchmark_main)
set_property(TARGET benches PROPERTY CXX_STANDARD 17)

add_executable(tests
  tests/main.cpp
  tests/passes.cpp
)
target_link_libraries(tests simdsp Catch2::Catch2)
set_property(TARGET tests PROPERTY CXX_STANDARD 17)

include(CTest)
include(third_party/catch2/contrib/Catch.cmake)
catch_discover_tests(tests)
