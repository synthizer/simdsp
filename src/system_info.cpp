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

#include "simdsp/system_info.hpp"

#include <atomic>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

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

static inline void runCpuId(unsigned level, unsigned subleaf, unsigned *eax, unsigned *ebx, unsigned *ecx,
                            unsigned *edx) {
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

/*
 * Helper macro to call cpuid without mixing up the order of the registers. Expects variables eax, ebx, ecx, and edx to
 * be in scope.
 **/
#define SAFE_CPUID(LEVEL, SUBLEAF) runCpuId((LEVEL), (SUBLEAF), &eax, &ebx, &ecx, &edx)

static inline CpuManufacturer getCpuManufacturer() {
  unsigned int eax, ebx, ecx, edx;

  SAFE_CPUID(0, 0);

  if (ebx == 0x756E6547 && ecx == 0x6C65746E && edx == 0x49656E69)
    return CpuManufacturer::INTEL; // "GenuineIntel"
  if (ebx == 0x68747541 && ecx == 0x444D4163 && edx == 0x69746E65)
    return CpuManufacturer::AMD; // "AuthenticAMD"
  return CpuManufacturer::UNKNOWN;
}

const char *cpuManufacturerToString(CpuManufacturer man) {
  if (man == CpuManufacturer::INTEL) {
    return "intel";
  } else if (man == CpuManufacturer::AMD) {
    return "amd";
  } else {
    return "unknown";
  }
}

inline uint64_t getXcr(unsigned level) {
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

CpuCapabilities getCpuCapabilities() {
  CpuCapabilities caps{};

  uint32_t eax, ebx, ecx, edx;
  bool xsave_xrstore_avail = false;

  runCpuId(0, 0, &eax, &ebx, &ecx, &edx);
  unsigned int max_cpuid_level = eax;
  auto mfg = getCpuManufacturer();

  runCpuId(0x80000000, 0, &eax, &ebx, &ecx, &edx);
  unsigned int max_ex_cpuid_level = eax;

  if (max_cpuid_level >= 0x00000001) {
    runCpuId(0x00000001, 0, &eax, &ebx, &ecx, &edx);

    if (edx & (1u << 26))
      caps |= CpuCapabilities::X86_SSE2;
    if (ecx & (1u << 0))
      caps |= CpuCapabilities::X86_SSE3;
    if (ecx & (1u << 9))
      caps |= CpuCapabilities::X86_SSSE3;
    if (ecx & (1u << 19))
      caps |= CpuCapabilities::X86_SSE4_1;
    if (ecx & (1u << 20) && mfg == CpuManufacturer::INTEL)
      caps |= CpuCapabilities::X86_POPCNT_INSN; // popcnt is included in SSE4.2 on Intel
    if (ecx & (1u << 23))
      caps |= CpuCapabilities::X86_POPCNT_INSN;
    if (ecx & (1u << 12))
      caps |= CpuCapabilities::X86_FMA3;
    if (ecx & (1u << 26)) {
      // XSAVE/XRSTORE available on hardware, now check OS support
      uint64_t xcr = getXcr(0);
      if ((xcr & 6) == 6)
        xsave_xrstore_avail = true;
    }

    if (ecx & (1u << 28) && xsave_xrstore_avail)
      caps |= CpuCapabilities::X86_AVX;
  }
  if (max_ex_cpuid_level >= 0x80000001) {
    runCpuId(0x80000001, 0, &eax, &ebx, &ecx, &edx);
    if (ecx & (1u << 16))
      caps |= CpuCapabilities::X86_FMA4;
    if (ecx & (1u << 11))
      caps |= CpuCapabilities::X86_XOP;
  }

  if (max_cpuid_level >= 0x00000007) {
    runCpuId(0x00000007, 0, &eax, &ebx, &ecx, &edx);
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

/*
 * Helper macro to call cpuid without mixing up the order of the registers. Expects variables eax, ebx, ecx, and edx to
 * be in scope.
 **/
#define SAFE_CPUID(LEVEL, SUBLEAF) runCpuId((LEVEL), (SUBLEAF), &eax, &ebx, &ecx, &edx)

CpuCaches getCpuCacheInfo() {
  unsigned int eax = 0, ebx = 0, ecx = 0, edx = 0;
  CpuCaches ret{0};

  SAFE_CPUID(0, 0);
  unsigned int max_fn = eax;

  if (max_fn < 4) {
    return ret;
  }

  for (unsigned int subleaf = 0;; subleaf++) {
    // Which cache field are we writing the size to?
    unsigned int *dest = nullptr;
    unsigned int cache_type, cache_level;

    SAFE_CPUID(4, subleaf);
    cache_type = eax & 0xf;
    cache_level = (eax >> 5) & 0x7;

    if (cache_type == 1) {
      switch (cache_level) {
      case 1:
        dest = &ret.l1d;
        break;
      case 2:
        dest = &ret.l2d;
        break;
      case 3:
        dest = &ret.l3d;
        break;
      }
    } else if (cache_type == 2) {
      switch (cache_level) {
      case 1:
        dest = &ret.l1i;
        break;
      case 2:
        dest = &ret.l2i;
        break;
      case 3:
        dest = &ret.l3i;
        break;
      }
    } else if (cache_type == 3) {
      switch (cache_level) {
      case 1:
        dest = &ret.l1u;
        break;
      case 2:
        dest = &ret.l2u;
        break;
      case 3:
        dest = &ret.l3u;
        break;
      }
    }

    if (dest == nullptr) {
      if (cache_type == 0) {
        // This is the end.
        break;
      }

      // We cant handle this one so skip.
      continue;
    }

    // Taken from here:
    // https://stackoverflow.com/questions/14283171/how-to-receive-l1-l2-l3-cache-size-using-cpuid-instruction-in-x86
    unsigned int ways = (ebx >> 22) + 1;
    unsigned int partitions = ((ebx >> 12) & 0x3ff) + 1;
    unsigned int coherency = (ebx & 0x3ff) + 1;
    unsigned int sets = ecx + 1;
    *dest = partitions * coherency * ways * sets;
  }

  return ret;
}

SystemInfo getSystemInfoUncached() {
  SystemInfo sysinfo{};

  sysinfo.cpu_capabilities = getCpuCapabilities();
  sysinfo.cpu_manufacturer = getCpuManufacturer();
  sysinfo.cache_info = getCpuCacheInfo();
  return sysinfo;
}

enum {
  INFO_CACHE_UNINITIALIZED = 0,
  INFO_CACHE_INITIALIZING,
  INFO_CACHE_INITIALIZED,
};

static struct {
  std::atomic<unsigned int> state = INFO_CACHE_UNINITIALIZED;
  SystemInfo info;
} info_cache;

SystemInfo getSystemInfo() {
  // Fast path: load and return.
  //
  // Must be acquire because of the first initialization.
  unsigned int old_state = info_cache.state.load(std::memory_order_acquire);

  if (old_state == INFO_CACHE_INITIALIZED) {
    return info_cache.info;
  }

  // If we got an old_state of uninitialized, try to win the CAS race and put the cache in place.
  if (old_state == INFO_CACHE_UNINITIALIZED) {
    if (info_cache.state.compare_exchange_strong(old_state, INFO_CACHE_INITIALIZING, std::memory_order_relaxed,
                                                 std::memory_order_relaxed) == true) {
      info_cache.info = getSystemInfoUncached();
      // Release, since capabilities themselves aren't atomic.
      info_cache.state.store(INFO_CACHE_INITIALIZED, std::memory_order_release);
    }
  }

  // Slow path: someone else is initializing, which takes on the order of a couple microseconds.  Spin until that's
  // done.  This must be acquire because of the update from the other thread, which touches the non-atomic field.
  while (info_cache.state.load(std::memory_order_acquire) != INFO_CACHE_INITIALIZED)
    ;

  return info_cache.info;
}

} // namespace simdsp
