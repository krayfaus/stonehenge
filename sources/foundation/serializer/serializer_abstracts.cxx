// SPDX-License-Identifier: GPL-2.0-only
// Copyright 2022 Ítalo "Krayfaus" Cadeu. All rights reserved.
// This source file is part of the Aethelwerka Component Library.
module;

//-----------------------------------------------------------------------------
// Includes:
//-----------------------------------------------------------------------------
#include <expected.hpp>
#include <pfr.hpp>      // Using: {for_each_field};
#include <type_traits>  // Using: {is_trivial};

//-----------------------------------------------------------------------------
// Module Declaration:
//-----------------------------------------------------------------------------
export module aethelwerka.foundation.serializer:abstracts;

//-----------------------------------------------------------------------------
// Imports:
//-----------------------------------------------------------------------------
#pragma warning(disable : 5050)
import aethelwerka.foundation.diagnostics; // Using: {expect, ensure, status, print};
import aethelwerka.foundation.platform;    // Using: {endian_swap};

//-----------------------------------------------------------------------------
// Internals:
//-----------------------------------------------------------------------------
namespace aethelwerka
{
	static constexpr auto k_stream_state_invalid = "Stream is not in a valid state.";
}  // namespace aethelwerka

//-----------------------------------------------------------------------------
// Exports:
//-----------------------------------------------------------------------------
export namespace aethelwerka
{

	template <typename StreamHandler>
	struct Serializer final : public StreamHandler
	{
		//-----------------------------------------------------------------------------
		// Serializer :: Common
		//-----------------------------------------------------------------------------

		bool initialize(auto &&...args)
		{
			return this->on_initialize(std::forward<decltype(args)>(args)...);
		}

		bool quiesce()
		{
			return this->on_quiesce();
		}

		bool validate()
		{
			return this->on_validate();
		}

		auto size() -> size_t
		{
			return this->on_size();
		}

		bool seek(size_t position)
		{
			return this->on_seek(position);
		}

		auto position()
		{
			return this->on_position();
		}

		//-----------------------------------------------------------------------------
		// Serializer :: Read
		//-----------------------------------------------------------------------------

		//! Reads a trivial type from the stream.
		template <typename T = char>
			requires(std::is_trivial<T>::value and not std::is_class<T>::value)
		auto read(std::endian data_endian = std::endian::native) -> tl::expected<T, Status>
		{
			expect(this->validate(), k_stream_state_invalid);

			auto       data    = T{};
			auto       pointer = reinterpret_cast<char *>(&data);
			auto const lenght  = sizeof(T);

			if (auto status = this->on_read(pointer, lenght); !status.success())
			{
				return tl::make_unexpected(status);
			}

			endian_swap(data_endian, &data);
			return data;
		}

		//! Reads a string type from the stream.
		//* Supports any null-terminated string based on std::basic_string.
		template <typename StringType>
			requires(std::is_base_of<std::basic_string<char>, StringType>::value)
		auto read(std::endian data_endian = std::endian::native) -> tl::expected<StringType, Status>
		{
			expect(this->validate(), k_stream_state_invalid);

			using string_type = StringType;
			using char_type   = StringType::value_type;

			auto result    = string_type{};
			auto character = char_type{};
			auto status    = Status{};

			auto buffer = static_cast<char *>(&character);
			do
			{
				status = this->on_read(buffer, sizeof(char_type));

				if (!status.success())
				{
					return tl::make_unexpected(status);
				}

				if constexpr (sizeof(char_type) > sizeof(char))
				{
					endian_swap(data_endian, &character);
				}

				result += character;
			} while (character != '\0');

			return result;
		}

		template <typename StringType>
			requires(std::is_base_of<std::basic_string<char>, StringType>::value)
		auto read(std::size_t lenght, std::endian data_endian = std::endian::native) -> tl::expected<StringType, Status>
		{
			expect(this->validate(), k_stream_state_invalid);

			using string_type = StringType;
			using char_type   = StringType::value_type;

			auto result = string_type{};
			result.resize(lenght);

			char *buffer = static_cast<char *>(&result[0]);
			if (auto status = this->on_read(buffer, sizeof(char_type) * lenght); !status.success())
			{
				return tl::make_unexpected(status);
			}

			return result;
		}

		//! Reads a plain-old-data struct type from the stream.
		template <typename T>
			requires(std::is_class<T>::value and std::is_standard_layout<T>::value and std::is_trivial<T>::value)
		auto read(std::endian data_endian = std::endian::native) -> tl::expected<T, Status>
		{
			expect(this->validate(), k_stream_state_invalid);

			auto       data    = T{};
			auto       pointer = reinterpret_cast<char *>(&data);
			auto const lenght  = sizeof(T);

			if (auto status = this->on_read(pointer, lenght); !status.success())
			{
				return tl::make_unexpected(status);
			}

			return data;
		}

		// Peeks a trivial type from the stream.
		template <typename T>
			requires(std::is_trivial<T>::value)
		auto peek(std::endian data_endian = std::endian::native) -> tl::expected<T, Status>
		{
			expect(this->validate(), k_stream_state_invalid);

			auto       data    = T{};
			auto       pointer = reinterpret_cast<char *>(&data);
			auto const lenght  = sizeof(T);

			if (auto status = this->on_peek(pointer, lenght); !status.success())
			{
				return tl::make_unexpected(status);
			}

			endian_swap(data_endian, &data);
			return data;
		}

		//-----------------------------------------------------------------------------
		// Serializer :: Write
		//-----------------------------------------------------------------------------

		// Writes a trivial type to the stream.
		template <typename T>
			requires(std::is_trivial<T>::value)
		auto write(T data, std::endian data_endian = std::endian::native) -> Status
		{
			expect(this->validate(), k_stream_state_invalid);

			endian_swap(data_endian, &data);

			auto const pointer = reinterpret_cast<char *>(&data);
			auto const lenght  = sizeof(T);
			if (auto status = this->on_write(pointer, lenght); !status.success())
			{
				return status;
			}

			return Status::Success();
		}

		auto write(std::string text)
		{
			this->on_write(text.data(), text.size());
		}
	};

	// ------------------------------------------------------------------

	template <typename StreamHandler>
	struct is_stream_serializer : std::false_type
	{
	};

	template <typename StreamHandler>
	struct is_stream_serializer<Serializer<StreamHandler>> : std::true_type
	{
	};

}  // namespace aethelwerka
