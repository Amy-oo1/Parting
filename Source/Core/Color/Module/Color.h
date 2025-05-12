#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"

PARTING_MODULE(Color)

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;

#else
#pragma once

#include "Core/ModuleBuild.h"

#include "Core/Utility/Include/UtilityMacros.h"
//Global

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"

#endif // PARTING_MODULE_BUILD#pragma once

class Color final {
	CONSTEXPR_TRIVIAL_FUNCTION(Color)
public:
	constexpr Color(float c) noexcept : R{ c }, G{ c }, B{ c }, A{ 1.0f } {}
	constexpr Color(float r, float g, float b, float a = 1.0f)noexcept : R{ r }, G{ g }, B{ b }, A{ a } {}

public:
	STDNODISCARD constexpr decltype(auto) Get_R(void)const noexcept { return this->R; }
	STDNODISCARD constexpr decltype(auto) Get_G(void)const noexcept { return this->G; }
	STDNODISCARD constexpr decltype(auto) Get_B(void)const noexcept { return this->B; }
	STDNODISCARD constexpr decltype(auto) Get_A(void)const noexcept { return this->A; }

public:
	STDNODISCARD constexpr bool operator==(const Color&) const noexcept = default;
	STDNODISCARD constexpr bool operator!=(const Color&) const noexcept = default;

public:
	float R{}, G{}, B{}, A{};
};