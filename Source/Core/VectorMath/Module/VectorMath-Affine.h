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

	/*template <typename type, uint32 n>
	struct affine;*/

	/*	template<typename Type,int ize,int a>
	struct Mat;*/

	/*template<typename Type, int size>
	struct vector;*/

	template <typename Type, Uint32 N>bool operator == (const Affine<Type, N>& a, const Affine<Type, N>& b) { return All(a.m_Linear == b.m_Linear) && All(a.m_Translation == b.m_Translation); }

	template <typename Type, Uint32 N>bool operator != (const Affine<Type, N>& a, const Affine<Type, N>& b) { return Any(a.m_Linear != b.m_Linear) || Any(a.m_Translation != b.m_Translation); }

	template <typename Type, Uint32 N>Affine<Type, N> operator * (const Affine<Type, N>& a, const Affine<Type, N>& b) {
		return Affine<Type, N>{
			a.m_Linear* b.m_Linear,
				a.m_Translation* b.m_Linear + b.m_Translation
		};
	}

	template <typename Type, Uint32 N>Affine<Type, N>& operator *= (const Affine<Type, N>& a, const Affine<Type, N>& b) { return a = a * b; }

	template <typename Type, Uint32 N>Affine<Type, N> Transpose(const Affine<Type, N>& a) {
		auto mTransposed{ Transpose(a.m_Linear) };
		return Affine<Type, N>{
			mTransposed,
				-a.m_Translation * mTransposed
		};
	}

	template <typename Type, int N>Affine<Type, N> Pow(const Affine<Type, N>& a, Uint32 b) {
		if (b <= 0)
			return Affine<Type, N>::Identity();
		if (b == 1)
			return a;
		auto oddpart{ Affine<Type, N>::Identity() }, evenpart{ a };
		while (b > 1) {
			if (b % 2 == 1)
				oddpart *= evenpart;

			evenpart *= evenpart;
			b /= 2;
		}
		return oddpart * evenpart;
	}

	template <typename Type, int N>Affine<Type, N> Inverse(const Affine<Type, N>& a) {
		auto mInverted{ Inverse(a.m_Linear) };
		return Affine<Type, N>{
			mInverted,
				-a.m_Translation * mInverted
		};
	}



	//NOTE : !!! this doesn't match the behavior of isnear() for vectors and matrices -
	// returns a single result rather than a componentwise result
	template <typename Type, Uint32 N>bool IsNear(const Affine<Type, N>& a, const Affine<Type, N>& b, Type epsilon = Epsilon) { return All(IsNear(a.m_Linear, b.m_Linear, epsilon)) && All(IsNear(a.m_Translation, b.m_Translation, epsilon)); }

	//NOTE : !!! this doesn't match the behavior of isfinite() for vectors and matrices -
	// returns a single result rather than a componentwise result
	template <typename Type, Uint32 N>bool IsFinite(const Affine<Type, N>& a) { return All(IsFinite(a.m_Linear)) && All(IsFinite(a.m_Translation)); }

	template <typename Type, Uint32 N>	Affine<Uint32, N> Round(const Affine<Type, N>& a) { return Affine<Uint32, N>{ Round(a.m_Linear), Round(a.m_Translation) }; }

	template <typename Type, Uint32 N>	Affine<Type, N> Translation(const Vec<Type, N>& a) { return Affine<Type, N> { Mat<Type, N, N>::Identity(), a }; }

	template <typename Type, Uint32 N>	Affine<Type, N> Scaling(Type a) { return Affine<Type, N> { Diagonal<Type, N>(a), Vec<Type, N>::Zero() }; }
	template <typename Type, Uint32 N>	Affine<Type, N> Scaling(const Affine<Type, N>& a) { return Affine<Type, N> { Diagonal(a), Vec<Type, N>::Zero() }; }

	template<typename Type>Affine<Type, 2> Rotation(Type radians) {
		auto sinTheta{ Sin(radians) };
		auto cosTheta{ Cos(radians) };
		return Affine<Type, 2>{
			cosTheta, sinTheta,
				-sinTheta, cosTheta,
				static_cast<Type>(0), static_cast<Type>(0)
		};
	}

	template<typename Type>Affine<Type, 3> Rotation(const Affine<Type, 3>& axis, Type radians) {
		// Note: assumes axis is normalized
		auto sinTheta{ Sin(radians) };
		auto cosTheta{ Cos(radians) };

		// Build matrix that does cross product by axis (on the right)
		Mat<Type, 3, 3> crossProductMat{
			static_cast<Type>(0), axis.Z, -axis.Y,
			-axis.Z,static_cast<Type>(0), axis.X,
			axis.Y, -axis.X, static_cast<Type>(0)
		};

		// Matrix form of Rodrigues' rotation formula
		Mat<Type, 3, 3> mat{ Diagonal<Type, 3>(cosTheta) + crossProductMat * sinTheta + OuterProduct(axis, axis) * static_cast<Type>(1) - cosTheta };

		return Affine<Type, 3>{ mat, Vec<Type, 3>::Zero() };
	}

	template<typename Type>Affine<Type, 3> Rotation(const Affine<Type, 3>& euler) {
		Type sinX{ Sin(euler.X) };
		Type cosX{ Cos(euler.X) };
		Type sinY{ Sin(euler.Y) };
		Type cosY{ Cos(euler.Y) };
		Type sinZ{ Sin(euler.Z) };
		Type cosZ{ Cos(euler.Z) };

		Mat<Type, 3, 3> matX{
			static_cast<Type>(1), static_cast<Type>(0), static_cast<Type>(0),
			static_cast<Type>(0), cosX, sinX,
			static_cast<Type>(0), -sinX, cosX
		};
		Mat<Type, 3, 3> matY{
			cosY, static_cast<Type>(0), -sinY,
			static_cast<Type>(0), static_cast<Type>(1), static_cast<Type>(0),
			sinY, static_cast<Type>(0), cosY
		};
		Mat<Type, 3, 3> matZ{
			cosZ, sinZ, static_cast<Type>(0),
			-sinZ, cosZ, static_cast<Type>(0),
			static_cast<Type>(0), static_cast<Type>(0), static_cast<Type>(1)
		};

		return Affine<Type, 3>{ matX* matY* matZ, Vec<Type, 3>::Zero() };
	}

	template<typename Type>Affine<Type, 3> YawPitchRoll(Type yaw, Type pitch, Type roll) {
		// adapted from glm

		Type tmp_sh{ Sin(yaw) };
		Type tmp_ch{ Cos(yaw) };
		Type tmp_sp{ Sin(pitch) };
		Type tmp_cp{ Cos(pitch) };
		Type tmp_sb{ Sin(roll) };
		Type tmp_cb{ Cos(roll) };

		Affine<Type, 3> result;
		result.m_Linear[0][0] = tmp_ch * tmp_cb + tmp_sh * tmp_sp * tmp_sb;
		result.m_Linear[0][1] = tmp_sb * tmp_cp;
		result.m_Linear[0][2] = -tmp_sh * tmp_cb + tmp_ch * tmp_sp * tmp_sb;
		result.m_Linear[0][3] = static_cast<Type>(0);
		result.m_Linear[1][0] = -tmp_ch * tmp_sb + tmp_sh * tmp_sp * tmp_cb;
		result.m_Linear[1][1] = tmp_cb * tmp_cp;
		result.m_Linear[1][2] = tmp_sb * tmp_sh + tmp_ch * tmp_sp * tmp_cb;
		result.m_Linear[1][3] = static_cast<Type>(0);
		result.m_Linear[2][0] = tmp_sh * tmp_cp;
		result.m_Linear[2][1] = -tmp_sp;
		result.m_Linear[2][2] = tmp_ch * tmp_cp;
		result.m_Linear[2][3] = static_cast<Type>(0);
		result.m_Translation = Vec<Type, 3>::Zero();
		return result;
	}


	template<typename Type>Affine<Type, 2> Lookat(const Vec<Type, 2>& look) {
		Vec<Type, 2> lookNormalized{ Normalize(look) };
		return Affine<Type, 2>::FromCols(lookNormalized, Orthogonal(lookNormalized), Vec<Type, 2>::Zero());
	}

	// lookatX: rotate so X axis faces 'look' and Z axis faces 'up', if specified.
	// lookatZ: rotate so -Z axis faces 'look' and Y axis faces 'up', if specified.

	template<typename Type>Affine<Type, 3> LookatX(const Vec<Type, 3>& look) {
		Vec<Type, 3> lookNormalized{ Normalize(look) };
		Vec<Type, 3> left{ Normalize(orthogonal(lookNormalized)) };
		Vec<Type, 3> up{ Cross(lookNormalized, left) };
		return Affine<Type, 3>::FromCols(lookNormalized, left, up, Vec<Type, 3>::Zero());
	}
	template<typename Type>Affine<Type, 3> LookatX(const Vec<Type, 3>& look, const Vec<Type, 3>& up) {
		Vec<Type, 3> lookNormalized{ Normalize(look) };
		Vec<Type, 3> left{ Normalize(Cross(up,lookNormalized)) };
		Vec<Type, 3> trueUp{ Cross(lookNormalized, left) };
		return Affine<Type, 3>::FromCols(lookNormalized, left, trueUp, Vec<Type, 3>::Zero());
	}

	template<typename Type>Affine<Type, 3> LookatZ(const Vec<Type, 3>& look) {
		Vec<Type, 3> lookNormalized{ Normalize(look) };
		Vec<Type, 3> left{ Normalize(orthogonal(lookNormalized)) };
		Vec<Type, 3> up{ Cross(lookNormalized, left) };
		return Affine<Type, 3>::FromCols(-left, up, -lookNormalized, Vec<Type, 3>::Zero());
	}
	template<typename Type>Affine<Type, 3> LookatZ(const Vec<Type, 3>& look, const Vec<Type, 3>& up) {
		Vec<Type, 3> lookNormalized{ Normalize(look) };
		Vec<Type, 3> left{ Normalize(Cross(up,lookNormalized)) };
		Vec<Type, 3> trueUp{ Cross(lookNormalized, left) };
		return Affine<Type, 3>::FromCols(-left, trueUp, -lookNormalized, Vec<Type, 3>::Zero());
	}


	template <typename Type, Uint32 N>
	Mat<Type, N + 1, N + 1> AffineToHomogeneous(const Affine<Type, N>& a) {
		Mat<Type, N + 1, N + 1> result;
		for (Uint32 RowIndex = 0; RowIndex < N; ++RowIndex) {
			for (Uint32 ColIndex = 0; ColIndex < N; ++ColIndex)
				result[RowIndex][ColIndex] = a.m_Linear[RowIndex][ColIndex];
			result[RowIndex][N] = static_cast<Type>(0);
		}
		for (Uint32 ColIndex = 0; ColIndex < N; ++ColIndex)
			result[N][ColIndex] = a.m_Translation[ColIndex];
		result[N][N] = static_cast<Type>(1);

		return result;
	}

	template <typename Type, Uint32 N>
	Affine<Type, N - 1> HomogeneousToAffine(const Mat<Type, N, N>& a) {
		// Extract the relevant components of the matrix; note, NO checking
		// that the matrix actually represents an affine transform!
		Affine<Type, N - 1> result;
		for (Uint32 RowIndex = 0; RowIndex < N - 1; ++RowIndex)
			for (Uint32 ColIndex = 0; ColIndex < N - 1; ++ColIndex)
				result.m_Linear[RowIndex][ColIndex] = a[RowIndex][ColIndex];
		for (Uint32 ColIndex = 0; ColIndex < N - 1; ++ColIndex)
			result.m_Translation[ColIndex] = a[N - 1][ColIndex];

		return result;
	}

}