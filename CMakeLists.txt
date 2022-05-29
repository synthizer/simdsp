cmake_minimum_required(VERSION 3.15.0)
project(simdsp C CXX)

include(FetchContent)

# Apparently we have to do this globally because CMake won't let us set a define for a subdirectory. Without it
# google/benchmark wants gtest.
set(BENCHMARK_ENABLE_TESTING OFF)

FetchContent_Declare(benchmark
  GIT_REPOSITORY https://github.com/google/benchmark
  GIT_TAG 37be1e8252527229cccad9f097afe68572f3c08a
)

FetchContent_Declare(catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2
  GIT_TAG 62fd660583d3ae7a7886930b413c3c570e89786c
)

FetchContent_MakeAvailable(benchmark catch2)

# these two variables define two kinds of source files: normal files, and files which must go via CPU dynamic dispatch.
#
# They are turned into the library, below.
set(VANILLA_FILES
  src/cpu_capabilities.cpp
)

# these are for dispatching
set(DISPATCHED_FILES
  src/dispatched/convolution/generic_block_convolution.cpp
)

function(setup_properties T)
  target_include_directories(${T} PRIVATE include)
  set_property(TARGET ${T} PROPERTY CXX_STANDARD 17)
endfunction()

add_library(simdsp STATIC
  ${VANILLA_FILES}
  ${DISPATCHED_FILES}
)
target_include_directories(simdsp PUBLIC include)
setup_properties(simdsp)

add_executable(benches
  bench/convolution_engine.cpp
  bench/cpu_capabilities.cpp
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


include("${catch2_SOURCE_DIR}/contrib/Catch.cmake")
catch_discover_tests(tests)

add_executable(print_cpuinfo utilities/print_cpuinfo.cpp)
set_property(TARGET print_cpuinfo PROPERTY CXX_STANDARD 17)
target_link_libraries(print_cpuinfo simdsp)
