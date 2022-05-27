#include <simdsp/cpu_capabilities.hpp>

#include <iostream>

#include <stdint.h>

using std::cout, std::endl;

int main() {
  cout << "Found the following CPU flags:" << endl;

  auto caps = simdsp::getCpuCapabilitiesUncached();
  for (size_t i = 0; i < simdsp::CpuCapabilities::ALL_BITS_COUNT; i++) {
    if (caps & simdsp::CpuCapabilities::ALL_BITS[i]) {
      cout << simdsp::CpuCapabilities::ALL_BITS[i].printable_string << endl;
    }
  }
  return 0;
}
