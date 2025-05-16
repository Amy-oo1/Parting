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

#endif // PARTING_MODULE_BUILD


namespace Math {

#define MATRIX_MEMBERS(Type, RowCount, ColCount) \
		using Array_T = Type[RowCount*ColCount]; \
		using ConstArray_T = const Type[RowCount*ColCount]; \
		Mat(void)= default; \
		Mat(const Type* ValuePtr) { for (Uint32 Index = 0; Index < RowCount*ColCount; ++Index) this->m_Data[Index] = ValuePtr[Index]; } \
		operator Array_T (void) { return this->m_Data; } \
		operator ConstArray_T (void) const { return this->m_Data; } \
		Vec<Type, ColCount> & operator [] (Uint32 RowIndex) { return reinterpret_cast<Vec<Type, ColCount> &>(this->m_Data[RowIndex*ColCount]); } \
		const Vec<Type, ColCount> & operator [] (Uint32 RowIndex) const { return reinterpret_cast<const Vec<Type, ColCount> &>(this->m_Data[RowIndex*ColCount]); } \
		Vec<Type, RowCount> Col(int ColIndex) const { Vec<Type, RowCount> Value; for (Uint32 RowIndex = 0; RowIndex < RowCount; RowIndex++) Value[RowIndex] = this->m_Data[RowIndex * ColCount + ColIndex]; return Value; }

#undef MATRIX_MEMBERS

#pragma warning(push)
#pragma warning(disable: 4201)	// Nameless struct/union
	template <typename Type, Uint32 RowCount, Uint32 ColCount>
	struct Mat {
		static_assert(RowCount > 1);
		static_assert(ColCount > 1);

		using Array_T = Type(&)[RowCount * ColCount];
		using ConstArray_T = const Type(&)[RowCount * ColCount];

		Type m_Data[RowCount * ColCount];

		Mat(void) = default;
		Mat(const Type* ValuePtr) { for (Uint32 Index = 0; Index < RowCount * ColCount; ++Index) this->m_Data[Index] = ValuePtr[Index]; }
		Mat(Type Scale) { for (Uint32 Index = 0; Index < RowCount * ColCount; ++Index) this->m_Data[Index] = Scale; }

		operator Array_T (void) { return this->m_Data; }
		operator ConstArray_T (void) const { return this->m_Data; }
		Vec<Type, ColCount>& operator [] (Uint32 RowIndex) { return reinterpret_cast<Vec<Type, ColCount>&>(this->m_Data[RowIndex * ColCount]); }
		const Vec<Type, ColCount>& operator [] (Uint32 RowIndex) const { return reinterpret_cast<const Vec<Type, ColCount>&>(this->m_Data[RowIndex * ColCount]); }
		Vec<Type, RowCount> Col(int ColIndex) const { Vec<Type, RowCount> Value; for (Uint32 RowIndex = 0; RowIndex < RowCount; RowIndex++) Value[RowIndex] = this->m_Data[RowIndex * ColCount + ColIndex]; return Value; }

		static Mat Diagonal(Vec<Type, (RowCount < ColCount) ? RowCount : ColCount> Value) {
			Mat result;
			for (Uint32 RowIndex = 0; RowIndex < RowCount; ++RowIndex)
				for (Uint32 ColIndex = 0; ColIndex < ColCount; ++ColIndex)
					result[RowIndex][ColIndex] = (RowIndex == ColIndex) ? Value[RowIndex] : Type{ 0 };
			return result;
		}

		static Mat Diagonal(Type Scale) {
			Mat result;
			for (Uint32 RowIndex = 0; RowIndex < RowCount; ++RowIndex)
				for (Uint32 ColIndex = 0; ColIndex < ColCount; ++ColIndex)
					result[RowIndex][ColIndex] = (RowIndex == ColIndex) ? Scale : Type{ 0 };
			return result;
		}

		static Mat Identity(void) { return Mat::Diagonal(Type{ 1 }); }

		static Mat Zero(void) { return Mat{ Type{ 0 } }; }
	};

	template <typename Type>
	struct Mat<Type, 2, 2> {
		using Array_T = Type(&)[2 * 2];
		using ConstArray_T = const Type(&)[2 * 2];

		union {
			Type m_Data[2 * 2];
			struct {
				Type
					M00, M01,
					M10, M11;
			};
			struct { Vec<Type, 2> Row0, Row1; };
		};

		Mat(void) = default;
		Mat(const Type* ValuePtr) { for (Uint32 Index = 0; Index < 2 * 2; ++Index) this->m_Data[Index] = ValuePtr[Index]; }
		constexpr Mat(Type Scale) :
			M00{ Scale }, M01{ Scale },
			M10{ Scale }, M11{ Scale }
		{
		}
		constexpr Mat(Type _m00, Type _m01, Type _m10, Type _m11) :
			M00{ _m00 }, M01{ _m01 },
			M10{ _m10 }, M11{ _m11 }
		{
		}
		constexpr Mat(const Vec<Type, 2>& _row0, const Vec<Type, 2>& _row1) :
			M00{ _row0.X }, M01{ _row0.Y },
			M10{ _row1.Y }, M11{ _row1.Y } {
		}
		template<typename OtherType>explicit constexpr Mat(const Mat<OtherType, 2, 2>& other) { for (Uint32 Index = 0; Index < 4; ++Index) this->m_Data[Index] = static_cast<Type>(other.m_Data[Index]); }

		operator Array_T (void) { return this->m_Data; }
		operator ConstArray_T (void) const { return this->m_Data; }
		Vec<Type, 2>& operator [] (Uint32 RowIndex) { return reinterpret_cast<Vec<Type, 2> &>(this->m_Data[RowIndex * 2]); }
		const Vec<Type, 2>& operator [] (Uint32 RowIndex) const { reinterpret_cast<const Vec<Type, 2>&>(this->m_Data[RowIndex * 2]); }

		Vec<Type, 2> Col(int ColIndex) const { Vec<Type, 2> Value; for (Uint32 RowIndex = 0; RowIndex < 2; RowIndex++) Value[RowIndex] = this->m_Data[RowIndex * 2 + ColIndex]; return Value; }

		constexpr static Mat FromCols(const Vec<Type, 2>& col0, const Vec<Type, 2>& col1) {
			return Mat{
				col0.X, col1.X,
				col0.Y, col1.Y
			};;
		}

		constexpr static Mat Diagonal(Type diag) {
			return Mat{
				diag, Type{ 0 },
				Type{ 0 }, diag
			};
		}

		constexpr static Mat Diagonal(Vec<Type, 2> Value) {
			return Mat{
				Value.X, Type{ 0 },
				Type{ 0 }, Value.Y };
		}

		constexpr static Mat Identity(void) { return Mat<Type, 3, 3>::Diagonal(Type{ 1 }); }

		constexpr static Mat Zero(void) { return Mat{ static_cast<Type>(0) }; }


	};

	template <typename Type>
	struct Mat<Type, 3, 3> {
		using Array_T = Type(&)[3 * 3];
		using ConstArray_T = const Type(&)[3 * 3];
		union {
			Type m_Data[3 * 3];
			struct {
				Type
					M00, M01, M02,
					M10, M11, M12,
					M20, M21, M22;
			};
			struct { Vec<Type, 3> Row0, Row1, Row2; };
		};

