#include "simdsp/convolution/generic_block_convolution.hpp"

#include <chrono>
#include <thread>

namespace simdsp {
void genericBlockConvolver(float *input, unsigned int input_len, unsigned int input_channels, float *impulse,
                           unsigned int impulse_len, float *output) {
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
}
} // namespace simdsp
