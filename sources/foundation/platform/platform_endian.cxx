// SPDX-License-Identifier: GPL-2.0-only
// Copyright 2022 Ítalo "Krayfaus" Cadeu. All rights reserved.
// This source file is part of the Aethelwerka Component Library.
module;

//-----------------------------------------------------------------------------
// Includes:
//-----------------------------------------------------------------------------
#include <bit>  // Using: {std::endian};

//-----------------------------------------------------------------------------
// Module Declaration:
//-----------------------------------------------------------------------------
export module aethelwerka.foundation.platform:endian;

//-----------------------------------------------------------------------------
// Imports:
//-----------------------------------------------------------------------------
import aethelwerka.foundation.memory; // Using: {aethelwerka::memory::reverse};

//-----------------------------------------------------------------------------
// Internals:
//-----------------------------------------------------------------------------
namespace aethelwerka::internal
{

	template <typename T>
	inline void on_endian_swap(T *data)
	{
		char *buffer = reinterpret_cast<char *>(data);
		memory::reverse(&buffer, sizeof(T));
	}

}  // namespace aethelwerka::internal

//-----------------------------------------------------------------------------
// Exports:
//-----------------------------------------------------------------------------
export namespace aethelwerka
{
	template <typename T>
	inline void endian_swap(std::endian data_endian, T *data, std::size_t data_lenght = 1)
	{
		if constexpr (sizeof(T) > sizeof(char))
		{
			if (std::endian::native == data_endian)
			{
				return;
			}

			for (size_t i = 0u; i < data_lenght; ++i)
			{
				internal::on_endian_swap<T>(&data[i]);
			}
		}
	}

}  // namespace aethelwerka
