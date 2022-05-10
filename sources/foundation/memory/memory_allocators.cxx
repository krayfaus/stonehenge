// SPDX-License-Identifier: GPL-2.0-only
// Copyright 2022 Ítalo "Krayfaus" Cadeu. All rights reserved.
// This source file is part of the Aethelwerka Component Library.

module;  // Global Module Fragment:

#include <compare>
#include <cstdlib>
#include <vector>
#include <algorithm>

// This partition exports: memory allocation strategies.
export module aethelwerka.foundation.memory:allocators;

import aethelwerka.foundation.diagnostics; // Using: {expect, ensure};
import :abstracts;                         // Using: {Block, Region, Allocator};
import :procedures;                        // Using: {address_of};

namespace aethelwerka::memory
{
	struct AllocatorStatistics
	{
		std::size_t block_count_current;      // Current allocated memory blocks.
		std::size_t block_count_total;        // Total   allocated memory blocks.
		std::size_t bytes_allocated_current;  // Current allocated memory in bytes.
		std::size_t bytes_allocated_total;    // Total   allocated memory in bytes.
		std::size_t bytes_deallocated_total;  // Total   deallocated memory in bytes.
	};

	static void print_allocator_statistics(AllocatorStatistics const &stats)
	{
		::printf("Allocator statistics:\n");
		::printf("  Current allocated blocks: %llu\n", stats.block_count_current);
		::printf("  Total allocated blocks:   %llu\n", stats.block_count_total);
		::printf("  Current allocated bytes:  %llu\n", stats.bytes_allocated_current);
		::printf("  Total allocated bytes:    %llu\n", stats.bytes_allocated_total);
		::printf("  Total deallocated bytes:  %llu\n", stats.bytes_deallocated_total);
	}

	struct DummyAllocatorSpecialization
	{
		auto on_allocate(std::size_t, std::size_t) -> Block
		{
			return Block::dummy();
		}

		auto on_reallocate(Block &block, std::size_t, std::size_t) -> Block
		{
			return Block::dummy();
		}

		auto on_deallocate(Block &block) -> bool
		{
			return this->on_contains(block) ? true : false;
		}

		auto on_contains(Block const &block) const -> bool
		{
			return !block ? true : false;
		}
	};

	struct MallocatorSpecialization
	{
		void on_print_statistics() const
		{
			print_allocator_statistics(this->stats);
		}
		
		auto on_allocate(std::size_t size, std::size_t alignment) -> Block
		{
#if _WIN32
			std::byte *pointer = static_cast<std::byte *>(::_aligned_malloc(size, alignment));
#else
			std::byte *pointer = static_cast<std::byte *>(::aligned_alloc(alignment, size));
#endif
			if (pointer == nullptr)
			{
				print("Failed to allocate memory block.\n");
				return Block();
			}

			print("Allocated memory block of size {} at address {}.\n", size, memory::address_of(pointer));
			{
				stats.bytes_allocated_total += size;
				stats.bytes_allocated_current += size;
				++stats.block_count_current;
				++stats.block_count_total;
			}

			auto block = Block(pointer, size);
			blocks.push_back(block);

			return block;
		}

		auto on_deallocate(Block &block) -> bool
		{
#if _WIN32
			::_aligned_free(block.pointer);
#else
			::free(block.pointer);
#endif
			print("Deallocated memory block of size {} at address {}.\n", block.size, block.address);
			{
				stats.bytes_allocated_current -= block.size;
				stats.bytes_deallocated_total += block.size;
				--stats.block_count_current;
			}

			blocks.erase(std::remove(std::begin(blocks), std::end(blocks), block), blocks.end());

			block.pointer = nullptr;
			block.size    = 0u;

			return true;
		}

		auto on_contains(Block const &block) const -> bool
		{
			return std::find(std::begin(blocks), std::end(blocks), block) != blocks.end();
		}

	  private:
		AllocatorStatistics stats{0};

		// This is a proof of concept, we need to remove the
		// extraneous allocations caused by 'std::vector'.
		std::vector<Block> blocks{};
	};


	struct Linear_AllocationStrategy
	{
	  private:
		struct ArenaData
		{
			std::byte     *buffer          = nullptr;
			std::size_t buffer_lenght   = 0;
			std::size_t previous_offset = 0;
			std::size_t current_offset  = 0;
		} arena;

	  public:
		auto on_allocate(std::size_t size, std::size_t alignment) -> Block
		{
			// Align 'current_offset' forward to the specified alignment.
			auto const buffer_address  = reinterpret_cast<std::uintptr_t>(arena.buffer);
			auto const current_address = buffer_address + arena.current_offset;
			auto const aligned_address = align_forward(current_address, alignment);
			auto const relative_offset = aligned_address - buffer_address;

			if (relative_offset + size > arena.buffer_lenght)
			{
				print("Failed to allocate memory block.\n");
				return Block();
			}

			std::byte *pointer    = &arena.buffer[relative_offset];
			arena.previous_offset = relative_offset;
			arena.current_offset  = relative_offset + size;

			std::memset(pointer, 0, size);

			return Block(pointer, size);
		}
	};

}  // namespace aethelwerka::memory


//------------------------------------------------------------------------------
// Exports:
//------------------------------------------------------------------------------
export namespace aethelwerka::memory
{

	// Dummy allocator. An allocator which does nothing.
	using DummyAllocator = Allocator<DummyAllocatorSpecialization>;

	// Mallocator. A wrapper around the standard C library memory utilities.
	using Mallocator = Allocator<MallocatorSpecialization>;

	// Todo (Krayfaus):
	// - Arena, Pool, System Pages;
	
	// Arena allocator. An allocator which allocates fixed-size memory blocks.
	// using Arena = Allocator<Linear_AllocationStrategy>;

}  // namespace aethelwerka::memory