		Mat(void) = default;
		Mat(const Type* ValuePtr) { for (Uint32 Index = 0; Index < 3 * 3; ++Index) this->m_Data[Index] = ValuePtr[Index]; }
		constexpr Mat(Type Scale) :
			M00{ Scale }, M01{ Scale }, M02{ Scale },
			M10{ Scale }, M11{ Scale }, M12{ Scale },
			M20{ Scale }, M21{ Scale }, M22{ Scale } {
		}
		constexpr Mat(Type _m00, Type _m01, Type _m02, Type _m10, Type _m11, Type _m12, Type _m20, Type _m21, Type _m22) :
			M00{ _m00 }, M01{ _m01 }, M02{ _m02 },
			M10{ _m10 }, M11{ _m11 }, M12{ _m12 },
			M20{ _m20 }, M21{ _m21 }, M22{ _m22 } {
		}
		constexpr Mat(const Vec<Type, 3>& _row0, const Vec<Type, 3>& _row1, const Vec<Type, 3>& _row2) :
			M00{ _row0.X }, M01{ _row0.Y }, M02{ _row0.Z },
			M10{ _row1.X }, M11{ _row1.Y }, M12{ _row1.Z },
			M20{ _row2.X }, M21{ _row2.Y }, M22{ _row2.Z } {
		}
		constexpr Mat(const Mat<Type, 3, 4>& matrix) :
			M00{ matrix.M00 }, M01{ matrix.M01 }, M02{ matrix.M02 },
			M10{ matrix.M10 }, M11{ matrix.M11 }, M12{ matrix.M12 },
			M20{ matrix.M20 }, M21{ matrix.M21 }, M22{ matrix.M22 } {
		}
		constexpr Mat(const Mat<Type, 4, 4>& matrix) :
			M00{ matrix.M00 }, M01{ matrix.M01 }, M02{ matrix.M02 },
			M10{ matrix.M10 }, M11{ matrix.M11 }, M12{ matrix.M12 },
			M20{ matrix.M20 }, M21{ matrix.M21 }, M22{ matrix.M22 } {
		}
		template<typename OtherType>explicit constexpr Mat(const Mat<OtherType, 3, 3>& matrix) { for (Uint32 Index = 0; Index < 9; ++Index) this->m_Data[Index] = Type{ matrix.m_Data[Index] }; }

		operator Array_T (void) { return this->m_Data; }
		operator ConstArray_T (void) const { return this->m_Data; }

		Vec<Type, 3>& operator [] (Uint32 RowIndex) { return reinterpret_cast<Vec<Type, 3> &>(this->m_Data[RowIndex * 3]); }
		const Vec<Type, 3>& operator [] (Uint32 RowIndex) const { return reinterpret_cast<const Vec<Type, 3> &>(this->m_Data[RowIndex * 3]); }
		Vec<Type, 3> Col(int ColIndex) const { Vec<Type, 3> Value; for (Uint32 RowIndex = 0; RowIndex < 3; RowIndex++) Value[RowIndex] = this->m_Data[RowIndex * 3 + ColIndex]; return Value; }

		constexpr static Mat FromCols(const Vec<Type, 3>& col0, const Vec<Type, 3>& col1, const Vec<Type, 3>& col2) {
			return Mat{
				col0.X, col1.X, col2.X,
				col0.Y, col1.Y, col2.Y,
				col0.Z, col1.Z, col2.Z
			};
		}

		constexpr static Mat Diagonal(Type diag) {
			return Mat{
				diag, static_cast<Type>(0), static_cast<Type>(0),
				static_cast<Type>(0), diag, static_cast<Type>(0),
				static_cast<Type>(0), static_cast<Type>(0), diag
			};
		}

		constexpr static Mat Diagonal(Vec<Type, 3> Value) {
			return Mat{
				Value.X, static_cast<Type>(0), static_cast<Type>(0),
				static_cast<Type>(0), Value.Y, static_cast<Type>(0),
				static_cast<Type>(0), static_cast<Type>(0), Value.Z
			};
		}

		constexpr static Mat Identity(void) { return Mat<Type, 3, 3>::Diagonal(static_cast<Type>(1)); }

		constexpr static Mat Zero(void) { return Mat{ static_cast<Type>(0) }; }

	};

	template <typename Type>
	struct Mat<Type, 3, 4> {
		using Array_T = Type(&)[3 * 4];
		using ConstArray_T = const Type(&)[3 * 4];
		union {
			Type m_Data[3 * 4];
			struct {
				Type
					M00, M01, M02, M03,
					M10, M11, M12, M13,
					M20, M21, M22, M23;
			};
			struct { Vec<Type, 4> Row0, Row1, Row2; };
		};

		Mat(void) = default;
		Mat(const Type* ValuePtr) { for (Uint32 Index = 0; Index < 3 * 4; ++Index) this->m_Data[Index] = ValuePtr[Index]; }
		constexpr Mat(Type Scale) :
			M00{ Scale }, M01{ Scale }, M02{ Scale }, M03{ Scale },
			M10{ Scale }, M11{ Scale }, M12{ Scale }, M13{ Scale },
			M20{ Scale }, M21{ Scale }, M22{ Scale }, M23{ Scale }
		{
		}
		constexpr Mat(Type _m00, Type _m01, Type _m02, Type _m03, Type _m10, Type _m11, Type _m12, Type _m13, Type _m20, Type _m21, Type _m22, Type _m23) :
			M00{ _m00 }, M01{ _m01 }, M02{ _m02 }, M03{ _m03 },
			M10{ _m10 }, M11{ _m11 }, M12{ _m12 }, M13{ _m13 },
			M20{ _m20 }, M21{ _m21 }, M22{ _m22 }, M23{ _m23 }
		{
		}
		constexpr Mat(const Vec<Type, 4>& _row0, const Vec<Type, 4>& _row1, const Vec<Type, 4>& _row2) :
			M00{ _row0.X }, M01{ _row0.Y }, M02{ _row0.Z }, M03{ _row0.W },
			M10{ _row1.X }, M11{ _row1.Y }, M12{ _row1.Z }, M03{ _row0.W },
			M20{ _row2.X }, M21{ _row2.Y }, M22{ _row2.Z }, M23{ _row2.W }
		{
		}
		constexpr Mat(const Mat<Type, 3, 3>& matrix, const Vec<Type, 3>& col3) :
			M00{ matrix.M00 }, M01{ matrix.M01 }, M02{ matrix.M02 }, M03{ col3.X },
			M10{ matrix.M10 }, M11{ matrix.M11 }, M12{ matrix.M12 }, M13{ col3.Y },
			M20{ matrix.M20 }, M21{ matrix.M21 }, M22{ matrix.M22 }, M23{ col3.Z }
		{
		}
		constexpr Mat(const Mat<Type, 4, 4>& matrix) :
			M00{ matrix.M00 }, M01{ matrix.M01 }, M02{ matrix.M02 }, M03{ matrix.M03 },
			M10{ matrix.M10 }, M11{ matrix.M11 }, M12{ matrix.M12 }, M13{ matrix.M13 },
			M20{ matrix.M20 }, M21{ matrix.M21 }, M22{ matrix.M22 }, M23{ matrix.M23 }
		{
		}
		template<typename OtherType>explicit constexpr Mat(const Mat<OtherType, 3, 4>& matrix) { for (Uint32 Index = 0; Index < 12; ++Index) this->m_Data[Index] = Type{ matrix.m_Data[Index] }; }

		operator Array_T (void) { return this->m_Data; }
		operator ConstArray_T (void) const { return this->m_Data; }
		Vec<Type, 4>& operator [] (Uint32 RowIndex) { return reinterpret_cast<Vec<Type, 4> &>(this->m_Data[RowIndex * 4]); }
		const Vec<Type, 4>& operator [] (Uint32 RowIndex) const { return reinterpret_cast<const Vec<Type, 4> &>(this->m_Data[RowIndex * 4]); }

		Vec<Type, 3> Col(int ColIndex) const { Vec<Type, 3> Value; for (Uint32 RowIndex = 0; RowIndex < 3; RowIndex++) Value[RowIndex] = this->m_Data[RowIndex * 4 + ColIndex]; return Value; }

		constexpr static Mat FromCols(const Vec<Type, 3>& col0, const Vec<Type, 3>& col1, const Vec<Type, 3>& col2, const Vec<Type, 3>& col3) {
			return Mat{
				col0.X, col1.X, col2.X, col3.X,
				col0.Y, col1.Y, col2.Y, col3.Y,
				col0.Z, col1.Z, col2.Z, col3.Z
			};
		}

		constexpr static Mat Diagonal(Type diag) {
			return Mat{
				diag, static_cast<Type>(0), static_cast<Type>(0),static_cast<Type>(0),
				static_cast<Type>(0), diag, static_cast<Type>(0),static_cast<Type>(0),
				static_cast<Type>(0), static_cast<Type>(0), diag, static_cast<Type>(0)
			};
		}

		constexpr static Mat Diagonal(Vec<Type, 3> Value) {
			return Mat{
				Value.X, static_cast<Type>(0), static_cast<Type>(0),static_cast<Type>(0),
				static_cast<Type>(0), Value.Y, static_cast<Type>(0),static_cast<Type>(0),
				static_cast<Type>(0), static_cast<Type>(0), Value.Z, static_cast<Type>(0)
			};
		}

