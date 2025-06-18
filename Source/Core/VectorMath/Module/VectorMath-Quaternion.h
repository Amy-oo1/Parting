#pragma once

#include "Core/Algorithm/Module/Algorithm.h"

#include<limits>
#include<cmath>
#include<algorithm>

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"


#include "Core/VectorMath/Module/VectorMath-Misc.h"
#include "Core/VectorMath/Module/VectorMath-Vec.h"
#include "Core/VectorMath/Module/VectorMath-Mat.h"
#include "Core/VectorMath/Module/VectorMath-Affine.h"

namespace Math {

	template<typename Type>
	struct Quaternion {
		// Conversions to C arrays of fixed size
		using Array_T = Type(&)[4];
		using ConstArray_T = const Type(&)[4];

		Type W{ 1.f }, X{ 0.f }, Y{ 0.f }, Z{ 0.f };

		Quaternion(void) = default;
		Quaternion(Type W, Type x, Type y, Type z) :W{ W }, X{ x }, Y{ y }, Z{ z } {}

		operator Array_T (void) { return reinterpret_cast<Array_T>(&this->W); }

		operator ConstArray_T (void) const { return reinterpret_cast<ConstArray_T>(&this->W); }

		// Subscript operators - built-in subscripts are ambiguous without these
		Type& operator [] (Uint32 i) { return (&this->W)[i]; }
		const Type& operator [] (Uint32 i) const { return (&this->W)[i]; }

		template<typename OtherType>
		explicit Quaternion(const Quaternion<OtherType>& q) : W{ static_cast<Type>(q.W) }, X{ static_cast<Type>(q.X) }, Y{ static_cast<Type>(q.Y) }, Z{ static_cast<Type>(q.Z) } {}

		STDNODISCARD Vec<Type, 4> ToXYZW(void) const { return Vec{ this->X, this->Y, this->Z, this->W }; }

		STDNODISCARD Vec<Type, 4> ToWXYZ(void) const { return Vec{ this->W, this->X, this->Y, this->Z }; }

		// Convert to a Mat
		STDNODISCARD Mat<Type, 3, 3> ToMat(void) const {
			return Mat<Type, 3, 3>{
				1 - 2 * (Y * Y + Z * Z), 2 * (X * Y + Z * W), 2 * (X * Z - Y * W),
					2 * (X * Y - Z * W), 1 - 2 * (X * X + Z * Z), 2 * (Y * Z + X * W),
					2 * (X * Z + Y * W), 2 * (Y * Z - X * W), 1 - 2 * (X * X + Y * Y)
			};
		}

		// Convert to an Affine transform
		STDNODISCARD Affine<Type, 3> ToAffine(void) const { return Affine<Type, 3>{ this->ToMat(), static_cast<Type>(0) }; }

		// Conversion to bool is not allowed (otherwise would
		// happen implicitly through array conversions)

		static Quaternion Identity(void) { return Quaternion{}; }

		static Quaternion FromWXYZ(Type W, const Vec<Type, 3>& v) { return Quaternion{ W, v.X, v.Y, v.Z }; }
		static Quaternion FromWXYZ(const Type* v) { return Quaternion{ v[0], v[1], v[2], v[3] }; }

