#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

#include "Core/Utility/Include/UtilityMacros.h"
PARTING_GLOBAL_MODULE

PARTING_SUBMODULE(VectorMath, Vec)

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;

#else
#pragma once

//#include "Core/ModuleBuild.h"

#include "Core/Utility/Include/UtilityMacros.h"
//Global
#include<cmath>

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"

#include "Core/VectorMath/Module/VectorMath-Misc.h"
#include "Core/VectorMath/Module/VectorMath-Vec.h"
#include "Core/VectorMath/Module/VectorMath-Mat.h"

#endif // PARTING_MODULE_BUILD

namespace Math {

	template <typename Type, Uint32 N>
	struct Box final {
		static_assert(N > 1);
		static constexpr int NumCorners{ 1 << N };

		Vec<Type, N> m_Mins{ std::numeric_limits<Type>::max() };
		Vec<Type, N> m_Maxs{ std::numeric_limits<Type>::lowest() };

		Box(void) = default;
		constexpr Box(const Vec<Type, N>& mins, const Vec<Type, N>& maxs) : m_Mins(mins), m_Maxs(maxs) {}
		Box(Uint32 numPoints, const Vec<Type, N>* points) {
			if (numPoints == 0)
				return;

			this->m_Mins = points[0];
			this->m_Maxs = points[0];

			for (Uint32 Index = 1; Index < numPoints; ++Index) {
				this->m_Mins = Min(this->m_Mins, points[Index]);
				this->m_Maxs = Max(this->m_Maxs, points[Index]);
			}
		}
		template<typename OtherType>constexpr Box(const Box<OtherType, N>& b) : m_Mins(b.m_Mins), m_Maxs(b.m_Maxs) {}

		constexpr bool Is_Empty(void) const { return Any(this->m_Mins > this->m_Maxs); }

		constexpr bool Contains(const Vec<Type, N>& a) const { return All(this->m_Mins <= a) && All(a <= this->m_Maxs); }
		constexpr bool Contains(const Box<Type, N>& a) const { return a.Is_Empty() || (All(this->m_Mins <= a.m_Mins) && all(a.m_Maxs <= this->m_Maxs)); }

		constexpr bool Intersects(const Box<Type, N>& a) const { return All(a.m_Mins <= this->m_Maxs) && All(this->m_Mins <= a.m_Maxs); }

		bool Is_Finite(void) const { return All(Is_Finite(this->m_Mins)) && All(Is_Finite(this->m_Maxs)); }

		constexpr Vec<Type, N> Clamp(const Vec<Type, N>& a) const { return Clamp(a, this->m_Mins, this->m_Maxs); }

		constexpr Vec<Type, N> Get_Center(void) const { return this->m_Mins + (this->m_Maxs - this->m_Mins) / static_cast<Type>(2); }
		constexpr Vec<Type, N> Get_Corner(Uint32 iCorner) const { return Select(BitVec<N>(iCorner), this->m_Maxs, this->m_Mins); }
		void Get_Corners(Vec<Type, N>* cornersOut) const {
			for (Uint32 Index = 0, nc = Box<Type, N>::NumCorners; Index < nc; ++Index)
				cornersOut[Index] = this->Get_Corner(Index);
		}

		constexpr Vec<Type, N> Diagonal(void) const { return this->m_Maxs - this->m_Mins; }

		void Get_ExtentsAlongAxis(const Vec<Type, N>& a, Type& outMin, Type& outMax) const {
			Type dotCenter{ Dot(this->Get_Center(), a) };
			Type dotDiagonal{ Dot(this->Diagonal(), Abs(a)) };
			outMin = dotCenter - dotDiagonal;
			outMax = dotCenter + dotDiagonal;
		}

		Type DotMin(const Vec<Type, N>& a) const {
			Type dotMin, dotMax;
			this->Get_ExtentsAlongAxis(a, dotMin, dotMax);
			return dotMin;
		}