		constexpr static Mat Identity(void) { return this->Diagonal(static_cast<Type>(1)); }

		constexpr static Mat Zero(void) { return Mat{ static_cast<Type>(0) }; }
	};

	template <typename Type>
	struct Mat<Type, 4, 4> {
		using Array_T = Type(&)[4 * 4];
		using ConstArray_T = const Type(&)[4 * 4];
		union {
			Type m_Data[4 * 4];
			struct {
				Type
					M00, M01, M02, M03,
					M10, M11, M12, M13,
					M20, M21, M22, M23,
					M30, M31, M32, M33;
			};
			struct { Vec<Type, 4> Row0, Row1, Row2, Row3; };
		};

		Mat(void) = default;
		Mat(const Type* ValuePtr) { for (Uint32 Index = 0; Index < 4 * 4; ++Index) this->m_Data[Index] = ValuePtr[Index]; }
		constexpr Mat(Type Scale) :
			M00{ Scale }, M01{ Scale }, M02{ Scale }, M03{ Scale },
			M10{ Scale }, M11{ Scale }, M12{ Scale }, M13{ Scale },
			M20{ Scale }, M21{ Scale }, M22{ Scale }, M23{ Scale },
			M30{ Scale }, M31{ Scale }, M32{ Scale }, M33{ Scale }
		{
		}
		constexpr Mat(Type _m00, Type _m01, Type _m02, Type _m03, Type _m10, Type _m11, Type _m12, Type _m13, Type _m20, Type _m21, Type _m22, Type _m23, Type _m30, Type _m31, Type _m32, Type _m33) :
			M00{ _m00 }, M01{ _m01 }, M02{ _m02 }, M03{ _m03 },
			M10{ _m10 }, M11{ _m11 }, M12{ _m12 }, M13{ _m13 },
			M20{ _m20 }, M21{ _m21 }, M22{ _m22 }, M23{ _m23 },
			M30{ _m30 }, M31{ _m31 }, M32{ _m32 }, M33{ _m33 }
		{
		}
		constexpr Mat(const Vec<Type, 4>& _row0, const Vec<Type, 4>& _row1, const Vec<Type, 4>& _row2, const Vec<Type, 4>& _row3) :
			M00{ _row0.X }, M01{ _row0.Y }, M02{ _row0.Z }, M03{ _row0.W },
			M10{ _row1.X }, M11{ _row1.Y }, M12{ _row1.Z }, M03{ _row0.W },
			M20{ _row2.X }, M21{ _row2.Y }, M22{ _row2.Z }, M23{ _row2.W },
			M30{ _row3.X }, M31{ _row3.Y }, M32{ _row3.Z }, M33{ _row3.W }
		{
		}
		constexpr Mat(const Mat<Type, 3, 4>& matrix, const Vec<Type, 4>& _row3) :
			M00{ matrix.M00 }, M01{ matrix.M01 }, M02{ matrix.M02 },
			M10{ matrix.M10 }, M11{ matrix.M11 }, M12{ matrix.M12 },
			M20{ matrix.M20 }, M21{ matrix.M21 }, M22{ matrix.M22 },
			M30{ _row3.X }, M31{ _row3.Y }, M32{ _row3.Z }, M33{ _row3.W }
		{
		}
		template<typename OtherType> explicit constexpr Mat(const Mat<OtherType, 4, 4>& matrix) { for (Uint32 Index = 0; Index < 16; ++Index) this->m_Data[Index] = Type{ matrix.m_Data[Index] }; }

		operator Array_T (void) { return this->m_Data; }
		operator ConstArray_T (void) const { return this->m_Data; }
		Vec<Type, 4>& operator [] (Uint32 RowIndex) { return reinterpret_cast<Vec<Type, 4> &>(this->m_Data[RowIndex * 4]); }
		const Vec<Type, 4>& operator [] (Uint32 RowIndex) const { return reinterpret_cast<const Vec<Type, 4> &>(this->m_Data[RowIndex * 4]); }

		Vec<Type, 4> Col(int ColIndex) const { Vec<Type, 4> Value; for (Uint32 RowIndex = 0; RowIndex < 4; RowIndex++) Value[RowIndex] = this->m_Data[RowIndex * 4 + ColIndex]; return Value; }

		constexpr static Mat FromCols(const Vec<Type, 4>& col0, const Vec<Type, 4>& col1, const Vec<Type, 4>& col2, const Vec<Type, 4>& col3) {
			return Mat{
			col0.X, col1.X, col2.X, col3.X,
			col0.Y, col1.Y, col2.Y, col3.Y,
			col0.Z, col1.Z, col2.Z, col3.Z,
			col0.W, col1.W, col2.W, col3.W
			};
		}

		constexpr static Mat Diagonal(Type diag) {
			return Mat{
				diag, static_cast<Type>(0), static_cast<Type>(0),static_cast<Type>(0),
				static_cast<Type>(0), diag, static_cast<Type>(0),static_cast<Type>(0),
				static_cast<Type>(0), static_cast<Type>(0), diag, static_cast<Type>(0),
				static_cast<Type>(0), static_cast<Type>(0), static_cast<Type>(0), diag
			};
		}
		constexpr static Mat Diagonal(Vec<Type, 4> Value) {
			return Mat{
				Value.X, static_cast<Type>(0), static_cast<Type>(0),static_cast<Type>(0),
				static_cast<Type>(0), Value.Y, static_cast<Type>(0),static_cast<Type>(0),
				static_cast<Type>(0), static_cast<Type>(0), Value.Z, static_cast<Type>(0),
				static_cast<Type>(0), static_cast<Type>(0), static_cast<Type>(0), Value.W
			};
		}

		constexpr static Mat Identity(void) { return Mat<Type, 4, 4>::Diagonal(static_cast<Type>(1)); }