		static Quaternion FromXYZW(const Vec<Type, 4>& v) { return Quaternion{ v.W, v.X, v.Y, v.Z }; }
		static Quaternion FromXYZW(const Vec<Type, 3>& v, Type W) { return Quaternion{ W, v.X, v.Y, v.Z }; }
		static Quaternion FromXYZW(const Type* v) { return Quaternion{ v[3], v[0], v[1], v[2] }; }
	};


#define DEFINE_UNARY_OPERATOR(OP) template<typename Type> Quaternion<Type> operator OP (const Quaternion<Type>& a) { return Quaternion<Type>{ OP a.W, OP a.X, OP a.Y, OP a.Z }; }

#define DEFINE_BINARY_SCALAR_OPERATORS(OP) \
			template<typename Type> Quaternion<Type> operator OP (Type a, const Quaternion<Type>& b) { return Quaternion<Type>{ a OP b.W, a OP b.X, a OP b.Y, a OP b.Z }; } \
			template<typename Type> Quaternion<Type> operator OP (const Quaternion<Type>& a, Type b) { return Quaternion<Type>{ a.W OP b, a.X OP b, a.Y OP b, a.Z OP b }; }

#define DEFINE_BINARY_OPERATORS(OP) \
			template<typename Type> \
			Quaternion<Type> operator OP (const Quaternion<Type>& a, const Quaternion<Type>& b) { return Quaternion<Type>{ a.W OP b.W, a.X OP b.X, a.Y OP b.Y, a.Z OP b.Z }; } \
			DEFINE_BINARY_SCALAR_OPERATORS(OP)

#define DEFINE_INPLACE_SCALAR_OPERATOR(OP) \
			template<typename Type> Quaternion<Type> & operator OP (Quaternion<Type> & a, Type b) {\
				a.W OP b; \
				a.X OP b; \
				a.Y OP b; \
				a.Z OP b; \
				return a; \
			}

#define DEFINE_INPLACE_OPERATORS(OP) \
			template<typename Type> \
			Quaternion<Type> & operator OP (Quaternion<Type> & a, const Quaternion<Type>& b) {\
				a.W OP b.W; \
				a.X OP b.X; \
				a.Y OP b.Y; \
				a.Z OP b.Z; \
				return a; \
			} \
			DEFINE_INPLACE_SCALAR_OPERATOR(OP)

#define DEFINE_RELATIONAL_OPERATORS(OP) \
			template<typename Type> Vec<bool,4> operator OP (const Quaternion<Type>& a, const Quaternion<Type>& b) {\
	Vec<bool, 4> result; \
		result.X = a.W OP b.W; \
		result.Y = a.X OP b.X; \
		result.Z = a.Y OP b.Y; \
		result.W = a.Z OP b.Z; \
		return result; \
		} \
		template<typename Type> Vec<bool, 4> operator OP (Type a, const Quaternion<Type>& b) {\
			Vec<bool, 4> result; \
			result.X = a OP b.W; \
			result.Y = a OP b.X; \
			result.Z = a OP b.Y; \
			result.W = a OP b.Z; \
			return result; \
		} \
		template<typename Type> Vec<bool, 4> operator OP (const Quaternion<Type>& a, Type b) {\
			Vec<bool, 4> result; \
				result.X = a.W OP b; \
				result.Y = a.X OP b; \
				result.Z = a.Y OP b; \
				result.W = a.Z OP b; \
				return result; \
		}

	/*DEFINE_BINARY_OPERATORS(+)
	DEFINE_BINARY_OPERATORS(-)
	DEFINE_UNARY_OPERATOR(-)
	DEFINE_BINARY_SCALAR_OPERATORS(*)
	DEFINE_BINARY_SCALAR_OPERATORS(/ )

	DEFINE_INPLACE_OPERATORS(+= )
	DEFINE_INPLACE_OPERATORS(-= )
	DEFINE_INPLACE_SCALAR_OPERATOR(*= )
	DEFINE_INPLACE_SCALAR_OPERATOR(/= )

	DEFINE_RELATIONAL_OPERATORS(== )
	DEFINE_RELATIONAL_OPERATORS(!= )
	DEFINE_RELATIONAL_OPERATORS(< )
	DEFINE_RELATIONAL_OPERATORS(> )
	DEFINE_RELATIONAL_OPERATORS(<= )
	DEFINE_RELATIONAL_OPERATORS(>= )*/

#undef DEFINE_UNARY_OPERATOR
#undef DEFINE_BINARY_OPERATORS
#undef DEFINE_INPLACE_OPERATORS
#undef DEFINE_RELATIONAL_OPERATORS


