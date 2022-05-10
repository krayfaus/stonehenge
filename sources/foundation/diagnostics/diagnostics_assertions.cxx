// SPDX-License-Identifier: GPL-2.0-only
// Copyright 2022 Ítalo "Krayfaus" Cadeu. All rights reserved.
// This source file is part of the Aethelwerka C++ Library.
// Available at: <https://github.com/aethelwerka/monolith>.
module;

#include <cstdlib>      // Using: {EXIT_FAILURE};
#include <iostream>     // Using: {std::cout, std::flush};
#include <string_view>  // Using: {std::string_view};

// This partition provides assertion utilities.
export module aethelwerka.foundation.diagnostics:assertions;

import :format; // Using: {aethelwerka::format};
import :print; // Using: {aethelwerka::print};

// Internals:
namespace aethelwerka::internal
{

	static constexpr bool assert_enabled_at_runtime      = true;
	static constexpr bool assert_print_debug_information = true;

	inline void on_assert [[noreturn]] (auto &&... args)
	{
		// todo (Krayfaus): improve message formatting, use std::source_location.
		//
		if constexpr (assert_print_debug_information and sizeof...(args) > 1u)
		{

			aethelwerka::print("Failed Assertion: {}\n", args...);
		}
		else
		{
			aethelwerka::print("Failed Assertion.\n");
		}

		// todo (Krayfaus): Replace std::exit with std::quick_exit.
		//
		// For some reason, std::quick_exit is so fast that the assert message is not printed.
		//
	    std::exit(EXIT_FAILURE);
	}
	template <typename... Args>
	inline auto assert([[maybe_unused]] bool condition, Args... args) -> void
	{
		if constexpr (assert_enabled_at_runtime)
		{
			return condition ? static_cast<void>(0) : on_assert(std::forward<Args>(args)...);
		}
	}

}  // namespace aethelwerka::internal

#define CREATE_TEMPLATE_FUNCTION_ALIAS(HighLevelFunction, LowLevelFunction) \
	template <typename... Args>                                             \
	inline auto HighLevelFunction(Args &&...args)                           \
	{                                                                       \
		return LowLevelFunction(std::forward<Args>(args)...);               \
	}


//-----------------------------------------------------------------------------

export namespace aethelwerka
{

	// Pre-conditional assert.
	CREATE_TEMPLATE_FUNCTION_ALIAS(expect, internal::assert);

	// Post-conditional assert.
	CREATE_TEMPLATE_FUNCTION_ALIAS(ensure, internal::assert);

}  // namespace aethelwerka