		constexpr static Mat Zero(void) { return Mat{ static_cast<Type>(0) }; }
	};

#pragma warning(pop)

#define DEFINE_UNARY_OPERATOR(OP) \
			template <typename Type, Uint32 RowCount, Uint32 ColCount> \
			Mat<Type, RowCount, ColCount> operator OP (const Mat<Type, RowCount, ColCount>& a) { \
				Mat<Type, RowCount, ColCount> result; \
				for (Uint32 Index = 0; Index < RowCount*ColCount; ++Index) \
					result.m_Data[Index] = OP a.m_Data[Index]; \
				return result; \
			}
#undef DEFINE_UNARY_OPERATOR

#define DEFINE_BINARY_SCALAR_OPERATORS(OP) \
			template <typename Type, Uint32 RowCount, Uint32 ColCount> \
			Mat<Type, RowCount, ColCount> operator OP (Type a, const Mat<Type, RowCount, ColCount>& b) {\
				Mat<Type, RowCount, ColCount> result; \
				for (Uint32 Index = 0; Index < RowCount*ColCount; ++Index) \
					result.m_Data[Index] = a OP b.m_Data[Index]; \
				return result; \
			} \
			template <typename Type, Uint32 RowCount, Uint32 ColCount> \
			Mat<Type, RowCount, ColCount> operator OP (const Mat<Type, RowCount, ColCount>& a, Type b) {\
				Mat<Type, RowCount, ColCount> result; \
				for (Uint32 Index = 0; Index < RowCount*ColCount; ++Index) \
					result.m_Data[Index] = a.m_Data[Index] OP b; \
				return result; \
			}


#define DEFINE_BINARY_OPERATORS(OP) \
			template <typename Type, Uint32 RowCount, Uint32 ColCount> \
			Mat<Type, RowCount, ColCount> & operator OP (const Mat<Type, RowCount, ColCount>& a,const Mat<Type, RowCount, ColCount>& b) {\
			Mat<Type, RowCount, ColCount> result; \
				for (Uint32 Index = 0; Index < RowCount*ColCount; ++Index) \
					result.m_Data[Index] = a.m_Data[Index] OP b.m_Data[Index]; \
				return result; \
			} \
			DEFINE_BINARY_SCALAR_OPERATORS(OP)

#undef DEFINE_BINARY_SCALAR_OPERATORS
#undef DEFINE_BINARY_OPERATORS


#define DEFINE_INPLACE_SCALAR_OPERATOR(OP) \
			template <typename Type, Uint32 RowCount, Uint32 ColCount> \
			Mat<Type, RowCount, ColCount> & operator OP (Mat<Type, RowCount, ColCount> & a, Type b) {\
				for (Uint32 Index = 0; Index < RowCount*ColCount; ++Index) \
					a.m_Data[Index] OP b; \
				return a; \
			}


#define DEFINE_INPLACE_OPERATORS(OP) \
			template <typename Type, Uint32 RowCount, Uint32 ColCount> \
			Mat<Type, RowCount, ColCount> & operator OP (Mat<Type, RowCount, ColCount>& a, const Mat<Type, RowCount, ColCount>& b) {\
				for (Uint32 Index = 0; Index < RowCount*ColCount; ++Index) \
					a.m_Data[Index] OP b.m_Data[Index]; \
				return a; \
			} \
			DEFINE_INPLACE_SCALAR_OPERATOR(OP)
#undef DEFINE_INPLACE_SCALAR_OPERATOR
#undef DEFINE_INPLACE_OPERATORS

#define DEFINE_RELATIONAL_OPERATORS(OP) \
			template <typename Type, Uint32 RowCount, Uint32 ColCount> \
			Mat<bool, RowCount, ColCount> operator OP (const Mat<Type, RowCount, ColCount>& a, const Mat<Type, RowCount, ColCount>& b) {\
				Mat<bool, RowCount, ColCount> result; \
				for (Uint32 Index = 0; Index < RowCount*ColCount; ++Index) \
					result.m_Data[Index] = a.m_Data[Index] OP b.m_Data[Index]; \
				return result; \
			} \
			template <typename Type, Uint32 RowCount, Uint32 ColCount> \
			Mat<bool, RowCount, ColCount> operator OP (Type a, const Mat<Type, RowCount, ColCount>& b) {\
				Mat<bool, RowCount, ColCount> result; \
				for (Uint32 Index = 0; Index < RowCount*ColCount; ++Index) \
					result.m_Data[Index] = a OP b.m_Data[Index]; \
				return result; \
			} \
			template <typename Type, Uint32 RowCount, Uint32 ColCount> \
			Mat<bool, RowCount, ColCount> operator OP (const Mat<Type, RowCount, ColCount>& a, Type b) {\
				Mat<bool, RowCount, ColCount> result; \
				for (Uint32 Index = 0; Index < RowCount*ColCount; ++Index) \
					result.m_Data[Index] = a.m_Data[Index] OP b; \
				return result; \
			}

#undef DEFINE_RELATIONAL_OPERATORS


	template <typename Type, Uint32 RowCount, Uint32 ColCount>Mat<Type, RowCount, ColCount> operator - (const Mat<Type, RowCount, ColCount>& a) {
		Mat<Type, RowCount, ColCount> result;
		for (Uint32 Index = 0; Index < RowCount * ColCount; ++Index)
			result.m_Data[Index] = -a.m_Data[Index];
		return result;
	}
	template <typename Type, Uint32 RowCount, Uint32 ColCount>Mat<Type, RowCount, ColCount> operator ! (const Mat<Type, RowCount, ColCount>& a) {
		Mat<Type, RowCount, ColCount> result;
		for (Uint32 Index = 0; Index < RowCount * ColCount; ++Index)
			result.m_Data[Index] = !a.m_Data[Index];
		return result;
	}
	template <typename Type, Uint32 RowCount, Uint32 ColCount>Mat<Type, RowCount, ColCount> operator ~ (const Mat<Type, RowCount, ColCount>& a) {
		Mat<Type, RowCount, ColCount> result;
		for (Uint32 Index = 0; Index < RowCount * ColCount; ++Index)
			result.m_Data[Index] = ~a.m_Data[Index];
		return result;
	}





	template <typename Type, Uint32 RowCount, Uint32 ColCount>Mat<Type, RowCount, ColCount> operator * (Type a, const Mat<Type, RowCount, ColCount>& b) {
		Mat<Type, RowCount, ColCount> result;
		for (Uint32 Index = 0; Index < RowCount * ColCount; ++Index)
			result.m_Data[Index] = a * b.m_Data[Index];
		return result;
	}
	template <typename Type, Uint32 RowCount, Uint32 ColCount>Mat<Type, RowCount, ColCount> operator * (const Mat<Type, RowCount, ColCount>& a, Type b) {
		Mat<Type, RowCount, ColCount> result;
		for (Uint32 Index = 0; Index < RowCount * ColCount; ++Index)
			result.m_Data[Index] = a.m_Data[Index] * b;
		return result;
	};
	template <typename Type, Uint32 RowCount, Uint32 ColCount>Mat<Type, RowCount, ColCount> operator / (Type a, const Mat<Type, RowCount, ColCount>& b) {
		Mat<Type, RowCount, ColCount> result;
		for (Uint32 Index = 0; Index < RowCount * ColCount; ++Index)
			result.m_Data[Index] = a / b.m_Data[Index];
		return result;
	}
	template <typename Type, Uint32 RowCount, Uint32 ColCount>Mat<Type, RowCount, ColCount> operator / (const Mat<Type, RowCount, ColCount>& a, Type b) {
		Mat<Type, RowCount, ColCount> result;
		for (Uint32 Index = 0; Index < RowCount * ColCount; ++Index)
			result.m_Data[Index] = a.m_Data[Index] / b;
		return result;
	}




