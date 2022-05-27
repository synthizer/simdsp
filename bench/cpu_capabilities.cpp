#include "simdsp/cpu_capabilities.hpp"

#include <benchmark/benchmark.h>

static void bm_getCpuCapabilities(benchmark::State &state) {
  for (auto _ : state) {
    benchmark::DoNotOptimize(simdsp::getCpuCapabilities());
  }
}

BENCHMARK(bm_getCpuCapabilities);