		Type DotMax(const Vec<Type, N>& a) const {
			Type dotMin, dotMax;
			this->Get_ExtentsAlongAxis(a, dotMin, dotMax);
			return dotMax;
		}

		static constexpr Box Empty(void) { return Box{}; }

		constexpr Box Translate(const Vec<Type, N>& v) const { return Box{ this->m_Mins + v, this->m_Maxs + v }; }

		constexpr Box Grow(const Vec<Type, N>& v) const { return Box{ this->m_Mins - v, this->m_Maxs + v }; }

		constexpr Box Grow(Type v) const { return Box{ this->m_Mins - v, this->m_Maxs + v }; }

		constexpr Box Round(void) const { return Box{ Round(this->m_Mins), Round(this->m_Maxs) }; }

		constexpr Box operator & (const Box& other) const { return Box<Type, N>{ Max(this->m_Mins, other.m_Mins), Min(this->m_Maxs, other.m_Maxs) }; }

		Box operator &= (const Box& other) { *this = *this & other;	return *this; }

		constexpr Box operator | (const Box& other) const { return Box<Type, N>{ Min(this->m_Mins, other.m_Mins), Max(this->m_Maxs, other.m_Maxs) }; }

		Box operator |= (const Box& other) { *this = *this | other;	return *this; }

		constexpr Box operator | (const Vec<Type, N>& v) const { return Box<Type, N>{ Min(this->m_Mins, v), Max(this->m_Maxs, v) }; }

		Box operator |= (const Vec<Type, N>& v) { *this = *this | v; return *this; }

		Box operator * (const Affine<Type, N>& transform) const {
			// fast method to apply an Affine transform to an AABB
			Box<Type, N> result;
			result.m_Mins = transform.m_Translation;
			result.m_Maxs = transform.m_Translation;

			const Vec<Type, N>* row{ &transform.m_Linear.Row0 };
			for (Uint32 Index = 0; Index < N; ++Index) {
				Vec<Type, N> e{ (&this->m_Mins.X)[Index] * (*row) };
				Vec<Type, N> f{ (&this->m_Maxs.X)[Index] * (*row) };
				result.m_Mins += Min(e, f);
				result.m_Maxs += Max(e, f);
				++row;
			}

			return result;
		}

		Box operator *= (const Affine<Type, N>& transform) const { *this = *this * transform; return *this; }

		constexpr bool operator == (Box<Type, N> const& other) const { return All(this->m_Mins == other.m_Mins) && All(this->m_Maxs == other.m_Maxs); }

		constexpr bool operator != (Box<Type, N> const& other) const { return Any(this->m_Mins != other.m_Mins) || Any(this->m_Maxs != other.m_Maxs); }
	};

	template <typename Type, Uint32 N>Type Distance(const Box<Type, N>& a, const Vec<Type, N>& b) { return Distance(a.Clamp(b), b); }
	template <typename Type, Uint32 N>Type Distance(const Vec<Type, N>& a, const Box<Type, N>& b) { return Distance(a, b.Clamp(a)); }

	template <typename Type, Uint32 N>Type DistanceSquared(const Box<Type, N>& a, const Vec<Type, N>& b) { return DistanceSquared(a.Clamp(b), b); }
	template <typename Type, Uint32 N>Type DistanceSquared(const Vec<Type, N>& a, const Box<Type, N>& b) { return DistanceSquared(a, b.Clamp(a)); }

	// !!! this doesn't match the behavior of isnear() for vectors and matrices -
	// returns a single result rather than a componentwise result
	template <typename Type, Uint32 N>bool Is_Near(const Box<Type, N>& a, const Box<Type, N>& b, float epsilon = Epsilon) {
		return
			All(Is_Near(a.m_Mins, b.m_Mins, epsilon)) &&
			All(Is_Near(a.m_Maxs, b.m_Maxs, epsilon));
	}

	// !!! this doesn't match the behavior of isfinite() for vectors and matrices -
	// returns a single result rather than a componentwise result
	template <typename Type, Uint32 N>bool Is_Finite(const Box<Type, N>& a) { return a.Is_Finite(); }

}