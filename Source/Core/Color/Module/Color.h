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

#include "Core/VectorMath/Module/VectorMath.h"

#endif // PARTING_MODULE_BUILD#pragma once

struct Color final {
	CONSTEXPR_TRIVIAL_FUNCTION(Color)
public:
	constexpr Color(float c) noexcept : R{ c }, G{ c }, B{ c }, A{ c } {}
	constexpr Color(float r, float g, float b, float a = 1.0f)noexcept : R{ r }, G{ g }, B{ b }, A{ a } {}

	constexpr Color(const Math::VecF3& vec)noexcept : R{ vec.X }, G{ vec.Y }, B{ vec.Z }, A{ 0.f } {}
	constexpr Color(const Math::VecF4& vec)noexcept : R{ vec.X }, G{ vec.Y }, B{ vec.Z }, A{ vec.W } {}

	operator Math::VecF3(void) { return Math::VecF3{ this->R, this->G, this->B }; }
	operator const Math::VecF3(void)const { return Math::VecF3{ this->R, this->G, this->B }; }
	operator Math::VecF4(void) { return Math::VecF4{ this->R, this->G, this->B, this->A }; }
	operator const Math::VecF4(void)const { return Math::VecF4{ this->R, this->G, this->B, this->A }; }

public:
public:
	STDNODISCARD constexpr bool operator==(const Color&) const noexcept = default;
	STDNODISCARD constexpr bool operator!=(const Color&) const noexcept = default;


public:
	STDNODISCARD float Get_Luminance(void)const { return Math::Dot(Math::VecF3{ this->R,this->G,this->B }, Color::LumaCoefficients()); }

public:
	static Math::VecF3 Black(void) { return Math::VecF3{ 0.f }; }
	static Math::VecF3 White(void) { return Math::VecF3{ 1.f }; }
	static Math::VecF3 Red(void) { return Math::VecF3{ 1.f, 0.f, 0.f }; }
	static Math::VecF3 Green(void) { return Math::VecF3{ 0.f, 1.f, 0.f }; }
	static Math::VecF3 Blue(void) { return Math::VecF3{ 0.f, 0.f, 1.f }; }

	static Math::VecF3 LumaCoefficients(void) { return Math::VecF3{ 0.2126f, 0.7152f, 0.0722f }; }// Rec. 709 luma coefficients for linear float3 space

	static Color OverPremul(const Color& a, const Color& b) {
		return Color{
			a.R + b.R * (1.f - a.A),
			a.G + b.G * (1.f - a.A),
			a.B + b.B * (1.f - a.A),
			1.f - ((1.f * a.A) * (1.f - b.A))
		};
	}// Composition operator for linear RGB space (premultiplied alpha)

	static Color OverNonpremul(const Color& a, const Color& b) {
		return Math::VecF4{
			Math::Lerp(static_cast<const Math::VecF3>(b),static_cast<const Math::VecF3>(a),a.A),
			1.f - ((1.f * a.A) * (1.f - b.A))
		};
	}

public:
	float R{ 0.f }, G{ 0.f }, B{ 0.f }, A{ 0.f };//TODO :use a nonamestruct include rgba and, a nuname union include it and a vecf4
};