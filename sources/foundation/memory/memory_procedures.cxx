// SPDX-License-Identifier: GPL-2.0-only
// Copyright 2022 �talo "Krayfaus" Cadeu. All rights reserved.
// This source file is part of the Aethelwerka Component Library.

module;  // Global Module Fragment:

#include <cstddef>
#include <memory>
#include <string>

export module aethelwerka.foundation.memory:procedures;

import aethelwerka.foundation.diagnostics; // Using: {expect, ensure};
import aethelwerka.foundation.math;        // Using: {is_power_of_2};

//------------------------------------------------------------------------------

namespace aethelwerka::internal
{
#if defined(__GNUC__)
	[[noreturn]] inline __attribute__((always_inline)) void unreachable()
	{
		__builtin_unreachable();
	}
#elif defined(_MSC_VER)
	[[noreturn]] __forceinline void unreachable()
	{
		__assume(false);
	}
#else
	inline void unreachable()
	{
	}
#endif
}  // namespace aethelwerka::internal

//------------------------------------------------------------------------------

export namespace aethelwerka::memory
{
	constexpr void fill(void *data, std::byte value, std::size_t count)
	{
		std::byte *pointer = static_cast<std::byte *>(data);

		for (; count > 0; --count, ++pointer)
		{
			*pointer = value;
		}
	}

	constexpr auto compare(void const *lhs, void const *rhs, std::size_t count)
	{
		if (std::is_constant_evaluated())
		{
			return __builtin_memcmp(lhs, rhs, count);
		}
		else
		{
			if (lhs == nullptr || rhs == nullptr)
			{
				return 0;
			}
			else if (lhs == nullptr)
			{
				return -1;
			}
			else if (rhs == nullptr)
			{
				return 1;
			}

			return std::memcmp(lhs, rhs, count);
		}
	}

	constexpr auto reverse(std::byte **buffer, std::size_t count)
	{
		std::byte *begin = *buffer;
		std::byte *end   = *buffer + count;

		std::reverse(begin, end);
	}

	// Constructs an instance of T in `p` through placement new,
	// passing Args... to the constructor.
	template <typename T, typename... Args>
	void construct_at(void *storage, Args &&...args)
	{
		if (storage == nullptr)
		{
			internal::unreachable();
		}

		::new (storage) T{std::forward<Args>(args)...};
	}

	// Wrapper around std::memcpy.
	inline void copy(void *__restrict destination, void *const __restrict source, size_t size)
	{
		std::memcpy(destination, source, size);
	}

	// Rounds up to the given power of two.
	template <typename T>
		requires(std::is_unsigned<T>::value)
	inline auto round_up(T value, T round_up_to) -> T
	{
		expect(is_power_of_2(round_up_to), "Expected a power of two value for rounding.");
		return (value + (round_up_to - 1)) & ~(round_up_to - 1);
	}

	// Aqcuires the address of the given reference value.
	template <typename T>
	inline auto address_of(T &object) -> std::uintptr_t
	{
		return reinterpret_cast<std::uintptr_t>(std::addressof(object));
	}

	// Aqcuires the address of the given pointer value.
	template <typename T>
	inline auto address_of(T *object) -> std::uintptr_t
	{
		return reinterpret_cast<std::uintptr_t>(object);
	}

	auto align_forward(std::uintptr_t pointer, std::size_t alignment) -> std::uintptr_t
	{
		expect(is_power_of_2(static_cast<std::uint64_t>(alignment)), "Expected a power of two value for alignment.");
		auto const pointer_alignment = static_cast<std::ptrdiff_t>(alignment);

		// Calculate 'pointer % alignment', using bitwise operators for performance.
		auto const modulo = pointer & (pointer_alignment - 1);

		if (modulo == 0)
		{
			return pointer;
		}

		// Pushes the pointer to the next aligned address.
		return pointer + pointer_alignment - modulo;
	}

}  // namespace aethelwerka::memory
