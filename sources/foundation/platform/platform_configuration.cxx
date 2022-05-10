// SPDX-License-Identifier: GPL-2.0-only
// Copyright 2022 Ítalo "Krayfaus" Cadeu. All rights reserved.
// This source file is part of the Aethelwerka Component Library.
module;

#include "platform_configuration.hxx"  // Using: AETHELWERKA_PLATFORM macros;

export module aethelwerka.foundation.platform:configuration;

export namespace aethelwerka
{

	constexpr char const *k_Host_Compiler           = AETHELWERKA_PLATFORM_COMPILER;
	constexpr char const *k_Target_Architecture     = AETHELWERKA_PLATFORM_ARCH;
	constexpr char const *k_Target_Operating_System = AETHELWERKA_PLATFORM_OS;

}  // namespace aethelwerka
