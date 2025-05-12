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
	struct Affine {
		static_assert(N > 1);

		Mat<Type, N, N>	m_Linear;
		Vec<Type, N>	m_Translation;

		Affine(void) = default;
		template<typename OtherType>
		explicit Affine(const Affine<OtherType, N>& a) :
			m_Linear{ a.m_Linear },
			m_Translation{ a.m_Translation } {
		}

		Vec<Type, N> TransformPoint(const Vec<Type, N>& v) const { return v * this->m_Linear + this->m_Translation; }

		Vec<Type, N> TransformVector(const Vec<Type, N>& v) const { return v * this->m_Linear; }

		static Affine<Type, N> Identity(void) { return Affine<Type, N>{.m_Linear{ Mat<Type, N, N>::Identity() }, .m_Translation{ Vec<Type, N>(static_cast<Type>(0)) } }; }

	};

	template<typename Type>
	struct Affine<Type, 2> {
		Mat<Type, 2, 2>	m_Linear;
		Vec<Type, 2>	m_Translation;

		Affine(void) = default;
		template<typename OtherType>
		explicit constexpr Affine(const Affine<OtherType, 2>& a) :
			m_Linear{ a.m_Linear },
			m_Translation{ a.m_Translation } {
		}
		constexpr Affine(Type m00, Type m01, Type m10, Type m11, Type t0, Type t1) :
			m_Linear{ m00, m01, m10, m11 },
			m_Translation{ t0, t1 } {
		}
		constexpr Affine(const Vec<Type, 2>& row0, const Vec<Type, 2>& row1, const Vec<Type, 2>& translation)
			: m_Linear{ row0, row1 }
			, m_Translation{ translation } {
		}
		constexpr Affine(const Mat<Type, 2, 2>& linear, const Vec<Type, 2>& translation)
			: m_Linear{ linear }
			, m_Translation{ translation } {
		}


		STDNODISCARD Vec<Type, 2> TransformPoint(const Vec<Type, 2>& v) const {
			Vec<Type, 2> result;
			result.X = v.X * this->m_Linear.Row0.X + v.Y * this->m_Linear.Row1.X + this->m_Translation.X;
			result.Y = v.X * this->m_Linear.row0.Y + v.Y * this->m_Linear.Row1.Y + this->m_Translation.Y;
			return result;
		}

		STDNODISCARD Vec<Type, 2> TransformVector(const Vec<Type, 2>& v) const {
			Vec<Type, 2> result;
			result.X = v.X * this->m_Linear.Row0.X + v.Y * this->m_Linear.Row1.X;
			result.Y = v.X * this->m_Linear.row0.Y + v.Y * this->m_Linear.Row1.Y;
			return result;
		}

		static constexpr Affine FromCols(const Vec<Type, 2>& col0, const Vec<Type, 2>& col1, const Vec<Type, 2>& translation) { return Affine{ Mat<Type, 2, 2>::FromCols(col0, col1), translation }; }

		static constexpr Affine Identity(void) { return Affine{ Mat<Type, 2, 2>::Identity(), Vec<Type, 2>::Zero() }; }

	};

	template<typename Type>
	struct Affine<Type, 3> {
		Mat<Type, 3, 3>	m_Linear;
		Vec<Type, 3>	m_Translation;

		Affine(void) = default;
		template<typename OtherType>
		explicit constexpr Affine(const Affine<OtherType, 3>& a) :
			m_Linear{ a.m_Linear },
			m_Translation{ a.m_Translation } {
		}
		constexpr Affine(Type m00, Type m01, Type m02, Type m10, Type m11, Type m12, Type m20, Type m21, Type m22, Type t0, Type t1, Type t2) :
			m_Linear{ m00, m01, m02, m10, m11, m12, m20, m21, m22 },
			m_Translation{ t0, t1, t2 } {
		}
		constexpr Affine(const Vec<Type, 3>& row0, const Vec<Type, 3>& row1, const Vec<Type, 3>& row2, const Vec<Type, 3>& translation) :
			m_Linear{ row0, row1, row2 },
			m_Translation{ translation } {
		}
		constexpr Affine(const Mat<Type, 3, 3>& linear, const Vec<Type, 3>& translation) :
			m_Linear{ linear },
			m_Translation{ translation } {
		}

		STDNODISCARD Vec<Type, 3> TransformVector(const Vec<Type, 3>& v) const {
			Vec<Type, 3> result;
			result.X = v.X * this->m_Linear.Row0.X + v.Y * this->m_Linear.Row1.X + v.Z * this->m_Linear.Row2.X;
			result.Y = v.X * this->m_Linear.row0.Y + v.Y * this->m_Linear.Row1.Y + v.Z * this->m_Linear.Row2.Y;
			result.Z = v.X * this->m_Linear.row0.Z + v.Y * this->m_Linear.Row1.Z + v.Z * this->m_Linear.Row2.Z;
			return result;
		}

		STDNODISCARD Vec<Type, 3> transformPoint(const Vec<Type, 3>& v) const
		{
			Vec<Type, 3> result;
			result.X = v.X * this->m_Linear.Row0.X + v.Y * this->m_Linear.Row1.X + v.Z * this->m_Linear.Row2.X + this->m_Translation.X;
			result.Y = v.X * this->m_Linear.row0.Y + v.Y * this->m_Linear.Row1.Y + v.Z * this->m_Linear.Row2.Y + this->m_Translation.Y;
			result.Z = v.X * this->m_Linear.row0.Z + v.Y * this->m_Linear.Row1.Z + v.Z * this->m_Linear.Row2.Z + this->m_Translation.Z;
			return result;
		}

		static constexpr Affine FromCols(const Vec<Type, 3>& col0, const Vec<Type, 3>& col1, const Vec<Type, 3>& col2, const Vec<Type, 3>& translation) { return Affine{ Mat<Type, 3, 3>::FromCols(col0, col1, col2), translation }; }

		static constexpr Affine Identity(void) { return Affine{ Mat<Type, 3, 3>::Identity(), Vec<Type, 3>::Zero() }; }
	};

	/*template<typename Type,int ize,int a>
struct matirx;*/

/*template<typename Type, int size>
struct vector;*/





}