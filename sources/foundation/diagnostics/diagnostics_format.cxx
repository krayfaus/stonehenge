// SPDX-License-Identifier: GPL-2.0-only
// Copyright 2022 Ítalo "Krayfaus" Cadeu. All rights reserved.
// This source file is part of the Aethelwerka C++ Library.
module;

#if __has_include(<format>)
#	include <format>
#	define AETHELWERKA_USING_STD_FORMAT
#elif __has_include(<fmt/core.h>)
#	include <fmt/core.h>
#	define AETHELWERKA_USING_FMT_FORMAT
#endif

// This partition is a wrapper around the format function found in C++20 and libfmt.
export module aethelwerka.foundation.diagnostics:format;

export namespace aethelwerka
{

#if defined(AETHELWERKA_USING_STD_FORMAT)
	using std::format;
#elif defined(AETHELWERKA_USING_FMT_FORMAT)
	using fmt::format;
#else
#	error "Text formatting utilities not available."
#endif

}  // namespace aethelwerka
