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

  inline constexpr static const CpuBit NONE{"none", 0}, X86_SSE2{"x86_sse2", 1 << 1}, X86_SSE3{"x86_sse3", 1 << 2},
      X86_SSSE3{"x86_ssse3", 1 << 3}, X86_SSE4_1{"x86_sse4_1", 1 << 4}, X86_POPCNT_INSN{"x86_popcnt_insn", 1 << 5},
      X86_AVX{"x86_avx", 1 << 6}, X86_AVX2{"x86_avx2", 1 << 7}, X86_FMA3{"x86_fma3", 1 << 8},
      X86_FMA4{"x86_fma4", 1 << 9}, X86_XOP{"x86_xop", 1 << 10}, X86_AVX512F{"x86_avx512f", 1 << 11},
      X86_AVX512BW{"x86_avx512bw", 1 << 12}, X86_AVX512DQ{"x86_avx512dq", 1 << 13},
      X86_AVX512VL{"x86_avx512vl", 1 << 14};

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
 * Cache configuration of a CPU
 *
 * 0 means unknown or not present.  We can't 100% detect which.
 *
 * Mnemonics are u=unified, i=instruction, d=data.  For the most part l2 and l3 are unified, l1 may or may not be split.
 * In the case where this is ambiguous (I'm looking at you AMD) we "default" to data.  In practice, unified caches are
 * usually 0.
 * */
struct CpuCaches {
  unsigned int l1i, l1d, l1u, l2i, l2d, l2u, l3i, l3d, l3u;
};

enum class CpuManufacturer { UNKNOWN, INTEL, AMD, APPLE };

enum class CpuArchitecture { AARCH64, X86 };

const char *cpuManufacturerToString(CpuManufacturer man);
const char *cpuArchitectureToString(CpuArchitecture arch);

struct SystemInfo {
  CpuCapabilities cpu_capabilities;
  CpuManufacturer cpu_manufacturer;
  CpuArchitecture cpu_architecture;
  CpuCaches cache_info;
};

/**
 *  Get uncached CPU capabilities.
 *
 * This is an expensive function, easily on the order of 10k cycles.  Users should cache the results.
 */
SystemInfo getSystemInfoUncached();

/*
 * Get cached system information.  After the first slow invocation, this function is an atomic load.
 */
SystemInfo getSystemInfo();

/**
 * Return a somewhat opaque JSON string that is useful for displaying or parsing in other languages.
 *
 * Primarily useful for debugging, or pulling apart simdsp in e.g. Rust or Python without having to write a bunch of
 * manual code.
 *
 * Should be free(2)d.
 * */
char *convertSystemInfoToJson(SystemInfo *sysinfo);

} // namespace simdsp