	template<typename Type> Quaternion<Type> operator + (const Quaternion<Type>& a, const Quaternion<Type>& b) { return Quaternion<Type>{ a.W + b.W, a.X + b.X, a.Y + b.Y, a.Z + b.Z }; }
	template<typename Type> Quaternion<Type> operator + (Type a, const Quaternion<Type>& b) { return Quaternion<Type>{ a + b.W, a + b.X, a + b.Y, a + b.Z }; }
	template<typename Type> Quaternion<Type> operator + (const Quaternion<Type>& a, Type b) { return Quaternion<Type>{ a.W + b, a.X + b, a.Y + b, a.Z + b }; }
	template<typename Type> Quaternion<Type> operator - (const Quaternion<Type>& a, const Quaternion<Type>& b) { return Quaternion<Type>{ a.W - b.W, a.X - b.X, a.Y - b.Y, a.Z - b.Z }; }
	template<typename Type> Quaternion<Type> operator - (Type a, const Quaternion<Type>& b) { return Quaternion<Type>{ a - b.W, a - b.X, a - b.Y, a - b.Z }; }
	template<typename Type> Quaternion<Type> operator - (const Quaternion<Type>& a, Type b) { return Quaternion<Type>{ a.W - b, a.X - b, a.Y - b, a.Z - b }; }
	template<typename Type> Quaternion<Type> operator - (const Quaternion<Type>& a) { return Quaternion<Type>{ -a.W, -a.X, -a.Y, -a.Z }; }
	template<typename Type> Quaternion<Type> operator * (Type a, const Quaternion<Type>& b) { return Quaternion<Type>{ a* b.W, a* b.X, a* b.Y, a* b.Z }; }
	template<typename Type> Quaternion<Type> operator * (const Quaternion<Type>& a, Type b) { return Quaternion<Type>{ a.W* b, a.X* b, a.Y* b, a.Z* b }; }
	template<typename Type> Quaternion<Type> operator / (Type a, const Quaternion<Type>& b) { return Quaternion<Type>{ a / b.W, a / b.X, a / b.Y, a / b.Z }; }
	template<typename Type> Quaternion<Type> operator / (const Quaternion<Type>& a, Type b) { return Quaternion<Type>{ a.W / b, a.X / b, a.Y / b, a.Z / b }; }

	template<typename Type> Quaternion<Type>& operator += (Quaternion<Type>& a, const Quaternion<Type>& b) { a.W += b.W; a.X += b.X; a.Y += b.Y; a.Z += b.Z; return a; }
	template<typename Type> Quaternion<Type>& operator += (Quaternion<Type>& a, Type b) { a.W += b; a.X += b; a.Y += b; a.Z += b; return a; }
	template<typename Type> Quaternion<Type>& operator -= (Quaternion<Type>& a, const Quaternion<Type>& b) { a.W -= b.W; a.X -= b.X; a.Y -= b.Y; a.Z -= b.Z; return a; }
	template<typename Type> Quaternion<Type>& operator -= (Quaternion<Type>& a, Type b) { a.W -= b; a.X -= b; a.Y -= b; a.Z -= b; return a; }
	template<typename Type> Quaternion<Type>& operator *= (Quaternion<Type>& a, Type b) { a.W *= b; a.X *= b; a.Y *= b; a.Z *= b; return a; }
	template<typename Type> Quaternion<Type>& operator /= (Quaternion<Type>& a, Type b) { a.W /= b; a.X /= b; a.Y /= b; a.Z /= b; return a; }

