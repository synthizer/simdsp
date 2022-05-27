/**
 * Support for determining CPU capabilities.
 *
 * parts of this file were taken from libsimdpp, under the Boost Software License 1.0:
 *
 * Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */
#pragma once

#include <stddef.h>
#include <stdint.h>

namespace simdsp {

/*
 * Info about a specific CPU bit: a printable string, and a bitflag.
 *
 * This is used instead of an enum class because it gives us the ability to add methods to CpuCapabilities itself and,
 * via some operator overloading, still be able to or/and with CPU bits.
 * */
struct CpuBit {
  /*
   * Support string for printing.
   */
  const char *printable_string;

  unsigned int bit;
};

/*
 * These are bitflags.
 *
 * We only concern ourselves with X86 for now.
 *
 * Unlike the original libsimdpp implementation, we also only really concern ourselves with the major ones:
 * SSE2/AVX/AVX2.
 * */
class CpuCapabilities {
public:
  CpuCapabilities(unsigned int bits = 0);

  inline static const CpuBit NONE{"none", 0}, X86_SSE2{"sse2", 1 << 1}, X86_SSE3{"sse3", 1 << 2},
      X86_SSSE3{"ssse3", 1 << 3}, X86_SSE4_1{"sse4_1", 1 << 4}, X86_POPCNT_INSN{"popcnt_insn", 1 << 5},
      X86_AVX{"avx", 1 << 6}, X86_AVX2{"avx2", 1 << 7}, X86_FMA3{"fma3", 1 << 8}, X86_FMA4{"fma4", 1 << 9},
      X86_XOP{"xop", 1 << 10}, X86_AVX512F{"avx512f", 1 << 11}, X86_AVX512BW{"avx512bw", 1 << 12},
      X86_AVX512DQ{"avx512dq", 1 << 13}, X86_AVX512VL{"avx512vl", 1 << 14};

  /*
   * these two constants expose a table of all CPU bits except for none for the purposes of iteration.
   *
   * Unfortunately we can't use std::array or std::vector: these might or might not potentially start bringing CPU
   * instructions in where we don't want them.
   * */
  static const CpuBit *ALL_BITS;
  static const size_t ALL_BITS_COUNT;

  unsigned int bits = 0;
  operator bool();
};

/*
 * We want these operators and helper functions out of line just in case the linker decides to be cute.
 */

#define BOOL_OPS(OP)                                                                                                   \
  CpuCapabilities operator OP(const CpuCapabilities &a, const CpuCapabilities &b);                                     \
  CpuCapabilities &operator OP##=(CpuCapabilities &a, const CpuCapabilities &b);                                       \
  CpuCapabilities operator OP(const CpuCapabilities &a, const CpuBit &b);                                              \
  CpuCapabilities &operator OP##=(CpuCapabilities &a, const CpuBit &b);

BOOL_OPS(&)
BOOL_OPS(|)
BOOL_OPS(^)

CpuCapabilities operator|(const CpuCapabilities &a, const CpuCapabilities &b);

#undef BOOL_OPS

/**
 *  Get uncached CPU capabilities.
 *
 * This is an expensive function, easily on the order of hundreds of cycles.  Users should cache the results.
 */
CpuCapabilities getCpuCapabilitiesUncached();
/*
 * Get cached CPU capabilities.  After the first slow invocation, this function is an atomic load.
 */
CpuCapabilities getCpuCapabilities();

} // namespace simdsp
