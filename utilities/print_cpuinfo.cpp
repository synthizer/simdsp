#include <simdsp/system_info.hpp>

#include <iostream>

#include <stdint.h>

using std::cout, std::endl;

int main() {
  cout << "Found the following CPU flags:" << endl;

  auto info = simdsp::getSystemInfo();
  for (size_t i = 0; i < simdsp::CpuCapabilities::ALL_BITS_COUNT; i++) {
    if (info.cpu_capabilities & simdsp::CpuCapabilities::ALL_BITS[i]) {
      cout << simdsp::CpuCapabilities::ALL_BITS[i].printable_string << endl;
    }
  }

  cout << endl;

  cout << "caches:" << endl;

  cout << "l1i=" << info.cache_info.l1i << endl;
  cout << "l1d=" << info.cache_info.l1d << endl;
  cout << "l1u=" << info.cache_info.l1u << endl;

  cout << "l2i=" << info.cache_info.l2i << endl;
  cout << "l2d=" << info.cache_info.l2d << endl;
  cout << "l2u=" << info.cache_info.l2u << endl;

  cout << "l3i=" << info.cache_info.l3i << endl;
  cout << "l3d=" << info.cache_info.l3d << endl;
  cout << "l3u=" << info.cache_info.l3u << endl;

  return 0;
}
