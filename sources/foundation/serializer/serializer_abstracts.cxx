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

	template <typename T>
	inline void endian_swap_visitor(std::endian data_endian, T *data)
	{
		auto field_visitor_lambda = [data_endian](auto &variable)
		{
			using field_type              = decltype(variable);
			using unreferenced_field_type = std::remove_reference<field_type>::type;

			if (std::is_class<unreferenced_field_type>::value and std::is_standard_layout<unreferenced_field_type>::value and std::is_trivial<unreferenced_field_type>::value)
			{
				print("[EndianSwapper] Sub-struct found!\n");
				endian_swap_visitor(data_endian, &variable);
			}

			if (std::is_bounded_array<unreferenced_field_type>::value)
			{
				print("[EndianSwapper] Field Type: Bounded Array\n");

				using array_value_type = std::remove_all_extents<unreferenced_field_type>::type;
				if (sizeof(array_value_type) > 1)
				{
					auto *array_pointer = reinterpret_cast<array_value_type *>(&variable);
					endian_swap<array_value_type>(data_endian, array_pointer, std::extent<unreferenced_field_type>::value);
				}
				else
				{
					print("[EndianSwapper] Skiped because sizeof(value_type) == 1\n");
				}
			}
			else if (std::is_arithmetic<unreferenced_field_type>::value)
			{
				endian_swap(data_endian, &variable);
			}
			else if (std::is_enum<unreferenced_field_type>::value)
			{
				endian_swap<unreferenced_field_type>(data_endian, &variable);
			}
			else
			{
				aethelwerka::print("[EndianSwapper] Unknown or Unsupported type.\n");
				// Todo (Krayfaus):
				// - Add support for other common types.
				// - Handle errors gracefully.
			}
		};

		pfr::for_each_field(*data, field_visitor_lambda);
	}

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
				return make_unexpected(status);
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
					return status;
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
		auto read(size_t lenght, std::endian data_endian = std::endian::native) -> tl::expected<StringType, Status>
		{
			expect(this->validate(), k_stream_state_invalid);

			using string_type = StringType;
			using char_type   = StringType::value_type;

			auto result = string_type{};
			result.reserve(lenght);

			auto buffer = static_cast<char *>(result.data());
			if (auto status = this->on_read(buffer, sizeof(char_type) * lenght); !status.success())
			{
				return status;
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
				return make_unexpected(status);
			}

			// Visit every field in the struct and swap the endianness if needed.
			endian_swap_visitor(data_endian, &data);

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
				return make_unexpected(status);
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

		auto write_string(std::string text)
		{
			this->on_write(text.data(), text.size() + 1);
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
