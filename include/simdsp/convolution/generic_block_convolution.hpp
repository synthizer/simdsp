#pragma once

namespace simdsp {

/**
 * A simple, generic, block convolver which executes in O(n^2).
 *
 * Under the hood this is implemented with a basic libsimdpp dispatch on the straightforward C++ code.  Used as a
 * benchmark/reference implementation/"we don't have anything better we can do so time to fall back to the one that
 * always works".
 *
 * The impulse pointer must be aligned with the simdsp convention, and contain the *reversed* impulse.  The input
 * pointer must point at the "current" sample, and it must be valid to access the frame at input[-impulse_len + 1]
 *
 * The channels of both the input and impulse must match.
 *
 * If add is true, then the output is added to the destination. Otherwise, the output replaces the destination.  Under
 * the hood this is done as a branch which converts to a set of template functions, not a branch on every iteration.
 */
void genericBlockConvolver(float *input, unsigned int input_len, unsigned int input_channels, float *impulse,
                           unsigned int impulse_len, float *output);
} // namespace simdsp
