#include "simdsp/convolution/generic_block_convolution.hpp"

#include <benchmark/benchmark.h>

static void bm_genericBlockConvolver(benchmark::State &state) {
  alignas(64) float input_array[32] = {0.0};
  alignas(64) float impulse[16] = {0.0};
  alignas(16) float output[17];

  for (auto _ : state) {
    simdsp::genericBlockConvolver(input_array + 15, 32 - 15, 1, impulse, 16, output);
  }
}

BENCHMARK(bm_genericBlockConvolver);
