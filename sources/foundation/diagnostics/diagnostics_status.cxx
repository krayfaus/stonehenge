// SPDX-License-Identifier: GPL-2.0-only
// Copyright 2022 Ítalo "Krayfaus" Cadeu. All rights reserved.
// This source file is part of the Aethelwerka Component Library.

module;  // Module Global Fragment:

#include <expected.hpp>
#include <memory>
#include <string_view>
#include <type_traits>

export module aethelwerka.foundation.diagnostics:status;

export namespace aethelwerka
{

	enum class StatusCode : std::uint32_t
	{
		// Not an error; returned on success
		success = 0,

		// The operation was cancelled, typically by the caller.
		cancelled = 1,

		// Unknown error.
		unknown = 2,

		// Invalid argument specified.
		invalid_argument = 3,

		// Deadline expired before the operation could be complete.
		deadline_exceeded = 4,

		// Some requested entity (e.g., file or directory) was not found.
		not_found = 5,

		// The entity that a client attempted to create (e.g., file or directory) already exists.
		already_exists = 6,

		// The caller does not have permission to execute the specified operation.
		permission_denied = 7,

		// Some resource has been exhausted, perhaps a per-user quota, or perhaps the entire file system is out of space.
		resource_exhausted = 8,

		// The operation was rejected because the system is not in a state required for the operation's execution.
		failed_precondition = 9,

		// The operation was aborted, typically due to a concurrency issue such as a sequencer check failure or transaction abort.
		aborted = 10,

		// The operation was attempted past the valid range (e.g., seeking or reading past end-of-file).
		out_of_range = 11,

		// The operation is not implemented or is not supported/enabled in this service.
		unimplemented = 12,

		// Internal errors.
		internal = 13,

		// The service is currently unavailable.
		unavailable = 14,

		// Unrecoverable data loss or corruption.
		data_loss = 15,

		// The request does not have valid authentication credentials for the operation.
		unauthenticated = 16,
	};

	constexpr auto status_code_to_string(StatusCode status_code) -> std::string
	{
		switch (status_code)
		{
			case StatusCode::success:
			{
				return "Success";
			}
			case StatusCode::cancelled:
			{
				return "Cancelled";
			}
			case StatusCode::unknown:
			{
				return "Unknown";
			}
			case StatusCode::invalid_argument:
			{
				return "Invalid Argument";
			}
			case StatusCode::deadline_exceeded:
			{
				return "Deadline Exceeded";
			}
			case StatusCode::not_found:
			{
				return "Not Found";
			}
			case StatusCode::already_exists:
			{
				return "Already Exists";
			}
			case StatusCode::permission_denied:
			{
				return "Permission Denied";
			}
			case StatusCode::resource_exhausted:
			{
				return "Resource Exhausted";
			}
			case StatusCode::failed_precondition:
			{
				return "Failed Precondition";
			}
			case StatusCode::aborted:
			{
				return "Aborted";
			}
			case StatusCode::out_of_range:
			{
				return "Out of Range";
			}
			case StatusCode::unimplemented:
			{
				return "Unimplemented";
			}
			case StatusCode::internal:
			{
				return "Internal";
			}
			case StatusCode::unavailable:
			{
				return "Unavailable";
			}
			case StatusCode::data_loss:
			{
				return "Data Loss";
			}
			case StatusCode::unauthenticated:
			{
				return "Unauthenticated";
			}
			default:
			{
				return "";
			}
		}
	}

	// Convert canonical code to a value known to this binary.
	constexpr auto map_to_status_code(typename std::underlying_type<StatusCode>::type value) -> StatusCode
	{
		auto const status_code = static_cast<StatusCode>(value);
		switch (status_code)
		{
			case StatusCode::success:
			case StatusCode::cancelled:
			case StatusCode::unknown:
			case StatusCode::invalid_argument:
			case StatusCode::deadline_exceeded:
			case StatusCode::not_found:
			case StatusCode::already_exists:
			case StatusCode::permission_denied:
			case StatusCode::resource_exhausted:
			case StatusCode::failed_precondition:
			case StatusCode::aborted:
			case StatusCode::out_of_range:
			case StatusCode::unimplemented:
			case StatusCode::internal:
			case StatusCode::unavailable:
			case StatusCode::data_loss:
			case StatusCode::unauthenticated:
			{
				return status_code;
			}
			default:
			{
				return StatusCode::unknown;
			}
		}
	}

	struct Status
	{
		constexpr Status(StatusCode status_code) noexcept
			: code{status_code}
			, message{}
		{
		}

		constexpr Status(StatusCode status_code, std::string_view status_message) noexcept
			: code(status_code)
			, message{status_message}
		{
		}

		static constexpr auto Success() -> Status
		{
			return Status(StatusCode::success);
		}

		// Whether the Status is a success or not.
		explicit constexpr operator bool() const
		{
			return success();
		}

		constexpr bool success() const
		{
			return this->code == StatusCode::success;
		}

		friend constexpr bool operator==(Status const &lhs, Status const &rhs)
		{
			return lhs.code == rhs.code;
		}

		constexpr auto to_string() const
		{
			return status_code_to_string(this->code);
		}

		StatusCode  code;
		std::string message;
	};

}  // namespace aethelwerka
