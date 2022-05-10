// SPDX-License-Identifier: GPL-2.0-only
// Copyright 2022 Ítalo "Krayfaus" Cadeu. All rights reserved.
// This source file is part of the Aethelwerka Component Library.
module;

#include "../platform/platform_configuration.hxx"

#if defined(AETHELWERKA_PLATFORM_COMPILER_MSVC)
#include <compare>
// If you import a class that uses the three-way comparison (spaceship) operator,
// you need to include the <compare> header.
// 
// https://developercommunity.visualstudio.com/t/Template-exports-requiring-importing-of-/1425979
#endif

export module aethelwerka.foundation.serializer;

//------------------------------------------------------------------------------
// Reexports:
//------------------------------------------------------------------------------
export import :abstracts;
export import :strategies;
