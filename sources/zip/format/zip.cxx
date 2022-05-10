// SPDX-License-Identifier: GPL-2.0-only
// Copyright 2022 Ítalo "Krayfaus" Cadeu. All rights reserved.
// This source file is part of the Aethelwerka Component Library.
module;

#include <bit>
#include <compare>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

#ifdef _MSC_VER
#	define PACKED_STRUCT(_struct_declaration_) \
		__pragma(pack(push, 1)) struct _struct_declaration_ __pragma(pack(pop))
#elif defined(__GNUC__)
#	define PACKED_STRUCT(_struct_declaration_) \
		struct _struct_declaration_ __attribute__((__packed__))
#endif

export module aethelwerka.service.zip;

import aethelwerka.foundation.serializer;
import aethelwerka.foundation.diagnostics;

namespace aethelwerka
{

	PACKED_STRUCT(LocalFileHeader {
		std::uint32_t signature;
		std::uint16_t version_needed;
		std::uint16_t flags;
		std::uint16_t compression_method;
		std::uint16_t last_mod_time;
		std::uint16_t last_mod_date;
		std::uint32_t crc32;
		std::uint32_t compressed_size;
		std::uint32_t uncompressed_size;
		std::uint16_t file_name_length;
		std::uint16_t extra_field_length;
	});

	static_assert(sizeof(LocalFileHeader) == 30, "LocalFileHeader is not packed.");

	static void print_local_file_header(LocalFileHeader const &file_header)
	{
		print("LocalFileHeader:\n");
		print("  signature: {0:#x}\n", file_header.signature);
		print("  version_needed: {0:#x}\n", file_header.version_needed);
		print("  flags: {0:#x}\n", file_header.flags);
		print("  compression_method: {0:#x}\n", file_header.compression_method);
		print("  last_mod_time: {0:#x}\n", file_header.last_mod_time);
		print("  last_mod_date: {0:#x}\n", file_header.last_mod_date);
		print("  crc32: {0:#x}\n", file_header.crc32);
		print("  compressed_size: {}\n", file_header.compressed_size);
		print("  uncompressed_size: {}\n", file_header.uncompressed_size);
		print("  file_name_length: {}\n", file_header.file_name_length);
		print("  extra_field_length: {}\n", file_header.extra_field_length);
		print("");
	}

	struct LocalFile
	{
		LocalFileHeader header;
		std::string     file_name;    // header.filename_length;
		std::string     extra_field;  // header.extra_field_length;
		std::string     data;         // header.compressed_size;
	};

	static void print_local_file(LocalFile const &local_file)
	{
		print_local_file_header(local_file.header);
		
		print("FileName: {}\n", local_file.file_name);

		if (local_file.header.extra_field_length > 0)
		{
			print("ExtraField: ");
			for (std::size_t i = 0u; i < local_file.extra_field.size(); ++i)
			{
				print("{:02X} ", local_file.extra_field[i]);
			}
			print("\n");
		}

		if (local_file.header.compressed_size > 0)
		{
			print("Data: ");
			for (std::size_t i = 0u; i < local_file.data.size(); ++i)
			{
				print("{:02X} ", local_file.data[i]);
			}
			print("\n");
		}
	}

	template <typename Serializer>
	struct ZipArchiveImplementation
	{
	  public:
		bool initialize(std::string_view filepath)
		{
			m_filename = std::filesystem::path(filepath).filename().string();
			print("File: {}\n\n", m_filename);

			if (!m_serializer.initialize(filepath))
			{
				print("Failed to read file.");
				return false;
			}

			return true;
		}

		auto filelist() -> std::vector<LocalFile>
		{
			auto result = std::vector<LocalFile>{};

			m_serializer.seek(0u);

			auto e_file_header = m_serializer.read<LocalFileHeader>(std::endian::native);
			if (!e_file_header) { return result; }

			auto &file_header = e_file_header.value();

			auto local_file   = LocalFile{};
			local_file.header = file_header;

			if (auto file_name = m_serializer.read<std::string>(file_header.file_name_length); file_name.has_value())
			{
				local_file.file_name = file_name.value();
			}
			if (auto extra_field = m_serializer.read<std::string>(file_header.extra_field_length); extra_field.has_value())
			{
				local_file.extra_field = extra_field.value();
			}
			if (auto data = m_serializer.read<std::string>(file_header.compressed_size); data.has_value())
			{
				local_file.data = data.value();
			}

			print_local_file(local_file);

			result.push_back(local_file);

			return result;
		}

	  private:
		std::string m_filename{};
		Serializer  m_serializer{};
	};

	using ZipArchive = ZipArchiveImplementation<FileStream>;

}  // namespace aethelwerka

export namespace aethelwerka
{

	int zip_extractor_main(int argc, char **argv)
	{
		if (argc < 2)
		{
			print("Usage: zip <filepath>\n");
			return 1;
		}

		char const *filepath = argv[1];

		auto archive = aethelwerka::ZipArchive{};

		if (!archive.initialize(filepath))
		{
			return 2;
		}

		auto files = archive.filelist();

		// Extract the data.
		FileStream fs;
		for (LocalFile const &file : files)
		{
			if (file.file_name.empty())
			{
				print("Zip entry doesn't have a filename.\n");
				return 3;
			}

			fs.initialize(file.file_name, true);

			fs.write(file.data);

			fs.quiesce();
		}

		return 0;
	}

}  // namespace aethelwerka