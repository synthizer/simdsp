#include "simdsp/convolution/generic_block_convolution.hpp"

namespace simdsp {
namespace SIMDPP_ARCH_NAMESPACE {

void genericBlockConvolver(float *input, unsigned int input_len, unsigned int input_channels, float *impulse,
                           unsigned int impulse_len, float *output) {
  float *hstart = input - (impulse_len + 1) * input_channels;
  for (unsigned int sample = 0; sample < input_len; sample++) {
    float *oframe = output + sample * input_channels;

    for (unsigned int impulse_ind = 0; impulse_ind < impulse_len; impulse_ind++) {
      float *iframe = hstart + (sample + impulse_ind) * input_channels;
      float *impulseframe = impulse + impulse_ind * input_channels;
      for (unsigned int ch = 0; ch < input_channels; ch++) {
        oframe[ch] += iframe[ch] * impulseframe[ch];
      }
    }
  }
}

} // namespace SIMDPP_ARCH_NAMESPACE

void genericBlockConvolver(float *input, unsigned int input_len, unsigned int input_channels, float *impulse,
                           unsigned int impulse_len, float *output) {}
} // namespace simdsp
