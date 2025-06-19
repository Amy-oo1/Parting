#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"

PARTING_MODULE(Algorithm)

PARTING_IMPORT std;

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;

#else
#pragma once

#include "Core/ModuleBuild.h"

#include "Core/Utility/Include/UtilityMacros.h"
//Global

#include<limits>
#include<cmath>
#include<algorithm>

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#endif // PARTING_MODULE_BUILD

namespace Math {

	constexpr float PI_F{ 3.141592654f };
	constexpr double PI_D{ 3.14159265358979323 };


	// Convenient float constants
	constexpr float Epsilon{ 1e-6f };		// A reasonable general-purpose epsilon
	constexpr float Infinity{ std::numeric_limits<float>::infinity() };
	constexpr float NaN{ std::numeric_limits<float>::quiet_NaN() };

	template <typename _Ty>constexpr decltype(auto) Abs(_Ty value) { return value < static_cast<_Ty>(0) ? -value : value; }


	inline bool Is_Near(float a, float b, float eps = Epsilon) { return (Math::Abs(b - a) < eps); }
	inline bool Is_Finite(float f) {
		union { Uint32 i; float f; } u;
		u.f = f;
		return ((u.i & 0x7f800000) != 0x7f800000);
	}



	template<typename Type> STDNODISCARD constexpr Type Floor(Type Value) { return static_cast<Type>(std::floor(Value)); }
	template<typename Type> STDNODISCARD constexpr Type Ceil(Type Value) { return static_cast<Type>(std::ceil(Value)); }
	template<typename Type> STDNODISCARD constexpr Type Round(Type Value) { return static_cast<Type>(std::round(Value)); }



	// Integer rounding to multiples
	inline Int32 RoundDown(Int32 i, Int32 multiple) { return (i / multiple) * multiple; }
	inline Int32 RoundUp(Int32 i, Int32 multiple) { return ((i + (multiple - 1)) / multiple) * multiple; }

	inline Int32 ModPositive(Int32 dividend, Int32 divisor) {
		Int32 result{ dividend % divisor };

		return result < 0 ? result + divisor : result;
	}
	inline float ModPositive(float dividend, float divisor) {
		float result = fmod(dividend, divisor);

		return result < 0 ? result + divisor : result;
	}

	// Integer division, with rounding up (assuming positive arguments)
	template<typename Type> constexpr Type DivCeil(Type dividend, Type divisor) { return (dividend + (divisor - static_cast<Type>(1))) / divisor; }

	// Base-2 exp and log
	inline float Exp2f(float x) { return exp(0.693147181f * x); }
	inline float Log2f(float x) { return 1.442695041f * log(x); }

	template<typename Type>
	inline Type Sqrt(Type a) { return sqrt(a); }

	template<typename Type>
	inline Type Pow(Type a, Type b) { return pow(a, b); }

	inline bool Is_Pow2(Int32 x) { return (x > 0) && ((x & (x - 1)) == 0); }

	PARTING_EXPORT STDNODISCARD constexpr Uint32 NextPowerOf2(Uint32 v) {
		if (v == 0) 
			return 1;
		--v;
		v |= v >> 1;
		v |= v >> 2;
		v |= v >> 4;
		v |= v >> 8;
		v |= v >> 16;

		return ++v;
	}

	PARTING_EXPORT template<typename Type> STDNODISCARD constexpr Type Align(Type size, Type alignment) { return (size + alignment - 1) & ~(alignment - 1); }

	inline float Degrees(float rad) { return rad * (180.f / PI_F); }
	inline float Radians(float deg) { return deg * (PI_F / 180.f); }
	inline double Degrees(double rad) { return rad * (180.0 / PI_D); }
	inline double Radians(double deg) { return deg * (PI_D / 180.0); }

	template<typename Type> inline Type Cos(Type red) { return cos(red); }
	template<typename Type> inline Type Sin(Type red) { return sin(red); }
	template<typename Type> inline Type Tan(Type red) { return tan(red); }
	template<typename Type> inline Type ACos(Type red) { return acos(red); }
	template<typename Type> inline Type ASin(Type red) { return asin(red); }
	template<typename Type> inline Type ATan(Type red) { return atan(red); }
	template<typename Type> inline Type ATan2(Type y, Type x) { return atan2(y, x); }