	template<typename Type> Vec<bool, 4> operator == (const Quaternion<Type>& a, const Quaternion<Type>& b) {
		Vec<bool, 4> result{};
		result.X = a.W == b.W;
		result.Y = a.X == b.X;
		result.Z = a.Y == b.Y;
		result.W = a.Z == b.Z;
		return result;
	}
	template<typename Type> Vec<bool, 4> operator == (Type a, const Quaternion<Type>& b) {
		Vec<bool, 4> result{};
		result.X = a == b.W;
		result.Y = a == b.X;
		result.Z = a == b.Y;
		result.W = a == b.Z;
		return result;
	}
	template<typename Type> Vec<bool, 4> operator == (const Quaternion<Type>& a, Type b) {
		Vec<bool, 4> result{};
		result.X = a.W == b;
		result.Y = a.X == b;
		result.Z = a.Y == b;
		result.W = a.Z == b;
		return result;
	}
	template<typename Type> Vec<bool, 4> operator != (const Quaternion<Type>& a, const Quaternion<Type>& b) { return !(a == b); }
	template<typename Type> Vec<bool, 4> operator != (Type a, const Quaternion<Type>& b) { return !(a == b); }
	template<typename Type> Vec<bool, 4> operator != (const Quaternion<Type>& a, Type b) { return !(a == b); }
	template<typename Type> Vec<bool, 4> operator < (const Quaternion<Type>& a, const Quaternion<Type>& b) {
		Vec<bool, 4> result{};
		result.X = a.W < b.W;
		result.Y = a.X < b.X;
		result.Z = a.Y < b.Y;
		result.W = a.Z < b.Z;
		return result;
	}
	template<typename Type> Vec<bool, 4> operator < (Type a, const Quaternion<Type>& b) {
		Vec<bool, 4>result{};
		result.X = a < b.W;
		result.Y = a < b.X;
		result.Z = a < b.Y;
		result.W = a < b.Z;
		return result;
	}
	template<typename Type> Vec<bool, 4> operator < (const Quaternion<Type>& a, Type b) {
		Vec<bool, 4>result{};
		result.X = a.W < b;
		result.Y = a.X < b;
		result.Z = a.Y < b;
		result.W = a.Z < b;
		return result;
	}
	template<typename Type> Vec<bool, 4> operator > (const Quaternion<Type>& a, const Quaternion<Type>& b) {
		Vec<bool, 4> result{};
		result.X = a.W > b.W;
		result.Y = a.X > b.X;
		result.Z = a.Y > b.Y;
		result.W = a.Z > b.Z;
		return result;
	}
	template<typename Type> Vec<bool, 4> operator > (Type a, const Quaternion<Type>& b) { return !(a <= b); }
	template<typename Type> Vec<bool, 4> operator > (const Quaternion<Type>& a, Type b) { return !(a <= b); }
	template<typename Type> Vec<bool, 4> operator <= (const Quaternion<Type>& a, const Quaternion<Type>& b) {
		Vec<bool, 4> result{};
		result.X = a.W <= b.W;
		result.Y = a.X <= b.X;
		result.Z = a.Y <= b.Y;
		result.W = a.Z <= b.Z;
		return result;
	}
	template<typename Type> Vec<bool, 4> operator <= (Type a, const Quaternion<Type>& b) {
		Vec<bool, 4> result{};
		result.X = a <= b.W;
		result.Y = a <= b.X;
		result.Z = a <= b.Y;
		result.W = a <= b.Z;
		return result;
	}
	template<typename Type> Vec<bool, 4> operator <= (const Quaternion<Type>& a, Type b) {
		Vec<bool, 4> result{};
		result.X = a.W <= b;
		result.Y = a.X <= b;
		result.Z = a.Y <= b;
		result.W = a.Z <= b;
		return result;
	}
	template<typename Type> Vec<bool, 4> operator >= (const Quaternion<Type>& a, const Quaternion<Type>& b) { return !(a < b); }
	template<typename Type> Vec<bool, 4> operator >= (Type a, const Quaternion<Type>& b) { return !(a < b); }
	template<typename Type> Vec<bool, 4> operator >= (const Quaternion<Type>& a, Type b) { return !(a < b); }

	template<typename Type>Quaternion<Type> operator * (const Quaternion<Type>& a, const Quaternion<Type>& b) {
		return Quaternion<Type>{
			a.W* b.W - a.X * b.X - a.Y * b.Y - a.Z * b.Z,
				a.W* b.X + a.X * b.W + a.Y * b.Z - a.Z * b.Y,
				a.W* b.Y + a.Y * b.W + a.Z * b.X - a.X * b.Z,
				a.W* b.Z + a.Z * b.W + a.X * b.Y - a.Y * b.X
		};
	}

	template<typename Type> Quaternion<Type>& operator *= (Quaternion<Type>& a, const Quaternion<Type>& b) { return a = a * b; }


	template<typename Type> Type Dot(const Quaternion<Type>& a, const Quaternion<Type>& b) { return a.W * b.W + a.X * b.X + a.Y * b.Y + a.Z * b.Z; }

	template<typename Type> Type LengthSquared(const Quaternion<Type>& a) { return Dot(a, a); }

	template<typename Type> Type Length(const Quaternion<Type>& a) { return Sqrt(LengthSquared(a)); }

	template<typename Type> Quaternion<Type> Normalize(const Quaternion<Type>& a) { return a / Length(a); }

	template<typename Type> Quaternion<Type> Conjugate(const Quaternion<Type>& a) { return Quaternion<Type>{ a.W, -a.X, -a.Y, -a.Z }; }

