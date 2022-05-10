// SPDX-License-Identifier: GPL-2.0-only
// Copyright 2022 Ítalo "Krayfaus" Cadeu. All rights reserved.
// This source file is part of the Aethelwerka Component Library.

module;  // Global Module Fragment:

#include <compare>

// This partition exports: memory abstraction utilities.
export module aethelwerka.foundation.memory:abstracts;

// Imports:
import aethelwerka.foundation.diagnostics; // Using: {expect, ensure};

// The idea behind the memory component is to experiment with interfaces to memory allocation strategies.
//
// We use CRTP and non-virtual interfaces to create efficient abstractions.
//
// There are some "ugly" uses of unions, but they are thought to be more efficient than casting every time.
//
namespace aethelwerka::memory
{

	// We assume the size of 'std::uintptr_t' and 'std::byte *' to be the same,
	// as it should if the compiler is compliant with the standard.
	static_assert(sizeof(std::uintptr_t) == sizeof(std::byte *));


	// Block
	//
	// > a contiguous sequence of bytes.
	//
	// Variables:
	//   [public] address / pointer - actual memory location.
	//   [public] size              - number of represented bytes.
	export struct [[nodiscard]] Block
	{

		union
		{
			std::uintptr_t address;
			std::byte     *pointer;
		};

		std::size_t size;

		//---------------------------------------------------------------------

		constexpr Block() noexcept
			: address(0)
			, size(0)
		{
		}

		constexpr Block(std::uintptr_t address, std::size_t size) noexcept
			: address(address)
			, size(size)
		{
		}

		constexpr Block(std::byte *pointer, std::size_t size) noexcept
			: pointer(pointer)
			, size(size)
		{
		}

		//---------------------------------------------------------------------

		// Creates a dummy (invalid) memory block.
		static constexpr auto dummy() -> Block
		{
			return Block{};
		}

		// Whether the Block is valid or not.
		explicit constexpr operator bool() const
		{
			return *this != dummy();
		}

		friend constexpr auto operator<=>(Block const &lhs, Block const &rhs)
		{
			return lhs.address <=> rhs.address;
		}

		friend constexpr bool operator==(Block const &lhs, Block const &rhs)
		{
			return lhs.address == rhs.address && lhs.size == rhs.size;
		}

		template <typename T>
		friend auto as_pointer(Block const &block) -> T *
		{
			// This is the correct way to use reinterpret_cast,
			// it's the job of the programmer to ensure that the cast is valid.
			return reinterpret_cast<T *>(block.pointer);
		}
	};


	// Region
	//
	// > a contiguous sequence of memory blocks.
	//
	// Variables:
	//   [public] start - address of the first element.
	//   [public] end   - address of the last element.
	//
	export struct [[nodiscard]] Region
	{
		union
		{
			std::uintptr_t first;
			std::byte     *begin;
		};

		union
		{
			std::uintptr_t last;
			std::byte     *end;
		};

		static constexpr auto dummy() -> Region
		{
			return {.first = 0u, .last = 0u};
		}

		explicit constexpr operator bool() const
		{
			return *this != dummy();
		}

		constexpr auto size() const -> std::size_t
		{
			// todo (krayfaus): Do we need to check for unsigned underflow?
			// expect(begin <= end, "Region .begin address is greater than .end address.");

			return last - first;
		}

		friend constexpr auto operator<=>(Region const &lhs, Region const &rhs)
		{
			return lhs.first <=> rhs.first;
		}

		friend constexpr bool operator==(Region const &lhs, Region const &rhs)
		{
			return lhs.first == rhs.first && lhs.last == rhs.last;
		}
	};


	namespace internal
	{

		// Requirements for an allocator specialization.
		//
		// Required methods: on_allocate, on_belongs, on_deallocate;
		//
		// Optional methods: on_reallocate;
		//
		// Note: Internal use only, details must not be exported.
		//
		// We need to disable clang-format bellow, because it doest not handle require blocks correctly.
		//
		template <typename T> concept
		has_foundation_methods = requires(T resource, Block block)
		{
			// clang-format off
			{ resource.on_allocate(0u, 0u) }  -> std::same_as<Block>;
			{ resource.on_deallocate(block) } -> std::same_as<bool>;
			{ resource.on_contains(block) }   -> std::same_as<bool>;
			// clang-format on
		};

		template <typename T> concept
		has_reallocate = requires(T resource, Block block)
		{
			// clang-format off
			{ resource.on_reallocate(block, 0, 0) } -> std::same_as<Block>;
			// clang-format on
		};

		// Thought (@krayfaus):
		//   We may need additional concepts for things like statistics, etc.

		//---------------------------------------------------------------------

		// Thought (Krayfaus): We need a better way to handle error strings.
		// We need to ensure we don't have duplicated strings in the final binary.
		static constexpr auto k_text_unrecognized_block = "Block doesn't belong to allocator.";

	}  // namespace internal


	// Allocator - Base class for memory allocators.
	//
	// > an interface to memory management.
	//
	// An allocator is a synonym for an allocation strategy.
	export template <typename Specialization>
		requires(internal::has_foundation_methods<Specialization>)
	struct Allocator final : private Specialization
	{
		// Allocates a memory block.
		//
		// Parameters:
		//   [in] size      - The number of bytes to allocate.
		//   [in] alignment - The alignment of the memory block.
		//
		// Returns: The allocated memory block.
		auto allocate(std::size_t size, std::size_t alignment = alignof(std::max_align_t)) -> Block
		{
			return this->on_allocate(size, alignment);
		}

		// Reallocates the given memory block.
		//
		// Parameters:
		//   [in, out] block - a reference to memory block.
		//   [in] size       - the number of bytes to allocate.
		//   [in] alignment  - the alignment of the memory block.
		//
		// Returns: the allocated memory block.
		template <typename = void>
			requires(internal::has_reallocate<Specialization>)
		auto reallocate(Block &block, std::size_t size, std::size_t alignment = alignof(std::max_align_t)) -> Block
		{
			expect(this->on_contains(block), internal::k_text_unrecognized_block);
			return this->on_reallocate(block, size, alignment);
		}

		// Deallocates a previously allocated memory block.
		//
		// Parameters:
		//   [in] block - a reference to the memory block.
		//
		// Returns:
		//   true  - the block was successfully deallocated;
		//   false - the block was not deallocated;
		bool deallocate(Block &block)
		{
			expect(this->on_contains(block), internal::k_text_unrecognized_block);
			return this->on_deallocate(block);
		}

		// Check if given memory block belongs to allocator.
		//
		// Parameters:
		//   [in] block - reference to memory block.
		//
		// Returns:
		//   true  - the block was created by the allocator;
		//   false - the block was not created by the allocator;
		//
		[[nodiscard]] bool contains(Block const &block) const
		{
			return this->on_contains(block);
		}
		// Thought (@krayfaus): Yeah, I know, it's not a good idea to overload operators, but it looked cool.
		[[nodiscard]] bool operator[](Block const &block) const
		{
			return this->on_contains(block);
		}

		void print_statistics() const
		{
			this->on_print_statistics();
		}

		// Todo (@krayfaus): Do we need a method to retrieve block size given owned address?
		// auto block_size(std::size_t address) const -> std::size_t;

		// Returns a singleton instance of the Allocator.
		[[nodiscard]] static auto &acquire()
		{
			static auto instance = decltype(this){};
			return instance;
		}
	};

}  // namespace aethelwerka::memory
