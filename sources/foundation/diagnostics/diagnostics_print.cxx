// SPDX-License-Identifier: GPL-2.0-only
// Copyright 2022 Ítalo "Krayfaus" Cadeu. All rights reserved.
// This source file is part of the Aethelwerka C++ Library.
module;

#include <string_view>
#include <iostream>

#if __has_include(<fmt/core.h>)
#	include <fmt/core.h>
#	define AETHELWERKA_USING_FMT_PRINT
#endif

// This partition is a wrapper around the print function, expected for C++23 (P2093R14).
export module aethelwerka.foundation.diagnostics:print;

import :format;  // Using: {aethelwerka::format};

export namespace aethelwerka
{

#if defined(AETHELWERKA_USING_FMT_PRINT)
	using fmt::print;
#else
	template <typename... Args>
	void print(std::string_view format, Args &&...args)
	{
		auto message = aethelwerka::format(format, std::forward<Args>(args)...);
		::fprintf(stdout, message.c_str());
	}
#endif

}  // namespace aethelwerka
