// SPDX-License-Identifier: GPL-2.0-only
// Copyright 2022 Ítalo "Krayfaus" Cadeu. All rights reserved.
// This source file is part of the Aethelwerka Component Library.
module;

#include <pfr.hpp>

//-----------------------------------------------------------------------------
// Includes:
//-----------------------------------------------------------------------------
#include <fstream>          // Using: {fstream};
#include <ios>              // Using: {in, out, binary, streampos, streamsize};
#include <limits>           // Using: {numeric_limits};
#include <source_location>  // Using: {source_location};
#include <string_view>      // Using: {string_view};
#include <type_traits>      // Using: {is_trivial};

//-----------------------------------------------------------------------------
// Module Declaration:
//-----------------------------------------------------------------------------
export module aethelwerka.foundation.serializer:strategies;

//-----------------------------------------------------------------------------
// Imports:
//-----------------------------------------------------------------------------
#pragma warning(disable : 5050)
import :abstracts;
import aethelwerka.foundation.diagnostics; // Using: {expect, ensure, status, print};
import aethelwerka.foundation.platform;    // Using: {endian_swap};
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Handle Pre-condition errors, by returning a status.
//-----------------------------------------------------------------------------
#define EXPECTED_PRECONDITION(condition, message)                               \
	if (false == condition)                                                     \
	{                                                                           \
		return ::aethelwerka::Status(StatusCode::failed_precondition, message); \
	}

#define EXPECTED_CONDITION(condition, status, message) \
	if (false == condition)                            \
	{                                                  \
		return ::aethelwerka::Status{status, message}; \
	}
//-----------------------------------------------------------------------------

namespace aethelwerka
{

	struct DummyStreamImplementation
	{
	  protected:
		DummyStreamImplementation() = default;

		auto on_read([[maybe_unused]] char *buffer, [[maybe_unused]] size_t count) -> Status
		{
			return Status::Success();
		}
	};

	struct FileStreamImplementation
	{
	  protected:
		virtual ~FileStreamImplementation() = default;

		bool on_initialize(std::string_view file_path, bool overwrite = false)
		{
			if (overwrite)
			{
				m_file.open(file_path.data(), std::ios::in | std::ios::out | std::ios::trunc | std::ios::binary);
			}
			else
			{
				m_file.open(file_path.data(), std::ios::in | std::ios::out | std::ios::binary);
			}

			if (!m_file.is_open())
			{
				// Todo (Krayfaus): Handle errors.
				//
				// diagnostics::log(Status::failed_to_aqcuire_resource("Failed to open file."));
				//
				return false;
			}

			return true;
		}

		bool on_quiesce()
		{
			m_file.close();

			return true;
		}

		auto on_validate()
		{
			return !m_file.fail();
		}

		auto on_size()
		{
			auto old_pos = m_file.tellp();
			m_file.seekp(0x0);
			m_file.ignore(std::numeric_limits<std::streamsize>::max());

			auto lenght = m_file.gcount();
			m_file.clear();
			m_file.seekp(old_pos);

			return lenght;
		}

		auto on_position()
		{
			return m_file.tellp();
		}

		bool on_seek(size_t position)
		{
			m_file.seekp(position);

			return true;
		}

		auto on_read(char *buffer, size_t count) -> Status
		{
			EXPECTED_PRECONDITION(count > 0, "Cannot read less than 1 byte.");

			m_file.read(buffer, count);

			EXPECTED_CONDITION(!m_file.eof(), StatusCode::out_of_range, "EOF reached.");
			EXPECTED_CONDITION(!m_file.fail(), StatusCode::aborted, "Unknow error, safe abort.");

			return Status::Success();
		}

		auto on_peek(char *buffer, size_t count) -> Status
		{
			EXPECTED_PRECONDITION(count > 0, "Cannot peek less than 1 byte.");

			auto old_pos = m_file.tellp();
			m_file.read(buffer, count);
			m_file.clear();
			m_file.seekp(old_pos);

			EXPECTED_CONDITION(!m_file.eof(), StatusCode::out_of_range, "EOF reached.");
			EXPECTED_CONDITION(!m_file.fail(), StatusCode::aborted, "Unknow error, safe abort.");

			return Status::Success();
		}

		auto on_write(char *buffer, size_t count) -> Status
		{
			EXPECTED_PRECONDITION(count > 0, "Cannot write less than 1 byte.");

			m_file.write(buffer, count);

			EXPECTED_CONDITION(!m_file.eof(), StatusCode::out_of_range, "EOF reached.");
			EXPECTED_CONDITION(!m_file.fail(), StatusCode::aborted, "Unknow error, safe abort.");

			return Status::Success();
		}

	  private:
		std::fstream m_file;
	};

	export
	{
		using DummyStream = Serializer<DummyStreamImplementation>;
		using FileStream  = Serializer<FileStreamImplementation>;
	}

}  // namespace aethelwerka
