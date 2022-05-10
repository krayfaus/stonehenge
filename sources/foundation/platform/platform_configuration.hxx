// SPDX-License-Identifier: GPL-2.0-only
// Copyright 2022 Ítalo "Krayfaus" Cadeu. All rights reserved.
// This source file is part of the Aethelwerka Component Library.
#pragma once

// clang-format off

//-----------------------------------------------------------------------------
// Target Platform :: Architecture
//-----------------------------------------------------------------------------
#if defined(_M_X64) || defined(_M_AMD64) || defined(__x86_64__)
	#define AETHELWERKA_PLATFORM_ARCH            "x64"
	#define AETHELWERKA_PLATFORM_X64             true
#elif defined(_M_IX86) || defined(i386) || defined(__i386__) || defined(__i386)
	#define AETHELWERKA_PLATFORM_ARCH            "x86"
	#define AETHELWERKA_PLATFORM_X86             true
#elif defined(__aarch64__) || defined(_M_ARM64)
	#define AETHELWERKA_PLATFORM_ARCH            "ARM64"
	#define AETHELWERKA_PLATFORM_ARM64           true
#elif defined(__arm__) || defined(_M_ARM)
	#define AETHELWERKA_PLATFORM_ARCH            "ARM32"
	#define AETHELWERKA_PLATFORM_ARM32           true
#elif defined(mips) || defined(__mips__) || defined(__mips)
	#define AETHELWERKA_PLATFORM_ARCH            "MIPS"
	#define AETHELWERKA_PLATFORM_MIPS            true
#elif defined(__sh__)
	#define AETHELWERKA_PLATFORM_ARCH            "SuperH"
	#define AETHELWERKA_PLATFORM_SUPERH          true
#elif defined(__powerpc) || defined(__powerpc__) || defined(__powerpc64__) || defined(__POWERPC__) || defined(__ppc__) || defined(__PPC__) || defined(_ARCH_PPC)
	#define AETHELWERKA_PLATFORM_ARCH            "PPC"
	#define AETHELWERKA_PLATFORM_POWERPC         true
#elif defined(__PPC64__) || defined(__ppc64__) || defined(_ARCH_PPC64)
	#define AETHELWERKA_PLATFORM_ARCH            "PPC64"
	#define AETHELWERKA_PLATFORM_POWERPC64       true
#elif defined(__sparc__) || defined(__sparc)
	#define AETHELWERKA_PLATFORM_ARCH            "SPARC"
	#define AETHELWERKA_PLATFORM_SPARC           true
#elif defined(__m68k__)
	#define AETHELWERKA_PLATFORM_ARCH            "M68K"
	#define AETHELWERKA_PLATFORM_M68K            true
#else
	#error "Unknown or unsupported architecture."
#endif


//-----------------------------------------------------------------------------
// Target Platform :: Operating System
//-----------------------------------------------------------------------------
#if defined(_WIN64) or defined(_WIN32)
	#define AETHELWERKA_PLATFORM_OS              "Windows"
	#define AETHELWERKA_PLATFORM_WINDOWS         true
#elif defined(_DURANGO)
	#define AETHELWERKA_PLATFORM_OS              "Xbox One"
	#define AETHELWERKA_PLATFORM_DURANGO         true
#elif defined(__ORBIS__)
	#define AETHELWERKA_PLATFORM_OS              "PS4"
	#define AETHELWERKA_PLATFORM_ORBIS           true
#elif defined(__ANDROID__)
	#define AETHELWERKA_PLATFORM_OS              "Android"
	#define AETHELWERKA_PLATFORM_ANDROID         true
	#define AETHELWERKA_PLATFORM_POSIX           true
#elif defined(__linux__)
	#define AETHELWERKA_PLATFORM_OS              "Linux"
	#define AETHELWERKA_PLATFORM_LINUX           true
	#define AETHELWERKA_PLATFORM_POSIX           true
#elif defined(__HAIKU__)
	#define AETHELWERKA_PLATFORM_OS              "Haiku"
	#define AETHELWERKA_PLATFORM_HAIKU           true
	#define AETHELWERKA_PLATFORM_POSIX           true
#elif defined(__unix__) or not defined(__APPLE__) and defined(__MACH__)
	#if __has_include(<sys/param.h>)
		#include <sys/param.h>
		#if defined(BSD)
			#define AETHELWERKA_PLATFORM_OS      "BSD"
			#define AETHELWERKA_PLATFORM_BSD     true
			#define AETHELWERKA_PLATFORM_POSIX   true
		#endif
	#endif
