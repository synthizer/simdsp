#include "simdsp/system_info.hpp"

#include <cstring>
#include <iostream>
#include <map>
#include <sstream>
#include <stddef.h>
#include <variant>

// Normally we don't do this, but using iostreams is suepr super annoying without it.
using namespace std;

namespace simdsp {

template <typename V> static void jsonWriteKv(ostringstream &out, const char *k, const V &v);

static void jsonify(ostringstream &out, const char *v) { out << '"' << v << '"'; }

static void jsonify(ostringstream &out, bool v) { out << (v ? "true" : "false"); }

static void jsonify(ostringstream &out, unsigned int v) { out << v; }

template <typename V> static void jsonify(ostringstream &out, const map<string, V> &v) {
  out << '{';

  size_t len = v.size();
  size_t seen = 0;

  for (const auto &entry : v) {
    seen += 1;

    jsonWriteKv(out, entry.first.c_str(), entry.second);
    if (seen < len) {
      out << ',';
    }
  }

  out << '}';
}

template <typename V> static void jsonWriteKv(ostringstream &out, const char *k, const V &v) {
  out << '"' << k << '"' << ':';
  jsonify(out, v);
}

char *convertSystemInfoToJson(SystemInfo *sysinfo) {
  ostringstream out;
  map<string, bool> cpu_capabilities;
  map<string, unsigned int> cache_info;

  for (size_t i = 0; i < CpuCapabilities::ALL_BITS_COUNT; i++) {
    const CpuBit &bit = CpuCapabilities::ALL_BITS[i];
    bool has = (sysinfo->cpu_capabilities & bit) != 0;
    cpu_capabilities[bit.printable_string] = has;
  }

#define CVAR(X) cache_info[#X] = sysinfo->cache_info.X
#define CLEV(L)                                                                                                        \
  CVAR(l##L##i);                                                                                                       \
  CVAR(l##L##d);                                                                                                       \
  CVAR(l##L##u)

  CLEV(1);
  CLEV(2);
  CLEV(3);
#undef CLEV
#undef CVAR

  out << '{';
  jsonWriteKv(out, "cache_info", cache_info);
  out << ',';
  jsonWriteKv(out, "cpu_capabilities", cpu_capabilities);
  out << ',';
  jsonWriteKv(out, "cpu_manufacturer", cpuManufacturerToString(sysinfo->cpu_manufacturer));
  out << '}';

  return strdup(out.str().c_str());
}

} // namespace simdsp