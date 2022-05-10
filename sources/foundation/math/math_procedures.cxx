// SPDX-License-Identifier: GPL-2.0-only
// Copyright 2022 Ítalo "Krayfaus" Cadeu. All rights reserved.
// This source file is part of the Aethelwerka Component Library.

module;  // Global Module Fragment:

#include <algorithm>
#include <cmath>
#include <limits>
#include <type_traits>

export module aethelwerka.foundation.math:procedures;

namespace aethelwerka::internal
{
	// todo (Krayfaus): Move all feature tests to aethelwerka.foundation.platform;
#if defined(__cpp_lib_bit_cast)
	constexpr auto compiler_has_bit_cast = __cpp_lib_bit_cast >= 201806L;
#else
	constexpr auto compiler_has_bit_cast = false;
#endif

	// SFINAE helper that converts a floating-point type to an integral type
	// of the same size.
	template <typename>
	struct to_integral;

	template <>
	struct to_integral<std::float_t>
	{
		using signed_t   = std::int32_t;
		using unsigned_t = std::uint32_t;
	};

	template <>
	struct to_integral<std::double_t>
	{
		using signed_t   = std::int64_t;
		using unsigned_t = std::uint64_t;
	};

	// Thought (Krayfaus):
	//  
	// This is one of the disadvantages of using standard types in a bleeding edge codebase.
	// We want 'to_integral' to support all floating-point types, but there's no:
	// - std::uint128_t
	// - std::long_double_t (aka long double, __float80, __float128)
	// 
	// Some implementations support more types and functions through builtins.
	// But as there's no sane way to add new definitions to the 'std' namespace, we reach an impasse.
	// 
	// That's why most game developers implement their own "standard library" and use it as foundation.
	// We need to know what is efficient and safe to use in the standard and when to avoid it.

}  // namespace aethelwerka::internal

export namespace aethelwerka
{
	// Helper to check if a integral type is a power of two.
	//
	// Basis: two's complement, the underlying representation of integral types in binary.
	template <typename T>
		requires(std::is_integral<T>::value)
	constexpr inline bool is_power_of_2(T value)
	{
		return (value & (value - 1)) == 0;
	}

	// Helper to check if a floating-point type is a power of two.
	//
	// Basis: IEEE 754 technical standard.
	template <typename T>
		requires(std::is_floating_point<T>::value)
	constexpr inline bool is_power_of_2(T float_value)
	{
		using unsigned_t = typename internal::to_integral<T>::unsigned_t;
		static_assert(sizeof(unsigned_t) == sizeof(T), "Unsigned integral type is not the same size as its floating-point counterpart.");

		constexpr auto mantissa = std::numeric_limits<T>::digits - 1;
		constexpr auto bits     = static_cast<unsigned_t>(float_value);
		if ((bits & ((unsigned_t{1} << mantissa) - 1)) != 0)
		{
			return is_power_of_2(bits);
		}

		auto const power = bits >> mantissa;

		return power > 0 and power < 255;
	}

	// Helper to determine the exponent bias of a floating_point type.
	//
	// Basis: IEEE 754 technical standard, section 2.1.5: biased exponent.
	template <typename T>
		requires(std::numeric_limits<T>::is_iec559)
	struct iec559_exponent_bias : std::integral_constant<typename internal::to_integral<T>::signed_t, std::numeric_limits<T>::max_exponent - std::numeric_limits<T>::radix>
	{
	};

	// Decomposes given floating_point value into a normalized fraction
	// and an exponent (integral power of two).
	//
	// Parameters:
	// - [in] value: a floating-point value.
	//
	// Returns: an unnamed struct containing the decomposed exponent and fraction.
	template <typename T>
		requires(std::numeric_limits<T>::is_iec559)
	auto frexp(T float_value) noexcept
	{
		// todo @Krayfaus: implement support for __float80 and __float128;
		static_assert(std::is_same<T, float>::value or std::is_same<T, double>::value, "Only floats and doubles are supported.");

		using signed_t   = typename internal::to_integral<T>::signed_t;
		using unsigned_t = typename internal::to_integral<T>::unsigned_t;

		static_assert(sizeof(signed_t) == sizeof(T), "Signed integral type is not the same size as its floating-point counterpart.");
		static_assert(sizeof(unsigned_t) == sizeof(T), "Unsigned integral type is not the same size as its floating-point counterpart.");

		constexpr auto mantissa_bits = std::numeric_limits<T>::digits - 1;
		constexpr auto exponent_bias = iec559_exponent_bias<T>::value;
		constexpr auto exponent_mask = std::is_same<T, float>::value ? unsigned_t{0xff} : unsigned_t{0x7ff0};

		// Ensure well defined bit transformation from float to integer.
		unsigned_t unsigned_value;
		if constexpr (internal::compiler_has_bit_cast)
		{
			unsigned_value = std::bit_cast<unsigned_t>(float_value);
		}
		else
		{
			std::memcpy(&unsigned_value, &float_value, sizeof(T));
		}

		// Extract the bits of the exponent (it has an offset of 'exponent_bias').
		signed_t const exponent = ((unsigned_value >> mantissa_bits) & exponent_mask) - exponent_bias;

		// Divide by 2^exponent (leaving mantissa intact while placing "0" into the exponent).
		unsigned_value &= ~(exponent_mask << mantissa_bits);  // Zero out the exponent bits.

		// Place 'exponent_bias' into exponent bits (representing 0).
		unsigned_value |= exponent_bias << mantissa_bits;

		// Copy fraction result back to float_value (reusing temporary rvalue storage).
		if constexpr (internal::compiler_has_bit_cast)
		{
			float_value = std::bit_cast<T>(unsigned_value);
		}
		else
		{
			std::memcpy(&float_value, &unsigned_value, sizeof(T));
		}

		// We use an unnamed struct so we can return mutiple names fields.
		struct
		{
			T        fraction;
			signed_t exponent;
		} result{float_value, exponent};

		return result;
	}

}  // namespace aethelwerka
