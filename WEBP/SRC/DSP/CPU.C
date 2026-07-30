// Copyright 2011 Google Inc. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the COPYING file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS. All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.
// -----------------------------------------------------------------------------
//
// CPU detection
//
// Author: Christian Duvivier (cduvivier@google.com)

#include "src/dsp/dsp.h"

#if defined(WEBP_HAVE_NEON_RTCD)
#include <stdio.h>
#include <string.h>
#endif

#if defined(WEBP_ANDROID_NEON)
#include <cpu-features.h>
#endif

#if !defined(__native_client__) && (defined(__i386__) || defined(__x86_64__))
static WEBP_INLINE uint64_t xgetbv(void) {
  const uint32_t ecx = 0;
  uint32_t eax, edx;
  __asm__ volatile (
    ".byte 0x0f, 0x01, 0xd0\n"
    : "=a"(eax), "=d"(edx) : "c" (ecx));
  return ((uint64_t)edx << 32) | eax;
}
#elif defined(_MSC_VER) && defined(_M_IX86)
static WEBP_INLINE uint64_t xgetbv(void) {
  uint32_t eax_, edx_;
  __asm {
    xor ecx, ecx
    __asm _emit 0x0f __asm _emit 0x01 __asm _emit 0xd0
    mov eax_, eax
    mov edx_, edx
  }
  return ((uint64_t)edx_ << 32) | eax_;
}
#else
#define xgetbv() 0U
#endif

#if defined(__i386__) || defined(__x86_64__) || defined(WEBP_HAVE_MSC_CPUID)

static int CheckSlowModel(int info) {
  static const uint8_t kSlowModels[] = {
    0x37, 0x4a, 0x4d,
    0x1c, 0x26, 0x27
  };
  const uint32_t model = ((info & 0xf0000) >> 12) | ((info >> 4) & 0xf);
  const uint32_t family = (info >> 8) & 0xf;
  if (family == 0x06) {
    size_t i;
    for (i = 0; i < sizeof(kSlowModels) / sizeof(kSlowModels[0]); ++i) {
      if (model == kSlowModels[i]) return 1;
    }
  }
  return 0;
}

static int x86CPUInfo(CPUFeature feature) {
  int max_cpuid_value;
  int cpu_info[4];
  int is_intel = 0;

  GetCPUInfo(cpu_info, 0);
  max_cpuid_value = cpu_info[0];
  if (max_cpuid_value < 1) {
    return 0;
  } else {
    const int VENDOR_ID_INTEL_EBX = 0x756e6547;
    const int VENDOR_ID_INTEL_EDX = 0x49656e69;
    const int VENDOR_ID_INTEL_ECX = 0x6c65746e;
    is_intel = (cpu_info[1] == VENDOR_ID_INTEL_EBX &&
                cpu_info[2] == VENDOR_ID_INTEL_ECX &&
                cpu_info[3] == VENDOR_ID_INTEL_EDX);
  }

  GetCPUInfo(cpu_info, 1);
  if (feature == kSSE2) {
    return !!(cpu_info[3] & (1 << 26));
  }
  if (feature == kSSE3) {
    return !!(cpu_info[2] & (1 << 0));
  }
  if (feature == kSlowSSSE3) {
    if (is_intel && (cpu_info[2] & (1 << 9))) {
      return CheckSlowModel(cpu_info[0]);
    }
    return 0;
  }

  if (feature == kSSE4_1) {
    return !!(cpu_info[2] & (1 << 19));
  }
  if (feature == kAVX) {
    if ((cpu_info[2] & 0x18000000) == 0x18000000) {
      return (xgetbv() & 0x6) == 0x6;
    }
  }
  if (feature == kAVX2) {
    if (x86CPUInfo(kAVX) && max_cpuid_value >= 7) {
      GetCPUInfo(cpu_info, 7);
      return !!(cpu_info[1] & (1 << 5));
    }
  }
  return 0;
}
VP8CPUInfo VP8GetCPUInfo = x86CPUInfo;
#elif defined(WEBP_ANDROID_NEON)
static int AndroidCPUInfo(CPUFeature feature) {
  const AndroidCpuFamily cpu_family = android_getCpuFamily();
  const uint64_t cpu_features = android_getCpuFeatures();
  if (feature == kNEON) {
    return cpu_family == ANDROID_CPU_FAMILY_ARM &&
           (cpu_features & ANDROID_CPU_ARM_FEATURE_NEON) != 0;
  }
  return 0;
}
VP8CPUInfo VP8GetCPUInfo = AndroidCPUInfo;
#elif defined(EMSCRIPTEN)
static int wasmCPUInfo(CPUFeature feature) {
  switch (feature) {
#ifdef WEBP_USE_SSE2
    case kSSE2:
      return 1;
#endif
#ifdef WEBP_USE_SSE41
    case kSSE3:
    case kSlowSSSE3:
    case kSSE4_1:
      return 1;
#endif
#ifdef WEBP_USE_NEON
    case kNEON:
      return 1;
#endif
    default:
      break;
  }
  return 0;
}
VP8CPUInfo VP8GetCPUInfo = wasmCPUInfo;
#elif defined(WEBP_USE_NEON)
static int armCPUInfo(CPUFeature feature) {
  if (feature != kNEON) return 0;
#if defined(__linux__) && defined(WEBP_HAVE_NEON_RTCD)
  {
    int has_neon = 0;
    char line[200];
    FILE* const cpuinfo = fopen("/proc/cpuinfo", "r");
    if (cpuinfo == NULL) return 0;
    while (fgets(line, sizeof(line), cpuinfo)) {
      if (!strncmp(line, "Features", 8)) {
        if (strstr(line, " neon ") != NULL) {
          has_neon = 1;
          break;
        }
      }
    }
    fclose(cpuinfo);
    return has_neon;
  }
#else
  return 1;
#endif
}
VP8CPUInfo VP8GetCPUInfo = armCPUInfo;
#elif defined(WEBP_USE_MIPS32) || defined(WEBP_USE_MIPS_DSP_R2) || \
      defined(WEBP_USE_MSA)
static int mipsCPUInfo(CPUFeature feature) {
  if ((feature == kMIPS32) || (feature == kMIPSdspR2) || (feature == kMSA)) {
    return 1;
  } else {
    return 0;
  }
}
VP8CPUInfo VP8GetCPUInfo = mipsCPUInfo;
#else
VP8CPUInfo VP8GetCPUInfo = NULL;
#endif
