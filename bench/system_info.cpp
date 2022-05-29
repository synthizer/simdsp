#include "simdsp/system_info.hpp"

#include <benchmark/benchmark.h>

static void bm_getSystemInfo(benchmark::State &state) {
  for (auto _ : state) {
    benchmark::DoNotOptimize(simdsp::getSystemInfo());
  }
}

static void bm_getSystemInfoUncached(benchmark::State &state) {
  for (auto _ : state) {
    benchmark::DoNotOptimize(simdsp::getSystemInfoUncached());
  }
}

BENCHMARK(bm_getSystemInfo);
BENCHMARK(bm_getSystemInfoUncached);