	template <typename Type, Uint32 RowCount, Uint32 ColCount> Mat<Type, RowCount, ColCount>& operator + (const Mat<Type, RowCount, ColCount>& a, const Mat<Type, RowCount, ColCount>& b) {
		Mat<Type, RowCount, ColCount> result;
		for (Uint32 Index = 0; Index < RowCount * ColCount; ++Index)
			result.m_Data[Index] = a.m_Data[Index] + b.m_Data[Index];
		return result;
	}
	template <typename Type, Uint32 RowCount, Uint32 ColCount> Mat<Type, RowCount, ColCount> operator + (Type a, const Mat<Type, RowCount, ColCount>& b) {
		Mat<Type, RowCount, ColCount> result;
		for (Uint32 Index = 0; Index < RowCount * ColCount; ++Index)
			result.m_Data[Index] = a + b.m_Data[Index];
		return result;
	}
	template <typename Type, Uint32 RowCount, Uint32 ColCount> Mat<Type, RowCount, ColCount> operator + (const Mat<Type, RowCount, ColCount>& a, Type b) {
		Mat<Type, RowCount, ColCount> result;
		for (Uint32 Index = 0; Index < RowCount * ColCount; ++Index)
			result.m_Data[Index] = a.m_Data[Index] + b;
		return result;
	}
	template <typename Type, Uint32 RowCount, Uint32 ColCount> Mat<Type, RowCount, ColCount>& operator - (const Mat<Type, RowCount, ColCount>& a, const Mat<Type, RowCount, ColCount>& b) {
		Mat<Type, RowCount, ColCount> result;
		for (Uint32 Index = 0; Index < RowCount * ColCount; ++Index)
			result.m_Data[Index] = a.m_Data[Index] - b.m_Data[Index];
		return result;
	}
	template <typename Type, Uint32 RowCount, Uint32 ColCount> Mat<Type, RowCount, ColCount> operator - (Type a, const Mat<Type, RowCount, ColCount>& b) {
		Mat<Type, RowCount, ColCount> result;
		for (Uint32 Index = 0; Index < RowCount * ColCount; ++Index)
			result.m_Data[Index] = a - b.m_Data[Index];
		return result;
	}
	template <typename Type, Uint32 RowCount, Uint32 ColCount> Mat<Type, RowCount, ColCount> operator - (const Mat<Type, RowCount, ColCount>& a, Type b) {
		Mat<Type, RowCount, ColCount> result;
		for (Uint32 Index = 0; Index < RowCount * ColCount; ++Index)
			result.m_Data[Index] = a.m_Data[Index] - b;
		return result;
	}
	template <typename Type, Uint32 RowCount, Uint32 ColCount> Mat<Type, RowCount, ColCount>& operator & (const Mat<Type, RowCount, ColCount>& a, const Mat<Type, RowCount, ColCount>& b) {
		Mat<Type, RowCount, ColCount> result;
		for (Uint32 Index = 0; Index < RowCount * ColCount; ++Index)
			result.m_Data[Index] = a.m_Data[Index] & b.m_Data[Index];
		return result;
	}
	template <typename Type, Uint32 RowCount, Uint32 ColCount> Mat<Type, RowCount, ColCount> operator & (Type a, const Mat<Type, RowCount, ColCount>& b) {
		Mat<Type, RowCount, ColCount> result;
		for (Uint32 Index = 0; Index < RowCount * ColCount; ++Index)
			result.m_Data[Index] = a & b.m_Data[Index];
		return result;
	}
	template <typename Type, Uint32 RowCount, Uint32 ColCount> Mat<Type, RowCount, ColCount> operator & (const Mat<Type, RowCount, ColCount>& a, Type b) {
		Mat<Type, RowCount, ColCount> result;
		for (Uint32 Index = 0; Index < RowCount * ColCount; ++Index)
			result.m_Data[Index] = a.m_Data[Index] & b;
		return result;
	}
	template <typename Type, Uint32 RowCount, Uint32 ColCount> Mat<Type, RowCount, ColCount>& operator | (const Mat<Type, RowCount, ColCount>& a, const Mat<Type, RowCount, ColCount>& b) {
		Mat<Type, RowCount, ColCount> result;
		for (Uint32 Index = 0; Index < RowCount * ColCount; ++Index)
			result.m_Data[Index] = a.m_Data[Index] | b.m_Data[Index];
		return result;
	}
	template <typename Type, Uint32 RowCount, Uint32 ColCount> Mat<Type, RowCount, ColCount> operator | (Type a, const Mat<Type, RowCount, ColCount>& b) {
		Mat<Type, RowCount, ColCount> result;
		for (Uint32 Index = 0; Index < RowCount * ColCount; ++Index)
			result.m_Data[Index] = a | b.m_Data[Index];
		return result;
	}
	template <typename Type, Uint32 RowCount, Uint32 ColCount> Mat<Type, RowCount, ColCount> operator | (const Mat<Type, RowCount, ColCount>& a, Type b) {
		Mat<Type, RowCount, ColCount> result;
		for (Uint32 Index = 0; Index < RowCount * ColCount; ++Index)
			result.m_Data[Index] = a.m_Data[Index] | b;
		return result;
	}
	template <typename Type, Uint32 RowCount, Uint32 ColCount> Mat<Type, RowCount, ColCount>& operator ^ (const Mat<Type, RowCount, ColCount>& a, const Mat<Type, RowCount, ColCount>& b) {
		Mat<Type, RowCount, ColCount> result;
		for (Uint32 Index = 0; Index < RowCount * ColCount; ++Index)
			result.m_Data[Index] = a.m_Data[Index] ^ b.m_Data[Index];
		return result;
	}
	template <typename Type, Uint32 RowCount, Uint32 ColCount> Mat<Type, RowCount, ColCount> operator ^ (Type a, const Mat<Type, RowCount, ColCount>& b) {
		Mat<Type, RowCount, ColCount> result;
		for (Uint32 Index = 0; Index < RowCount * ColCount; ++Index)
			result.m_Data[Index] = a ^ b.m_Data[Index];
		return result;
	}
	template <typename Type, Uint32 RowCount, Uint32 ColCount> Mat<Type, RowCount, ColCount> operator ^ (const Mat<Type, RowCount, ColCount>& a, Type b) {
		Mat<Type, RowCount, ColCount> result;
		for (Uint32 Index = 0; Index < RowCount * ColCount; ++Index)
			result.m_Data[Index] = a.m_Data[Index] ^ b;
		return result;
	}




	template <typename Type, Uint32 RowCount, Uint32 ColCount>	Mat<Type, RowCount, ColCount>& operator *= (Mat<Type, RowCount, ColCount>& a, Type b) {
		for (Uint32 Index = 0; Index < RowCount * ColCount; ++Index)
			a.m_Data[Index] *= b;
		return a;
	}
	template <typename Type, Uint32 RowCount, Uint32 ColCount>	Mat<Type, RowCount, ColCount>& operator /= (Mat<Type, RowCount, ColCount>& a, Type b) {
		for (Uint32 Index = 0; Index < RowCount * ColCount; ++Index)
			a.m_Data[Index] /= b;
		return a;
	}


	template <typename Type, Uint32 RowCount, Uint32 ColCount>	Mat<Type, RowCount, ColCount>& operator += (Mat<Type, RowCount, ColCount>& a, const Mat<Type, RowCount, ColCount>& b) {
		for (Uint32 Index = 0; Index < RowCount * ColCount; ++Index)
			a.m_Data[Index] += b.m_Data[Index];
		return a;
	}
	template <typename Type, Uint32 RowCount, Uint32 ColCount>	Mat<Type, RowCount, ColCount>& operator += (Mat<Type, RowCount, ColCount>& a, Type b) {
		for (Uint32 Index = 0; Index < RowCount * ColCount; ++Index)
			a.m_Data[Index] += b;
		return a;
	}
	template <typename Type, Uint32 RowCount, Uint32 ColCount>	Mat<Type, RowCount, ColCount>& operator -= (Mat<Type, RowCount, ColCount>& a, const Mat<Type, RowCount, ColCount>& b) {
		for (Uint32 Index = 0; Index < RowCount * ColCount; ++Index)
			a.m_Data[Index] -= b.m_Data[Index];
		return a;
	}
	template <typename Type, Uint32 RowCount, Uint32 ColCount> Mat<Type, RowCount, ColCount>& operator -= (Mat<Type, RowCount, ColCount>& a, Type b) {
		for (Uint32 Index = 0; Index < RowCount * ColCount; ++Index)
			a.m_Data[Index] -= b;
		return a;
	}
	template <typename Type, Uint32 RowCount, Uint32 ColCount>	Mat<Type, RowCount, ColCount>& operator &= (Mat<Type, RowCount, ColCount>& a, const Mat<Type, RowCount, ColCount>& b) {
		for (Uint32 Index = 0; Index < RowCount * ColCount; ++Index)
			a.m_Data[Index] &= b.m_Data[Index];
		return a;
	}
	template <typename Type, Uint32 RowCount, Uint32 ColCount>	Mat<Type, RowCount, ColCount>& operator &= (Mat<Type, RowCount, ColCount>& a, Type b) {
		for (Uint32 Index = 0; Index < RowCount * ColCount; ++Index)
			a.m_Data[Index] &= b;
		return a;
	}
	template <typename Type, Uint32 RowCount, Uint32 ColCount>	Mat<Type, RowCount, ColCount>& operator |= (Mat<Type, RowCount, ColCount>& a, const Mat<Type, RowCount, ColCount>& b) {
		for (Uint32 Index = 0; Index < RowCount * ColCount; ++Index)
			a.m_Data[Index] |= b.m_Data[Index];
		return a;
	}
	template <typename Type, Uint32 RowCount, Uint32 ColCount>	Mat<Type, RowCount, ColCount>& operator |= (Mat<Type, RowCount, ColCount>& a, Type b) {
		for (Uint32 Index = 0; Index < RowCount * ColCount; ++Index)
			a.m_Data[Index] |= b;
		return a;
	}
	template <typename Type, Uint32 RowCount, Uint32 ColCount>	Mat<Type, RowCount, ColCount>& operator ^= (Mat<Type, RowCount, ColCount>& a, const Mat<Type, RowCount, ColCount>& b) {
		for (Uint32 Index = 0; Index < RowCount * ColCount; ++Index)
			a.m_Data[Index] ^= b.m_Data[Index];
		return a;
	}
	template <typename Type, Uint32 RowCount, Uint32 ColCount>	Mat<Type, RowCount, ColCount>& operator ^= (Mat<Type, RowCount, ColCount>& a, Type b) {
		for (Uint32 Index = 0; Index < RowCount * ColCount; ++Index)
			a.m_Data[Index] ^= b;
		return a;
	}




