#pragma once

/*
 * Defines macros which are used to determine what is available in the current compilation unit.  For example, "is this
 * Intel? Do we definitely have avx?".
 * */

/*
 * Exactly one of these will be defined:
 *
 * - SIMDSP_IS_X86: either 32-bit or 64-bit X86.
 * - SIMDSP_IS_AARCH64: Aarch64.
 **/

#if !defined(_MSC_VER)
#if defined(__aarch64__) || defined(_M_ARM64)
#define SIMDSP_IS_AARCH64 1
#elif defined(__i386__) || defined(__x86_64__)
#define SIMDSP_IS_X86 1
#endif
#else
// For MSVC, we're either ARM or not.
#if defined(_M_ARM64)
#define SIMDSP_IS_AARCH64
#elif !defined(_M_ARM)
#define SIMDSP_IS_X86 1
#endif
#endif

#if (defined(SIMDSP_IS_X86) + defined(SIMDSP_IS_AARCH64)) != 1
#error Unable to unambiguously determine architecture.
#endif