	template<typename Type> Quaternion<Type> Pow(const Quaternion<Type>& a, Int32 b) {
		if (b <= 0)
			return Quaternion<Type>::Identity();
		if (b == 1)
			return a;
		Quaternion<Type> oddpart{ Quaternion<Type>::Identity() };
		Quaternion<Type> evenpart{ a };
		while (b > 1) {
			if (b % 2 == 1)
				oddpart *= evenpart;

			evenpart *= evenpart;
			b /= 2;
		}
		return oddpart * evenpart;
	}

	template<typename Type> Quaternion<Type> Inverse(const Quaternion<Type>& a) { return Conjugate(a) / LengthSquared(a); }

	// Apply a normalized quat as a rotation to a Vec or point

	template <typename Type>Vec<Type, 3> ApplyQuat(const Quaternion<Type>& a, const Vec<Type, 3>& b) {
		Quaternion<Type> v{ static_cast<Type>(0), b.X, b.Y, b.Z };
		Quaternion<Type> resultQ{ a * v * Conjugate(a) };
		return Vec<Type, 3>{ resultQ.X, resultQ.Y, resultQ.Z };
	}

	template<typename Type>Vec<bool, 4> Is_Near(const Quaternion<Type>& a, const Quaternion<Type>& b, Type eps = Epsilon) {
		Vec<bool, 4> result{};
		for (Uint32 Index = 0; Index < 4; ++Index)
			result[Index] = Is_Near(a[Index], b[Index], eps);
		return result;
	}
	template<typename Type>Vec<bool, 4> Is_Near(const Quaternion<Type>& a, Type b, Type eps = Epsilon) {
		Vec<bool, 4> result{};
		for (Uint32 Index = 0; Index < 4; ++Index)
			result[Index] = Is_Near(a[Index], b, eps);
		return result;
	}
	template<typename Type>Vec<bool, 4> Is_Near(Type a, const Quaternion<Type>& b, Type eps = Epsilon) {
		Vec<bool, 4> result{};
		for (Uint32 Index = 0; Index < 4; ++Index)
			result[Index] = Is_Near(a, b[Index], eps);
		return result;
	}

	template<typename Type>Vec<bool, 4> Is_Finite(const Quaternion<Type>& a) {
		Vec<bool, 4> result{};
		for (Uint32 Index = 0; Index < 4; ++Index)
			result[Index] = Is_Finite(a[Index]);
		return result;
	}

	template<typename Type> Quaternion<Type> Select(const Vec<bool, 4>& cond, const Quaternion<Type>& a, const Quaternion<Type>& b) {
		Quaternion<Type> result;
		for (Uint32 Index = 0; Index < 4; ++Index)
			result[Index] = cond[Index] ? a[Index] : b[Index];
		return result;
	}

	template<typename Type>Quaternion<Type> Min(const Quaternion<Type>& a, const Quaternion<Type>& b) { return Select(a < b, a, b); }

	template<typename Type>Quaternion<Type> Max(const Quaternion<Type>& a, const Quaternion<Type>& b) { return Select(a < b, b, a); }

	template<typename Type>Quaternion<Type> Abs(const Quaternion<Type>& a) { return Select(a < static_cast<Type>(0), -a, a); }

	template<typename Type>Quaternion<Type> Saturate(const Quaternion<Type>& value) { return Clamp(value, Quaternion<Type>{ static_cast<Type>(0) }, Quaternion<Type>{ static_cast<Type>(1) }); }

	template<typename Type>Type MinComponent(const Quaternion<Type>& a) {
		Type result{ a[0] };
		for (Uint32 Index = 1; Index < 4; ++Index)
			result = Min(result, a[Index]);
	}

	template<typename Type>Type MaxComponent(const Quaternion<Type>& a) {
		Type result{ a[0] };
		for (Uint32 Index = 1; Index < 4; ++Index)
			result = Max(result, a[Index]);
		return result;
	}

