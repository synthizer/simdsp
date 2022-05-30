#include <simdsp/system_info.hpp>

#include <iostream>
#include <stdint.h>
#include <stdlib.h>

using std::cout, std::endl;

int main() {
  auto info = simdsp::getSystemInfo();
  auto json = simdsp::convertSystemInfoToJson(&info);
  cout << json << endl;
  free(json);
  return 0;
}