	PARTING_EXPORT template <class _Ty>	STDNODISCARD constexpr const  _Ty& Max(const _Ty& left, const _Ty& right) { return std::max(left, right); }
	PARTING_EXPORT template <class _Ty, class _Pr> STDNODISCARD constexpr const _Ty& Max(const _Ty& left, const _Ty& right, _Pr& pred) { return std::max(left, right, pred); }
	PARTING_EXPORT template <class _Ty>	STDNODISCARD constexpr _Ty Max(std::initializer_list<_Ty> list) { return std::max(list); }
	PARTING_EXPORT template <class _Ty, class _Pr> STDNODISCARD constexpr _Ty Max(std::initializer_list<_Ty> list, _Pr pr) { return std::max(list, pr); }

	PARTING_EXPORT template <class _Ty> STDNODISCARD constexpr const _Ty& Min(const _Ty& left, const _Ty& right) { return std::min(left, right); }
	PARTING_EXPORT template <class _Ty, class _Pr> STDNODISCARD constexpr const _Ty& Min(const _Ty& left, const _Ty& right, _Pr& pred) { return std::min(left, right, pred); }
	PARTING_EXPORT template <class _Ty> STDNODISCARD constexpr _Ty Min(std::initializer_list<_Ty> list) { return std::min(list); }
	PARTING_EXPORT template <class _Ty, class _Pr> STDNODISCARD constexpr _Ty Min(std::initializer_list<_Ty> list, _Pr pr) { return std::min(list, pr); }

	template <typename _Ty>	constexpr decltype(auto) Clamp(const _Ty& Value, const _Ty& Lower, const _Ty& Upper) { return Min(Max(Value, Lower), Upper); }//NOTE :Not Use std::clamp

	template<typename _Ty>	constexpr decltype(auto) Saturate(_Ty value) { return Math::Clamp(value, static_cast<_Ty>(0), static_cast<_Ty>(1)); }

	template <typename _Ty>	constexpr decltype(auto) Lerp(_Ty a, _Ty b, float u) { return a + (b - a) * u; }

	// Generic square
	template <typename _Ty>	constexpr decltype(auto) Square(_Ty a) { return a * a; }

}

PARTING_EXPORT template <class _FwdIt, class _Ty, class _Pr>
STDNODISCARD constexpr _FwdIt LowerBound(_FwdIt _First, _FwdIt _Last, const _Ty& _Val, _Pr&& _Pred) { return std::lower_bound(_First, _Last, _Val, std::forward<_Pr>(_Pred)); }
PARTING_EXPORT template<class _FwdIt, class _Ty>
STDNODISCARD constexpr _FwdIt LowerBound(_FwdIt _First, _FwdIt _Last, const _Ty& _Val) { return std::lower_bound(_First, _Last, _Val); }

PARTING_EXPORT template <class _FwdIt, class _Ty, class _Pr>
STDNODISCARD constexpr _FwdIt UpperBound(_FwdIt _First, _FwdIt _Last, const _Ty& _Val, _Pr&& _Pred) { return std::upper_bound(_First, _Last, _Val, std::forward<_Pr>(_Pred)); }
PARTING_EXPORT template<class _FwdIt, class _Ty>
STDNODISCARD constexpr _FwdIt UpperBound(_FwdIt _First, _FwdIt _Last, const _Ty& _Val) { return std::upper_bound(_First, _Last, _Val); }

PARTING_EXPORT template <class _FwdIt, class _Pr>
STDNODISCARD constexpr bool Is_Sorted(_FwdIt _First, _FwdIt _Last, _Pr _Pred) { return std::is_sorted(_First, _Last, _Pred); }

PARTING_EXPORT template <class _FwdIt>
STDNODISCARD constexpr bool Is_Sorted(_FwdIt _First, _FwdIt _Last) { return std::is_sorted(_First, _Last); }

PARTING_EXPORT template <class _InIt, class _Ty>
STDNODISCARD constexpr _InIt STDFind(_InIt _First, _InIt _Last, const _Ty& _Val) { return std::find(_First, _Last, _Val); }

template <class _RanIt, class _Pr>
void Sort(_RanIt _First, _RanIt _Last, _Pr&& _Pred) { std::sort(_First, _Last, std::forward<_Pr>(_Pred)); }

template <class _RanIt>
void Sort(_RanIt _First, _RanIt _Last) { std::sort(_First, _Last); }