	template<typename Type>Quaternion<Type> RotationQuat(const Vec<Type, 3>& axis, Type radians) {// Note: assumes axis is normalized
		Type sinHalfTheta{ Sin(static_cast<Type>(0.5) * radians) };
		Type cosHalfTheta{ Cos(static_cast<Type>(0.5) * radians) };

		return Quaternion<Type>{ cosHalfTheta, axis* sinHalfTheta };
	}
	template<typename Type>Quaternion<Type> RotationQuat(const Vec<Type, 3>& euler) {
		Type sinHalfX{ Sin(static_cast<Type>(0.5) * euler.X) };
		Type cosHalfX{ Cos(static_cast<Type>(0.5) * euler.X) };
		Type sinHalfY{ Sin(static_cast<Type>(0.5) * euler.Y) };
		Type cosHalfY{ Cos(static_cast<Type>(0.5) * euler.Y) };
		Type sinHalfZ{ Sin(static_cast<Type>(0.5) * euler.Z) };
		Type cosHalfZ{ Cos(static_cast<Type>(0.5) * euler.Z) };

		Quaternion<Type> quatX{ cosHalfX, sinHalfX,				static_cast<Type>(0),	static_cast<Type>(0) };
		Quaternion<Type> quatY{ cosHalfY, static_cast<Type>(0),	sinHalfY,				static_cast<Type>(0) };
		Quaternion<Type> quatZ{ cosHalfZ, static_cast<Type>(0), static_cast<Type>(0),	sinHalfZ };

		// Note: multiplication order for quats is like column-Vec convention
		return quatZ * quatY * quatX;
	}

	template<typename Type> Quaternion<Type> Slerp(const Quaternion<Type>& a, const Quaternion<Type>& b, Type u) {
		// shortest path on 4D sphere
		Type sign{ static_cast<Type>(1) };
		Type fa{ static_cast<Type>(1) - u };
		Type fb{ u };
		Type dp{ Dot(a, b) };

		if (dp < static_cast<Type>(0)) {
			sign = static_cast<Type>(-1);
			dp = -dp;
		}
		if (static_cast<Type>(1) - dp > static_cast<Type>(0.001)) {
			Type theta{ Acos(dp) };
			fa = Sin(theta * fa) / Sin(theta);
			fb = Sin(theta * fb) / Sin(theta);
		}

		return fa * a + sign * fb * b;
	}

	template<typename Type> void DecomposeAffine(const Affine<Type, 3>& transform, Vec<Type, 3>* pTranslation, Quaternion<Type>* pRotation, Vec<Type, 3>* pScaling) {
		if (nullptr != pTranslation)
			*pTranslation = transform.m_Translation;

		Vec<Type, 3> col0{ transform.m_Linear.Col(0) };
		Vec<Type, 3> col1{ transform.m_Linear.Col(1) };
		Vec<Type, 3> col2{ transform.m_Linear.Col(2) };

		Vec<Type, 3> scaling;
		scaling.X = Length(col0);
		scaling.Y = Length(col1);
		scaling.Z = Length(col2);
		if (scaling.X > 0.f)
			col0 /= scaling.X;
		if (scaling.Y > 0.f)
			col1 /= scaling.Y;
		if (scaling.Z > 0.f)
			col2 /= scaling.Z;

		Vec<Type, 3> zAxis{ Cross(col0, col1) };
		if (Dot(zAxis, col2) < static_cast<Type>(0)) {
			scaling.X = -scaling.X;
			col0 = -col0;
		}

		if (nullptr != pScaling)
			*pScaling = scaling;

		if (nullptr != pRotation) {
			// https://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/
			Quaternion<Type> rotation;
			rotation.W = Sqrt(Max(static_cast<Type>(0), static_cast<Type>(1) + col0.X + col1.Y + col2.Z)) * static_cast<Type>(0.5);
			rotation.X = Sqrt(Max(static_cast<Type>(0), static_cast<Type>(1) + col0.X - col1.Y - col2.Z)) * static_cast<Type>(0.5);
			rotation.Y = Sqrt(Max(static_cast<Type>(0), static_cast<Type>(1) - col0.X + col1.Y - col2.Z)) * static_cast<Type>(0.5);
			rotation.Z = Sqrt(Max(static_cast<Type>(0), static_cast<Type>(1) - col0.X - col1.Y + col2.Z)) * static_cast<Type>(0.5);

			rotation.X = std::copysign(rotation.X, col2.Y - col1.Z);//TODO :
			rotation.Y = std::copysign(rotation.Y, col0.Z - col2.X);
			rotation.Z = std::copysign(rotation.Z, col1.X - col0.Y);
			
			*pRotation = rotation;
		}
	}

}