#elif defined(__APPLE__) and defined(__MACH__)
	#if __has_include(<TargetConditionals.h>)
		#include <TargetConditionals.h>
		#if TARGET_IPHONE_SIMULATOR == 1 or TARGET_OS_IPHONE == 1
			#define AETHELWERKA_PLATFORM_OS      "iOS"
			#define AETHELWERKA_PLATFORM_IOS     true
			#define AETHELWERKA_PLATFORM_APPLE   true
			#define AETHELWERKA_PLATFORM_POSIX   true
		#elif TARGET_OS_MAC == 1
			#define AETHELWERKA_PLATFORM_OS      "macOS"
			#define AETHELWERKA_PLATFORM_OSX     true
			#define AETHELWERKA_PLATFORM_POSIX   true
			#define AETHELWERKA_PLATFORM_APPLE   true
		#endif
	#endif
#else
	#error "Unknown or unsupported operating system."
#endif


//-----------------------------------------------------------------------------
// Target Platform :: Platform-Specific Definitions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// _WIN64 is defined by both 32-bit and 64-bit Windows,
//   so we need to check the architecture of the target platform.
#if AETHELWERKA_PLATFORM_WINDOWS
	#if AETHELWERKA_PLATFORM_X64
		#define AETHELWERKA_PLATFORM_WIN64       true
	#else
		#define AETHELWERKA_PLATFORM_WIN32       true
	#endif
#endif

//-----------------------------------------------------------------------------
// The following definitions are here mostly for completeness.
// It's unlikely that a cross-platform application will need them.
#if AESTHETEWERKA_PLATFORM_DURANGO or AETHELWERKA_PLATFORM_ORBIS
	#define AETHELWERKA_PLATFORM_DEVICE          "Console"
	#define AETHELWERKA_PLATFORM_CONSOLE         true
#elif AETHELWERKA_PLATFORM_ANDROID or AETHELWERKA_PLATFORM_IOS
	#define AETHELWERKA_PLATFORM_DEVICE          "Mobile"
	#define AETHELWERKA_PLATFORM_MOBILE          true
#else // Windows, Linux, BSD, macOS, etc.
	#define AETHELWERKA_PLATFORM_DEVICE          "Desktop"
	#define AETHELWERKA_PLATFORM_DESKTOP         true
#endif


//-----------------------------------------------------------------------------
// Host Platform :: Compiler
//-----------------------------------------------------------------------------
#if defined(__clang__)
	#define AETHELWERKA_PLATFORM_COMPILER        "Clang"
	#define AETHELWERKA_PLATFORM_COMPILER_CLANG  true
// __clang__ is defined by clang-cl compiler.
#elif defined(_MSC_VER)
	#define AETHELWERKA_PLATFORM_COMPILER        "Microsoft Visual C/C++"
	#define AETHELWERKA_PLATFORM_COMPILER_MSVC            true
// _MSC_VER is defined by Microsoft Visual C/C++ compiler.
#elif defined(__GNUC__)
	#define AETHELWERKA_PLATFORM_COMPILER        "GNU C/C++"
	#define AETHELWERKA_PLATFORM_COMPILER_GNUC   true
// __GNUC__ is defined by GNU C/C++ compiler.
#elif defined(__INTEL_COMPILER)
	#define AETHELWERKA_PLATFORM_COMPILER        "Intel C/C++"
	#define AETHELWERKA_PLATFORM_COMPILER_INTEL  true
//  __INTEL_COMPILER is defined by both ICC and EDG based compilers
#elif defined(__IBMCPP__) or defined(__xlC__) or defined(__xlc__)
	#define AETHELWERKA_PLATFORM_COMPILER        "IBM XL C/C++"
	#define AETHELWERKA_PLATFORM_COMPILER_IBM    true
// __IBMC__ is defined by both XL and Visualage compilers.
#elif defined(__llvm__)
	#define AETHELWERKA_PLATFORM_COMPILER        "LLVM"
	#define AETHELWERKA_PLATFORM_COMPILER_LLVM   true
// __clang__ is defined by clang-cl compiler.
#else
	#error "Unknown or unsupported compiler."
#endif