	template <typename Type, Uint32 RowCount, Uint32 ColCount>	Mat<bool, RowCount, ColCount> operator == (const Mat<Type, RowCount, ColCount>& a, const Mat<Type, RowCount, ColCount>& b) {
		Mat<bool, RowCount, ColCount> result;
		for (Uint32 Index = 0; Index < RowCount * ColCount; ++Index)
			result.m_Data[Index] = a.m_Data[Index] == b.m_Data[Index];
		return result;
	}
	template <typename Type, Uint32 RowCount, Uint32 ColCount>	Mat<bool, RowCount, ColCount> operator == (Type a, const Mat<Type, RowCount, ColCount>& b) {
		Mat<bool, RowCount, ColCount> result;
		for (Uint32 Index = 0; Index < RowCount * ColCount; ++Index)
			result.m_Data[Index] = a == b.m_Data[Index];
		return result;
	}
	template <typename Type, Uint32 RowCount, Uint32 ColCount>	Mat<bool, RowCount, ColCount> operator == (const Mat<Type, RowCount, ColCount>& a, Type b) {
		Mat<bool, RowCount, ColCount> result;
		for (Uint32 Index = 0; Index < RowCount * ColCount; ++Index)
			result.m_Data[Index] = a.m_Data[Index] == b;
		return result;
	}
	template <typename Type, Uint32 RowCount, Uint32 ColCount>	Mat<bool, RowCount, ColCount> operator != (const Mat<Type, RowCount, ColCount>& a, const Mat<Type, RowCount, ColCount>& b) {
		Mat<bool, RowCount, ColCount> result;
		for (Uint32 Index = 0; Index < RowCount * ColCount; ++Index)
			result.m_Data[Index] = a.m_Data[Index] != b.m_Data[Index];
		return result;
	}
	template <typename Type, Uint32 RowCount, Uint32 ColCount>	Mat<bool, RowCount, ColCount> operator != (Type a, const Mat<Type, RowCount, ColCount>& b) {
		Mat<bool, RowCount, ColCount> result;
		for (Uint32 Index = 0; Index < RowCount * ColCount; ++Index)
			result.m_Data[Index] = a != b.m_Data[Index];
		return result;
	}
	template <typename Type, Uint32 RowCount, Uint32 ColCount>	Mat<bool, RowCount, ColCount> operator != (const Mat<Type, RowCount, ColCount>& a, Type b) {
		Mat<bool, RowCount, ColCount> result;
		for (Uint32 Index = 0; Index < RowCount * ColCount; ++Index)
			result.m_Data[Index] = a.m_Data[Index] != b;
		return result;
	}
	template <typename Type, Uint32 RowCount, Uint32 ColCount>	Mat<bool, RowCount, ColCount> operator < (const Mat<Type, RowCount, ColCount>& a, const Mat<Type, RowCount, ColCount>& b) {
		Mat<bool, RowCount, ColCount> result;
		for (Uint32 Index = 0; Index < RowCount * ColCount; ++Index)
			result.m_Data[Index] = a.m_Data[Index] < b.m_Data[Index];
		return result;
	}
	template <typename Type, Uint32 RowCount, Uint32 ColCount>	Mat<bool, RowCount, ColCount> operator < (Type a, const Mat<Type, RowCount, ColCount>& b) {
		Mat<bool, RowCount, ColCount> result;
		for (Uint32 Index = 0; Index < RowCount * ColCount; ++Index)
			result.m_Data[Index] = a < b.m_Data[Index];
		return result;
	}
	template <typename Type, Uint32 RowCount, Uint32 ColCount>	Mat<bool, RowCount, ColCount> operator < (const Mat<Type, RowCount, ColCount>& a, Type b) {
		Mat<bool, RowCount, ColCount> result;
		for (Uint32 Index = 0; Index < RowCount * ColCount; ++Index)
			result.m_Data[Index] = a.m_Data[Index] < b;
		return result;
	}
	template <typename Type, Uint32 RowCount, Uint32 ColCount>	Mat<bool, RowCount, ColCount> operator > (const Mat<Type, RowCount, ColCount>& a, const Mat<Type, RowCount, ColCount>& b) {
		Mat<bool, RowCount, ColCount> result;
		for (Uint32 Index = 0; Index < RowCount * ColCount; ++Index)
			result.m_Data[Index] = a.m_Data[Index] > b.m_Data[Index];
		return result;
	}
	template <typename Type, Uint32 RowCount, Uint32 ColCount>	Mat<bool, RowCount, ColCount> operator > (Type a, const Mat<Type, RowCount, ColCount>& b) {
		Mat<bool, RowCount, ColCount> result;
		for (Uint32 Index = 0; Index < RowCount * ColCount; ++Index)
			result.m_Data[Index] = a > b.m_Data[Index];
		return result;
	}
	template <typename Type, Uint32 RowCount, Uint32 ColCount>	Mat<bool, RowCount, ColCount> operator > (const Mat<Type, RowCount, ColCount>& a, Type b) {
		Mat<bool, RowCount, ColCount> result;
		for (Uint32 Index = 0; Index < RowCount * ColCount; ++Index)
			result.m_Data[Index] = a.m_Data[Index] > b;
		return result;
	}
	template <typename Type, Uint32 RowCount, Uint32 ColCount>	Mat<bool, RowCount, ColCount> operator <= (const Mat<Type, RowCount, ColCount>& a, const Mat<Type, RowCount, ColCount>& b) {
		Mat<bool, RowCount, ColCount> result;
		for (Uint32 Index = 0; Index < RowCount * ColCount; ++Index)
			result.m_Data[Index] = a.m_Data[Index] <= b.m_Data[Index];
		return result;
	}
	template <typename Type, Uint32 RowCount, Uint32 ColCount>	Mat<bool, RowCount, ColCount> operator <= (Type a, const Mat<Type, RowCount, ColCount>& b) {
		Mat<bool, RowCount, ColCount> result;
		for (Uint32 Index = 0; Index < RowCount * ColCount; ++Index)
			result.m_Data[Index] = a <= b.m_Data[Index];
		return result;
	}
	template <typename Type, Uint32 RowCount, Uint32 ColCount>	Mat<bool, RowCount, ColCount> operator <= (const Mat<Type, RowCount, ColCount>& a, Type b) {
		Mat<bool, RowCount, ColCount> result;
		for (Uint32 Index = 0; Index < RowCount * ColCount; ++Index)
			result.m_Data[Index] = a.m_Data[Index] <= b;
		return result;
	}
	template <typename Type, Uint32 RowCount, Uint32 ColCount>	Mat<bool, RowCount, ColCount> operator >= (const Mat<Type, RowCount, ColCount>& a, const Mat<Type, RowCount, ColCount>& b) {
		Mat<bool, RowCount, ColCount> result;
		for (Uint32 Index = 0; Index < RowCount * ColCount; ++Index)
			result.m_Data[Index] = a.m_Data[Index] >= b.m_Data[Index];
		return result;
	}
	template <typename Type, Uint32 RowCount, Uint32 ColCount>	Mat<bool, RowCount, ColCount> operator >= (Type a, const Mat<Type, RowCount, ColCount>& b) {
		Mat<bool, RowCount, ColCount> result;
		for (Uint32 Index = 0; Index < RowCount * ColCount; ++Index)
			result.m_Data[Index] = a >= b.m_Data[Index];
		return result;
	}
	template <typename Type, Uint32 RowCount, Uint32 ColCount>	Mat<bool, RowCount, ColCount> operator >= (const Mat<Type, RowCount, ColCount>& a, Type b) {
		Mat<bool, RowCount, ColCount> result;
		for (Uint32 Index = 0; Index < RowCount * ColCount; ++Index)
			result.m_Data[Index] = a.m_Data[Index] >= b;
		return result;
	}

	// Matrix multiplication

	template <typename Type, Uint32 RowCount, Uint32 Inner, Uint32 ColCount>	Mat<Type, RowCount, ColCount> operator * (const Mat<Type, RowCount, Inner>& a, const Mat<Type, Inner, ColCount>& b) {
		auto result{ Mat<Type, RowCount, ColCount>::Zero() };
		for (Uint32 RowIndex = 0; RowIndex < RowCount; ++RowIndex)
			for (Uint32 ColIndex = 0; ColIndex < ColCount; ++ColIndex)
				for (Uint32 InnerIndex = 0; InnerIndex < Inner; ++InnerIndex)
					result[RowIndex][ColIndex] += a[RowIndex][InnerIndex] * b[InnerIndex][ColIndex];
		return result;
	}

