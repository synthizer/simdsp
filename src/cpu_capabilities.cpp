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

#include "simdsp/cpu_capabilities.hpp"

#include <stddef.h>
#include <stdint.h>

#if __GNUC__
#include <cpuid.h>
#endif

#if _MSC_VER
#include <immintrin.h>
#include <intrin.h> // __cpuidex on MSVC 2017
#endif

namespace simdsp {
/*
 * by using this intermediate array, we can guarantee the count is correct with sizeof.
 */
static const CpuBit BITS_ARRAY[] = {
    CpuCapabilities::X86_SSE2,     CpuCapabilities::X86_SSE3,        CpuCapabilities::X86_SSSE3,
    CpuCapabilities::X86_SSE4_1,   CpuCapabilities::X86_POPCNT_INSN, CpuCapabilities::X86_AVX,
    CpuCapabilities::X86_AVX2,     CpuCapabilities::X86_FMA3,        CpuCapabilities::X86_FMA4,
    CpuCapabilities::X86_XOP,      CpuCapabilities::X86_AVX512F,     CpuCapabilities::X86_AVX512BW,
    CpuCapabilities::X86_AVX512DQ, CpuCapabilities::X86_AVX512VL,
};

const CpuBit *CpuCapabilities::ALL_BITS = BITS_ARRAY;
const size_t CpuCapabilities::ALL_BITS_COUNT = sizeof(BITS_ARRAY) / sizeof(BITS_ARRAY[0]);

/*
 * We want these operators and helper functions out of line just in case the linker decides to be cute.
 */

CpuCapabilities::CpuCapabilities(unsigned int _bits) : bits(_bits) {}

CpuCapabilities::operator bool() { return this->bits != 0; }

#define BOOL_OPS(OP)                                                                                                   \
  CpuCapabilities operator OP(const CpuCapabilities &a, const CpuCapabilities &b) {                                    \
    CpuCapabilities ret = a;                                                                                           \
    ret.bits OP## = b.bits;                                                                                            \
    return ret;                                                                                                        \
  }                                                                                                                    \
                                                                                                                       \
  CpuCapabilities &operator OP##=(CpuCapabilities &a, const CpuCapabilities &b) {                                      \
    a.bits OP## = b.bits;                                                                                              \
    return a;                                                                                                          \
  }                                                                                                                    \
                                                                                                                       \
  CpuCapabilities operator OP(const CpuCapabilities &a, const CpuBit &b) {                                             \
    CpuCapabilities ret = a;                                                                                           \
    ret.bits OP## = b.bit;                                                                                             \
    return ret;                                                                                                        \
  }                                                                                                                    \
                                                                                                                       \
  CpuCapabilities operator OP(const CpuBit &a, const CpuCapabilities &b) { return b OP a; }                            \
                                                                                                                       \
  CpuCapabilities &operator OP##=(CpuCapabilities &a, const CpuBit &b) {                                               \
    a.bits OP## = b.bit;                                                                                               \
    return a;                                                                                                          \
  }

BOOL_OPS(|)
BOOL_OPS(&)
BOOL_OPS(^)

#undef BOOL_OPS

namespace detail {

inline void get_cpuid(unsigned level, unsigned subleaf, unsigned *eax, unsigned *ebx, unsigned *ecx, unsigned *edx) {
#if __GNUC__
  __cpuid_count(level, subleaf, *eax, *ebx, *ecx, *edx);
#elif _MSC_VER
  uint32_t regs[4];
  __cpuidex((int *)regs, level, subleaf);
  *eax = regs[0];
  *ebx = regs[1];
  *ecx = regs[2];
  *edx = regs[3];
#else
#error "unsupported compiler"
#endif
}

inline uint64_t get_xcr(unsigned level) {
#if (defined(_MSC_FULL_VER) && _MSC_FULL_VER >= 160040000) || (defined(__INTEL_COMPILER) && __INTEL_COMPILER >= 1200)
  return _xgetbv(level);
#elif defined(__GNUC__) || defined(__clang__)
  uint32_t eax, edx;
  __asm("xgetbv" : "=a"(eax), "=d"(edx) : "c"(level) :);
  return eax | (uint64_t(edx) << 32);
#else
  return 0;
#endif
}

enum cpu_manufacturer { CPU_INTEL, CPU_AMD, CPU_UNKNOWN };

static inline cpu_manufacturer get_cpu_manufacturer(uint32_t ebx, uint32_t ecx, uint32_t edx) {
  if (ebx == 0x756E6547 && ecx == 0x6C65746E && edx == 0x49656E69)
    return CPU_INTEL; // "GenuineIntel"
  if (ebx == 0x68747541 && ecx == 0x444D4163 && edx == 0x69746E65)
    return CPU_AMD; // "AuthenticAMD"
  return CPU_UNKNOWN;
}

} // namespace detail

