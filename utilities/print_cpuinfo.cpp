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

  cout << endl;

  auto caches = simdsp::getCpuCacheInfo();

  cout << "caches:" << endl;

  cout << "l1i=" << caches.l1i << endl;
  cout << "l1d=" << caches.l1d << endl;
  cout << "l1u=" << caches.l1u << endl;

  cout << "l2i=" << caches.l2i << endl;
  cout << "l2d=" << caches.l2d << endl;
  cout << "l2u=" << caches.l2u << endl;

  cout << "l3i=" << caches.l3i << endl;
  cout << "l3d=" << caches.l3d << endl;
  cout << "l3u=" << caches.l3u << endl;

  return 0;
}