	template <typename Type, Uint32 RowCount, Uint32 ColCount>	Mat<Type, RowCount, ColCount>& operator *= (Mat<Type, RowCount, ColCount>& a, const Mat<Type, ColCount, ColCount>& b) { return a = a * b; }


	// Matrix-Vec multiplication

	template <typename Type, Uint32 RowCount, Uint32 ColCount>Vec<Type, RowCount> operator * (const Mat<Type, RowCount, ColCount>& a, const Vec<Type, ColCount>& b) {
		auto result{ Vec<Type, RowCount>::Zero() };
		for (Uint32 RowIndex = 0; RowIndex < RowCount; ++RowIndex)
			for (Uint32 ColIndex = 0; ColIndex < ColCount; ++ColIndex)
				result[RowIndex] += a[RowIndex][ColIndex] * b[ColIndex];
		return result;
	}

	template <typename Type, Uint32 RowCount, Uint32 ColCount>	Vec<Type, ColCount> operator * (const Vec<Type, RowCount>& a, const Mat<Type, RowCount, ColCount>& b) {
		auto result{ Vec<Type, ColCount>::Zero() };
		for (Uint32 RowIndex = 0; RowIndex < RowCount; ++RowIndex)
			for (Uint32 ColIndex = 0; ColIndex < ColCount; ++ColIndex)
				result[ColIndex] += a[RowIndex] * b[RowIndex][ColIndex];
		return result;
	}

	template <typename Type>	Vec<Type, 3> operator * (const Mat<Type, 3, 3>& a, const Vec<Type, 3>& b) {
		Vec<Type, 3> result;
		result.X = a.Row0.X * b.X + a.Row0.Y * b.Y + a.Row0.Z * b.Z;
		result.Y = a.Row1.X * b.X + a.Row1.Y * b.Y + a.Row1.Z * b.Z;
		result.Z = a.Row2.X * b.X + a.Row2.Y * b.Y + a.Row2.Z * b.Z;
		return result;
	}

	template <typename Type>	Vec<Type, 3> operator * (const Vec<Type, 3>& a, const Mat<Type, 3, 3>& b) {
		Vec<Type, 3> result;
		result.X = a.X * b.Row0.X + a.Y * b.Row1.X + a.Z * b.Row2.X;
		result.Y = a.X * b.Row0.Y + a.Y * b.Row1.Y + a.Z * b.Row2.Y;
		result.Z = a.X * b.Row0.Z + a.Y * b.Row1.Z + a.Z * b.Row2.Z;
		return result;
	}

	template <typename Type>	Vec<Type, 4> operator * (const Mat<Type, 4, 4>& a, const Vec<Type, 4>& b) {
		Vec<Type, 4> result;
		result.X = a.Row0.X * b.X + a.Row0.Y * b.Y + a.Row0.Z * b.Z + a.Row0.W * b.W;
		result.Y = a.Row1.X * b.X + a.Row1.Y * b.Y + a.Row1.Z * b.Z + a.Row1.W * b.W;
		result.Z = a.Row2.X * b.X + a.Row2.Y * b.Y + a.Row2.Z * b.Z + a.Row2.W * b.W;
		result.W = a.Row3.X * b.X + a.Row3.Y * b.Y + a.Row3.Z * b.Z + a.Row3.W * b.W;
		return result;
	}

	template <typename Type>	Vec<Type, 4> operator * (const Vec<Type, 4>& a, const Mat<Type, 4, 4>& b) {
		Vec<Type, 4> result;
		result.X = a.X * b.Row0.X + a.Y * b.Row1.X + a.Z * b.Row2.X + a.W * b.row3.X;
		result.Y = a.X * b.Row0.Y + a.Y * b.Row1.Y + a.Z * b.Row2.Y + a.W * b.row3.Y;
		result.Z = a.X * b.Row0.Z + a.Y * b.Row1.Z + a.Z * b.Row2.Z + a.W * b.row3.Z;
		result.W = a.X * b.Row0.W + a.Y * b.Row1.W + a.Z * b.Row2.W + a.W * b.row3.W;
		return result;
	}

	template <typename Type, Uint32 N>	Vec<Type, N> operator *= (Vec<Type, N>& a, const Mat<Type, N, N>& b) { return a = a * b; }

	// Other math functions

	template <typename Type, Uint32 RowCount, Uint32 ColCount>Mat<Type, ColCount, RowCount> Transpose(const Mat<Type, RowCount, ColCount>& a) {
		Mat<Type, ColCount, RowCount> result;
		for (Uint32 RowIndex = 0; RowIndex < RowCount; ++RowIndex)
			for (Uint32 ColIndex = 0; ColIndex < ColCount; ++ColIndex)
				result[ColIndex][RowIndex] = a[RowIndex][ColIndex];
		return result;
	}

	template <typename Type, Uint32 N>Mat<Type, N, N> Pow(const Mat<Type, N, N>& a, Uint32 b) {
		if (b <= 0)
			return Mat<Type, N, N>::Identity();
		if (b == 1)
			return a;
		auto oddpart{ Mat<Type, N, N>::Identity() }, evenpart{ a };
		while (b > 1) {
			if (b % 2 == 1)
				oddpart *= evenpart;

			evenpart *= evenpart;
			b /= 2;
		}
		return oddpart * evenpart;
	}

	template <typename Type, Uint32 N>Mat<Type, N, N> Inverse(const Mat<Type, N, N>& m) {
		// Calculate inverse using Gaussian elimination
		auto a{ m }, b{ Mat<Type, N, N>::Identity() };

		// Loop through columns
		for (Uint32 ColIndex = 0; ColIndex < N; ++ColIndex) {
			// Select pivot element: maximum magnitude in this column at or below main diagonal
			Uint32 pivot{ ColIndex };
			for (Uint32 RowIndex = ColIndex + 1; RowIndex < N; ++RowIndex)
				if (Abs(a[RowIndex][ColIndex]) > Abs(a[pivot][ColIndex]))
					pivot = RowIndex;
			if (Abs(a[pivot][ColIndex]) < Epsilon)
				return Mat<Type, N, N>{ NaN };

			// Interchange rows to put pivot element on the diagonal,
			// if it is not already there
			if (pivot != ColIndex) {
				Swap(a[ColIndex], a[pivot]);
				Swap(b[ColIndex], b[pivot]);
			}

			// Divide the whole row by the pivot element
			if (a[ColIndex][ColIndex] != static_cast<Type>(1)) {// Skip if already equal to 1
				Type scale{ a[ColIndex][ColIndex] };
				a[ColIndex] /= scale;
				b[ColIndex] /= scale;
				// Now the pivot element has become 1
			}

			// Subtract this row from others to make the rest of column j zero
			for (Uint32 RowIndex = 0; RowIndex < N; ++RowIndex)
				if ((RowIndex != ColIndex) && (Abs(a[RowIndex][ColIndex]) > Epsilon)) {	// skip rows already zero
					Type scale{ -a[RowIndex][ColIndex] };
					a[RowIndex] += a[ColIndex] * scale;
					b[RowIndex] += b[ColIndex] * scale;
				}
		}

		// At this point, a should have been transformed to the identity matrix,
		// and b should have been transformed into the inverse of the original a.
		return b;
	}
	template <typename Type>Mat<Type, 2, 2> Inverse(const Mat<Type, 2, 2>& a) {
		Mat<Type, 2, 2> result{
			a[1][1], -a[0][1],
			-a[1][0], a[0][0]
		};
		return result / Determinant(a);
	}