CpuCapabilities getCpuCapabilities() {
  CpuCapabilities caps{};

  uint32_t eax, ebx, ecx, edx;
  bool xsave_xrstore_avail = false;

  simdsp::detail::get_cpuid(0, 0, &eax, &ebx, &ecx, &edx);
  unsigned int max_cpuid_level = eax;
  simdsp::detail::cpu_manufacturer mfg = simdsp::detail::get_cpu_manufacturer(ebx, ecx, edx);

  simdsp::detail::get_cpuid(0x80000000, 0, &eax, &ebx, &ecx, &edx);
  unsigned int max_ex_cpuid_level = eax;

  if (max_cpuid_level >= 0x00000001) {
    simdsp::detail::get_cpuid(0x00000001, 0, &eax, &ebx, &ecx, &edx);

    if (edx & (1u << 26))
      caps |= CpuCapabilities::X86_SSE2;
    if (ecx & (1u << 0))
      caps |= CpuCapabilities::X86_SSE3;
    if (ecx & (1u << 9))
      caps |= CpuCapabilities::X86_SSSE3;
    if (ecx & (1u << 19))
      caps |= CpuCapabilities::X86_SSE4_1;
    if (ecx & (1u << 20) && mfg == simdsp::detail::CPU_INTEL)
      caps |= CpuCapabilities::X86_POPCNT_INSN; // popcnt is included in SSE4.2 on Intel
    if (ecx & (1u << 23))
      caps |= CpuCapabilities::X86_POPCNT_INSN;
    if (ecx & (1u << 12))
      caps |= CpuCapabilities::X86_FMA3;
    if (ecx & (1u << 26)) {
      // XSAVE/XRSTORE available on hardware, now check OS support
      uint64_t xcr = simdsp::detail::get_xcr(0);
      if ((xcr & 6) == 6)
        xsave_xrstore_avail = true;
    }

    if (ecx & (1u << 28) && xsave_xrstore_avail)
      caps |= CpuCapabilities::X86_AVX;
  }
  if (max_ex_cpuid_level >= 0x80000001) {
    simdsp::detail::get_cpuid(0x80000001, 0, &eax, &ebx, &ecx, &edx);
    if (ecx & (1u << 16))
      caps |= CpuCapabilities::X86_FMA4;
    if (ecx & (1u << 11))
      caps |= CpuCapabilities::X86_XOP;
  }

  if (max_cpuid_level >= 0x00000007) {
    simdsp::detail::get_cpuid(0x00000007, 0, &eax, &ebx, &ecx, &edx);
    if (ebx & (1u << 5) && xsave_xrstore_avail)
      caps |= CpuCapabilities::X86_AVX2;
    if (ebx & (1u << 16) && xsave_xrstore_avail)
      caps |= CpuCapabilities::X86_AVX512F;
    if (ebx & (1u << 30) && xsave_xrstore_avail)
      caps |= CpuCapabilities::X86_AVX512BW;
    if (ebx & (1u << 17) && xsave_xrstore_avail)
      caps |= CpuCapabilities::X86_AVX512DQ;
    if (ebx & (1u << 31) && xsave_xrstore_avail)
      caps |= CpuCapabilities::X86_AVX512VL;
  }

  return caps;
}

} // namespace simdsp