	template <typename Type, Uint32 N>Type Determinant(const Mat<Type, N, N>& m) {
		// Calculate determinant using Gaussian elimination

		Mat<Type, N, N> a = m;
		Type result{ 1 };

		// Loop through columns
		for (Uint32 ColIndex = 0; ColIndex < N; ++ColIndex) {
			// Select pivot element: maximum magnitude in this column at or below main diagonal
			Uint32 pivot = ColIndex;
			for (Uint32 RowIndex = ColIndex + 1; RowIndex < N; ++RowIndex)
				if (Abs(a[RowIndex][ColIndex]) > Abs(a[pivot][ColIndex]))
					pivot = RowIndex;
			if (Abs(a[pivot][ColIndex]) < Epsilon)
				return Type{ 0 };

			// Interchange rows to put pivot element on the diagonal,
			// if it is not already there
			if (pivot != ColIndex) {
				Swap(a[ColIndex], a[pivot]);
				result *= Type{ -1 };
			}

			// Divide the whole row by the pivot element
			if (a[ColIndex][ColIndex] != Type{ 1 }) {								// Skip if already equal to 1
				Type scale{ a[ColIndex][ColIndex] };
				a[ColIndex] /= scale;
				result *= scale;
				// Now the pivot element has become 1
			}

			// Subtract this row from others to make the rest of column j zero
			for (int RowIndex = 0; RowIndex < N; ++RowIndex)
				if ((RowIndex != ColIndex) && (Abs(a[RowIndex][ColIndex]) > Epsilon)) {		// skip rows already zero
					Type scale{ -a[RowIndex][ColIndex] };
					a[RowIndex] += a[ColIndex] * scale;
				}
		}

		// At this point, a should have been transformed to the identity matrix,
		// and we've accumulated the original a's determinant in result.
		return result;
	}
	template <typename Type>Type Determinant(const Mat<Type, 2, 2>& a) { return Type{ a[0][0] * a[1][1] - a[0][1] * a[1][0] }; }
	template <typename Type>Type Determinant(const Mat<Type, 3, 3>& a) {
		return Type{
			 (a[0][0] * a[1][1] * a[2][2]
			+ a[0][1] * a[1][2] * a[2][0]
			+ a[0][2] * a[1][0] * a[2][1])

			- (a[2][0] * a[1][1] * a[0][2]
			+ a[2][1] * a[1][2] * a[0][0]
			+ a[2][2] * a[1][0] * a[0][1])
		};
	}

	template <typename Type, int N>Type Trace(const Mat<Type, N, N>& a) {
		Type result{ 0 };
		for (Uint32 RowIndex = 0; RowIndex < N; ++RowIndex)
			result += a[RowIndex][RowIndex];
		return result;
	}

	template <typename Type, Uint32 N>Mat<Type, N, N> Diagonal(Type a) { return Mat<Type, N, N>::Diagonal(a); }
	template <typename Type, Uint32 N>Mat<Type, N, N> Diagonal(const Vec<Type, N>& a) { return Mat<Type, N, N>::Diagonal(a); }

	template <typename Type, Uint32 RowCount, Uint32 ColCount>Mat<Type, RowCount, ColCount> OuterProduct(const Vec<Type, RowCount>& a, const Vec<Type, ColCount>& b) {
		Mat<Type, RowCount, ColCount> result;
		for (Uint32 RowIndex = 0; RowIndex < RowCount; ++RowIndex)
			result[RowIndex] = a[RowIndex] * b;
		return result;
	}

	template <typename Type, Uint32 RowCount, Uint32 ColCount>Mat<bool, RowCount, ColCount> IsNear(const Mat<Type, RowCount, ColCount>& a, const Mat<Type, RowCount, ColCount>& b, float epsilon = Epsilon) {
		Mat<bool, RowCount, ColCount> result;
		for (Uint32 RowIndex = 0; RowIndex < RowCount * ColCount; ++RowIndex)
			result.m_Data[RowIndex] = Isnear(a.m_Data[RowIndex], b.m_Data[RowIndex], epsilon);
		return result;
	}
	template <typename Type, Uint32 RowCount, Uint32 ColCount>Mat<bool, RowCount, ColCount> IsNear(Mat<Type, RowCount, ColCount> const& a, Type b, float epsilon = Epsilon) {
		Mat<bool, RowCount, ColCount> result;
		for (Uint32 RowIndex = 0; RowIndex < RowCount * ColCount; ++RowIndex)
			result.m_Data[RowIndex] = Isnear(a.m_Data[RowIndex], b, epsilon);
		return result;
	}
	template <typename Type, Uint32 RowCount, Uint32 ColCount>Mat<bool, RowCount, ColCount> IsNear(Type a, const Mat<Type, RowCount, ColCount>& b, float epsilon = Epsilon) {
		Mat<bool, RowCount, ColCount> result;
		for (Uint32 RowIndex = 0; RowIndex < RowCount * ColCount; ++RowIndex)
			result.m_Data[RowIndex] = Isnear(a, b.m_Data[RowIndex], epsilon);
		return result;
	}

	template <typename Type, Uint32 RowCount, Uint32 ColCount>Mat<bool, RowCount, ColCount> IsFinite(const Mat<Type, RowCount, ColCount>& a) {
		Mat<bool, RowCount, ColCount> result;
		for (Uint32 RowIndex = 0; RowIndex < RowCount * ColCount; ++RowIndex)
			result.m_Data[RowIndex] = Isfinite(a.m_Data[RowIndex]);
		return result;
	}

	template <typename Type, Uint32 RowCount, Uint32 ColCount>Mat<Uint32, RowCount, ColCount> Round(const Mat<Type, RowCount, ColCount>& a) {
		Mat<Uint32, RowCount, ColCount> result;
		for (Uint32 Indedx = 0; Indedx < RowCount * ColCount; ++Indedx)
			result.m_Data[Indedx] = Round(a.m_Data[Indedx]);
		return result;
	}


	template <Uint32 RowCount, Uint32 ColCount>bool Any(const Mat<bool, RowCount, ColCount>& a) {
		bool result{ false };
		for (Uint32 Index = 0; Index < RowCount * ColCount; ++Index)
			result = result || a.m_Data[Index];
		return result;
	}

	template <Uint32 RowCount, Uint32 ColCount>bool All(Mat<bool, RowCount, ColCount> const& a) {
		bool result{ true };
		for (Uint32 Index = 0; Index < RowCount * ColCount; ++Index)
			result = result && a.m_Data[Index];
		return result;
	}

	template <typename Type, Uint32 RowCount, Uint32 ColCount>Mat<Type, RowCount, ColCount> Select(const Mat<bool, RowCount, ColCount>& cond, const Mat<Type, RowCount, ColCount>& a, const Mat<Type, RowCount, ColCount>& b) {
		Mat<Type, RowCount, ColCount> result;
		for (int Index = 0; Index < RowCount * ColCount; ++Index)
			result.m_Data[Index] = cond.m_Data[Index] ? a.m_Data[Index] : b.m_Data[Index];
		return result;
	}

	template <typename Type, Uint32 RowCount, Uint32 ColCount>Mat<Type, RowCount, ColCount> Min(const Mat<Type, RowCount, ColCount>& a, const Mat<Type, RowCount, ColCount>& b) { return Select(a < b, a, b); }

	template <typename Type, Uint32 RowCount, Uint32 ColCount>Mat<Type, RowCount, ColCount> Max(const Mat<Type, RowCount, ColCount>& a, const Mat<Type, RowCount, ColCount>& b) { return Select(a < b, b, a); }

	template <typename Type, Uint32 RowCount, Uint32 ColCount>Mat<Type, RowCount, ColCount> Abs(const Mat<Type, RowCount, ColCount>& a) { return Select(a < Type{ 0 }, -a, a); }

	template <typename Type, Uint32 RowCount, Uint32 ColCount>Mat<Type, RowCount, ColCount> Saturate(const Mat<Type, RowCount, ColCount>& value) { return Clamp(value, Mat<Type, RowCount, ColCount>::Zero(), Mat<Type, RowCount, ColCount>{static_cast<Type>(1)}); }

	template <typename Type, Uint32 RowCount, Uint32 ColCount>Type MinComponent(const Mat<Type, RowCount, ColCount>& a) {
		Type result = a.m_Data[0];
		for (Uint32 Inex = 1; Inex < RowCount * ColCount; ++Inex)
			result = Min(result, a.m_Data[Inex]);
		return result;
	}

	template <typename Type, Uint32 RowCount, Uint32 ColCount>Type MaxComponent(const Mat<Type, RowCount, ColCount>& a) {
		Type result = a.m_Data[0];
		for (Uint32 Index = 1; Index < RowCount * ColCount; ++Index)
			result = Max(result, a.m_Data[Index]);
		return result;
	}





}