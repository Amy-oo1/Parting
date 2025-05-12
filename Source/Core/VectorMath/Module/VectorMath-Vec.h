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

#include "Core/Algorithm/Module/Algorithm.h"

#endif // PARTING_MODULE_BUILD

namespace Math {
#define VECTOR_MEMBERS(Type, N) \
			static constexpr Uint32 DIM = N; \
			using Array_T = Type(&)[N];\
			using ConstArray_T = const Type(&)[N];\
			Vec(void)= default;\
			Vec(const Type* VaulePtr) { for(Uint32 Index = 0; Index < N; ++Index) this->Data()[Index] = VaulePtr[Index]; } \
			operator Array_T (void) { return reinterpret_cast<Array_T>(*this->Data()); } \
			operator ConstArray_T (void) const { return reinterpret_cast<ConstArray_T>(*this->Data()); } \
			Type& operator [] (Uint32 Index) { return this->Data()[Index]; } \
			const Type& operator [] (Uint32 Index) const { return this->Data()[Index]; }

#undef VECTOR_MEMBERS

#define DEFINE_UNARY_OPERATOR(OP) \
		template<typename Type> constexpr Vec<Type, 2> operator OP (const Vec<Type, 2>& a) { return Vec<Type, 2>{OP a.X, OP a.Y}; } \
		template<typename Type> constexpr Vec<Type, 3> operator OP (const Vec<Type, 3>& a) { return Vec<Type, 3>{OP a.X, OP a.Y, OP a.Z}; } \
		template<typename Type> constexpr Vec<Type, 4> operator OP (const Vec<Type, 4>& a) { return Vec<Type, 4>{OP a.X, OP a.Y, OP a.Z, OP a.W}; }

#undef DEFINE_UNARY_OPERATOR

#define DEFINE_BINARY_OPERATORS(OP) \
		template<typename Type> constexpr Vec<Type, 2> operator OP (const Vec<Type, 2>& a, const Vec<Type, 2>& b) { return Vec<Type, 2>{a.X OP b.X, a.Y OP b.Y}; } \
		template<typename Type> constexpr Vec<Type, 3> operator OP (const Vec<Type, 3>& a, const Vec<Type, 3>& b) { return Vec<Type, 3>{a.X OP b.X, a.Y OP b.Y, a.Z OP b.Z}; } \
		template<typename Type> constexpr Vec<Type, 4> operator OP (const Vec<Type, 4>& a, const Vec<Type, 4>& b) { return Vec<Type, 4>{a.X OP b.X, a.Y OP b.Y, a.Z OP b.Z, a.W OP b.W}; } \
		template<typename Type> constexpr Vec<Type, 2> operator OP (Type a, const Vec<Type, 2>& b) { return Vec<Type, 2>{a OP b.X, a OP b.Y}; } \
		template<typename Type> constexpr Vec<Type, 3> operator OP (Type a, const Vec<Type, 3>& b) { return Vec<Type, 3>{a OP b.X, a OP b.Y, a OP b.Z}; } \
		template<typename Type> constexpr Vec<Type, 4> operator OP (Type a, const Vec<Type, 4>& b) { return Vec<Type, 4>{a OP b.X, a OP b.Y, a OP b.Z, a OP b.W}; } \
		template<typename Type> constexpr Vec<Type, 2> operator OP (const Vec<Type, 2>& a, Type b) { return Vec<Type, 2>{a.X OP b, a.Y OP b}; } \
		template<typename Type> constexpr Vec<Type, 3> operator OP (const Vec<Type, 3>& a, Type b) { return Vec<Type, 3>{a.X OP b, a.Y OP b, a.Z OP b}; } \
		template<typename Type> constexpr Vec<Type, 4> operator OP (const Vec<Type, 4>& a, Type b) { return Vec<Type, 4>{a.X OP b, a.Y OP b, a.Z OP b, a.W OP b}; } 

#undef DEFINE_BINARY_OPERATORS

#define DEFINE_INPLACE_OPERATORS(OP) \
		template<typename Type> Vec<Type, 2> & operator OP (Vec<Type, 2>& a, const Vec<Type, 2>& b) { a.X OP b.X; a.Y OP b.Y; return a; } \
		template<typename Type> Vec<Type, 3> & operator OP (Vec<Type, 3>& a, const Vec<Type, 3>& b) { a.X OP b.X; a.Y OP b.Y; a.Z OP b.Z; return a; } \
		template<typename Type> Vec<Type, 4> & operator OP (Vec<Type, 4>& a, const Vec<Type, 4>& b) { a.X OP b.X; a.Y OP b.Y; a.Z OP b.Z; a.W OP b.W; return a; } \
		template<typename Type> Vec<Type, 2> & operator OP (Vec<Type, 2>& a, Type b) { a.X OP b; a.Y OP b; return a; } \
		template<typename Type> Vec<Type, 3> & operator OP (Vec<Type, 3>& a, Type b) { a.X OP b; a.Y OP b; a.Z OP b; return a; } \
		template<typename Type> Vec<Type, 4> & operator OP (Vec<Type, 4>& a, Type b) { a.X OP b; a.Y OP b; a.Z OP b; a.W OP b; return a; }

#undef DEFINE_INPLACE_OPERATORS

#define DEFINE_RELATIONAL_OPERATORS(OP) \
		template<typename Type> constexpr Vec<bool, 2> operator OP (const Vec<Type, 2>& a, const Vec<Type, 2>& b) { return Vec<bool, 2>{a.X OP b.X, a.Y OP b.Y}; } \
		template<typename Type> constexpr Vec<bool, 3> operator OP (const Vec<Type, 3>& a, const Vec<Type, 3>& b) { return Vec<bool, 3>{a.X OP b.X, a.Y OP b.Y, a.Z OP b.Z}; } \
		template<typename Type> constexpr Vec<bool, 4> operator OP (const Vec<Type, 4>& a, const Vec<Type, 4>& b) { return Vec<bool, 4>{a.X OP b.X, a.Y OP b.Y, a.Z OP b.Z, a.W OP b.W}; } \
		template<typename Type> constexpr Vec<bool, 2> operator OP (Type a, const Vec<Type, 2>& b) { return Vec<bool, 2>{a OP b.X, a OP b.Y}; } \
		template<typename Type> constexpr Vec<bool, 3> operator OP (Type a, const Vec<Type, 3>& b) { return Vec<bool, 3>{a OP b.X, a OP b.Y, a OP b.Z}; } \
		template<typename Type> constexpr Vec<bool, 4> operator OP (Type a, const Vec<Type, 4>& b) { return Vec<bool, 4>{a OP b.X, a OP b.Y, a OP b.Z, a OP b.W}; } \
		template<typename Type> constexpr Vec<bool, 2> operator OP (const Vec<Type, 2>& a, Type b) { return Vec<bool, 2>{a.X OP b, a.Y OP b}; } \
		template<typename Type> constexpr Vec<bool, 3> operator OP (const Vec<Type, 3>& a, Type b) { return Vec<bool, 3>{a.X OP b, a.Y OP b, a.Z OP b}; } \
		template<typename Type> constexpr Vec<bool, 4> operator OP (const Vec<Type, 4>& a, Type b) { return Vec<bool, 4>{a.X OP b, a.Y OP b, a.Z OP b, a.W OP b}; } 

#undef DEFINE_RELATIONAL_OPERATORS




	template <typename Type, Uint32 N>
	struct Vec final {
		static_assert(N > 4);

		using Array_T = Type(&)[N];
		using ConstArray_T = const Type(&)[N];

		static constexpr Uint32 DIM = N;
		Type m_Data[N];

		Vec(void) = default;
		Vec(Type Value) { for (Uint32 Index = 0; Index < N; ++Index) this->m_Data[Index] = Value; }
		Vec(const Type* VaulePtr) { for (Uint32 Index = 0; Index < N; ++Index) this->Data()[Index] = VaulePtr[Index]; }
		template<typename OtherType> explicit Vec(const Vec<OtherType, N>& other) { for (Uint32 Index = 0; Index < N; ++Index)  this->m_Data[Index] = static_cast<OtherType>(other.m_Data[Index]); }

		operator Array_T (void) { return reinterpret_cast<Array_T>(*this->Data()); }
		operator ConstArray_T (void) const { return reinterpret_cast<ConstArray_T>(*this->Data()); }
		Type& operator [] (Uint32 Index) { return this->Data()[Index]; }
		const Type& operator [] (Uint32 Index) const { return this->Data()[Index]; }

		Type* Data(void) { return this->m_Data; }
		const Type* Data(void) const { return this->m_Data; }
	};

	template <typename Type>
	struct Vec<Type, 2> {
		using Array_T = Type(&)[2];
		using ConstArray_T = const Type(&)[2];

		static constexpr Uint32 DIM = 2;
		Type X, Y;

		constexpr Vec(void) = default;
		constexpr Vec(Type Scale) : X{ Scale }, Y{ Scale } {}
		constexpr Vec(Type _x, Type _y) : X{ _x }, Y{ _y } {}
		template<typename OtherType> explicit constexpr Vec(const Vec<OtherType, 2>& other) : X{ static_cast<OtherType>(other.X) }, Y{ static_cast<OtherType>(other.Y) } {}
		constexpr Vec(const Vec<Type, 3>& other) : X{ other.X }, Y{ other.Y } {}
		constexpr Vec(const Vec<Type, 4>& other) : X{ other.X }, Y{ other.Y } {}
		Vec(const Type* VaulePtr) { for (Uint32 Index = 0; Index < 2; ++Index) this->Data()[Index] = VaulePtr[Index]; }

		operator Array_T (void) { return reinterpret_cast<Array_T>(*this->Data()); }
		operator ConstArray_T (void) const { return reinterpret_cast<ConstArray_T>(*this->Data()); }
		Type& operator [] (Uint32 Index) { return this->Data()[Index]; }
		const Type& operator [] (Uint32 Index) const { return this->Data()[Index]; }

		Type* Data(void) { return &this->X; }
		const Type* Data(void) const { return &this->Y; }

		constexpr static Vec Zero(void) { return Vec{ static_cast<Type>(0) }; }
	};

	template <typename Type>
	struct Vec<Type, 3> {
		using Array_T = Type(&)[3];
		using ConstArray_T = const Type(&)[3];

		static constexpr Uint32 DIM = 3;
		Type X, Y, Z;

		constexpr Vec(void) = default;
		constexpr Vec(Type Scale) : X{ Scale }, Y{ Scale }, Z{ Scale } {}
		constexpr Vec(Type _x, Type _y, Type _z) : X{ _x }, Y{ _y }, Z{ _z } {}
		constexpr Vec(const Vec<Type, 2>& xy, Type _z) : X{ xy.X }, Y{ xy.Y }, Z{ _z } {}
		constexpr Vec(const Vec<Type, 4>& Value) : X{ Value.X }, Y{ Value.Y }, Z{ Value.Z } {}
		template<typename OtherType> explicit constexpr Vec(const Vec<OtherType, 3>& other) : X{ static_cast<Type>(other.X) }, Y{ static_cast<Type>(other.Y) }, Z{ static_cast<Type>(other.Z) } {}
		Vec(const Type* VaulePtr) { for (Uint32 Index = 0; Index < 3; ++Index) this->Data()[Index] = VaulePtr[Index]; }

		operator Array_T (void) { return reinterpret_cast<Array_T>(*this->Data()); }
		operator ConstArray_T (void) const { return reinterpret_cast<ConstArray_T>(*this->Data()); }
		Type& operator [] (Uint32 Index) { return this->Data()[Index]; } const Type& operator [] (Uint32 Index) const { return this->Data()[Index]; }

		Vec<Type, 2>& XY(void) { return *reinterpret_cast<Vec<Type, 2>*>(&this->X); }
		const Vec<Type, 2>& XY(void) const { return *reinterpret_cast<const Vec<Type, 2>*>(&this->X); }

		Type* Data(void) { return  &this->X; }
		const Type* Data(void) const { return  &this->X; }

		constexpr static Vec Zero(void) { return Vec{ static_cast<Type>(0) }; }


	};

	template <typename Type>
	struct Vec<Type, 4> {
		using Array_T = Type(&)[4];
		using ConstArray_T = const Type(&)[4];

		static constexpr Uint32 DIM = 4;
		Type X, Y, Z, W;

		Vec(void) = default;
		constexpr Vec(Type Scale) : X{ Scale }, Y{ Scale }, Z{ Scale }, W{ Scale } {}
		constexpr Vec(Type _x, Type _y, Type _z, Type _w) : X{ _x }, Y{ _y }, Z{ _z }, W{ _w } {}
		constexpr Vec(const Vec<Type, 2>& xy, Type _z, Type _w) : X{ xy.X }, Y{ xy.Y }, Z{ _z }, W{ _w } {}
		constexpr Vec(const Vec<Type, 2>& xy, const Vec<Type, 2>& zw) : X{ xy.X }, Y{ xy.Y }, Z{ zw.X }, W{ zw.Y } {}
		constexpr Vec(const Vec<Type, 3>& xyz, Type _w) : X{ xyz.X }, Y{ xyz.Y }, Z{ xyz.Z }, W{ _w } {}
		template<typename OtherType> explicit constexpr Vec(const Vec<OtherType, 4>& other) : X{ static_cast<Type>(other.X) }, Y{ static_cast<Type>(other.Y) }, Z{ static_cast<Type>(other.Z) }, W{ static_cast<Type>(other.W) } {}
		Vec(const Type* VaulePtr) { for (Uint32 Index = 0; Index < 4; ++Index) this->Data()[Index] = VaulePtr[Index]; }

		operator Array_T (void) { return reinterpret_cast<Array_T>(*this->Data()); }
		operator ConstArray_T (void) const { return reinterpret_cast<ConstArray_T>(*this->Data()); }
		Type& operator [] (Uint32 Index) { return this->Data()[Index]; }
		const Type& operator [] (Uint32 Index) const { return this->Data()[Index]; }

		Type* Data(void) { return &this->X; }
		const Type* Data(void) const { return &this->X; }

		Vec<Type, 2>& XY(void) { return *reinterpret_cast<Vec<Type, 2>*>(&this->X); }
		const Vec<Type, 2>& XY(void) const { return *reinterpret_cast<const Vec<Type, 2>*>(&this->X); }
		Vec<Type, 2>& ZW(void) { return *reinterpret_cast<Vec<Type, 2>*>(&this->Z); }
		const Vec<Type, 2>& ZW(void) const { return *reinterpret_cast<const Vec<Type, 2>*>(&this->Z); }
		Vec<Type, 3>& XYZ(void) { return *reinterpret_cast<Vec<Type, 3>*>(&this->X); }
		const Vec<Type, 3>& XYZ(void) const { return *reinterpret_cast<const Vec<Type, 3>*>(&this->X); }

		constexpr static Vec zero(void) { return Vec{ static_cast<Type>(0) }; }


	};

	template<typename Type> constexpr Vec<Type, 2> operator + (const Vec<Type, 2>& a) { return Vec<Type, 2>{+a.X, +a.Y}; }
	template<typename Type> constexpr Vec<Type, 3> operator + (const Vec<Type, 3>& a) { return Vec<Type, 3>{+a.X, +a.Y, +a.Z}; }
	template<typename Type> constexpr Vec<Type, 4> operator + (const Vec<Type, 4>& a) { return Vec<Type, 4>{+a.X, +a.Y, +a.Z, +a.W}; }
	template<typename Type> constexpr Vec<Type, 2> operator - (const Vec<Type, 2>& a) { return Vec<Type, 2>{-a.X, -a.Y}; }
	template<typename Type> constexpr Vec<Type, 3> operator - (const Vec<Type, 3>& a) { return Vec<Type, 3>{-a.X, -a.Y, -a.Z}; }
	template<typename Type> constexpr Vec<Type, 4> operator - (const Vec<Type, 4>& a) { return Vec<Type, 4>{-a.X, -a.Y, -a.Z, -a.W}; }
	template<typename Type> constexpr Vec<Type, 2> operator ! (const Vec<Type, 2>& a) { return Vec<Type, 2>{!a.X, !a.Y}; }
	template<typename Type> constexpr Vec<Type, 3> operator ! (const Vec<Type, 3>& a) { return Vec<Type, 3>{!a.X, !a.Y, !a.Z}; }
	template<typename Type> constexpr Vec<Type, 4> operator ! (const Vec<Type, 4>& a) { return Vec<Type, 4>{!a.X, !a.Y, !a.Z, !a.W}; }



	template<typename Type> constexpr Vec<Type, 2> operator + (const Vec<Type, 2>& a, const Vec<Type, 2>& b) { return Vec<Type, 2>{a.X + b.X, a.Y + b.Y}; }
	template<typename Type> constexpr Vec<Type, 3> operator + (const Vec<Type, 3>& a, const Vec<Type, 3>& b) { return Vec<Type, 3>{a.X + b.X, a.Y + b.Y, a.Z + b.Z}; }
	template<typename Type> constexpr Vec<Type, 4> operator + (const Vec<Type, 4>& a, const Vec<Type, 4>& b) { return Vec<Type, 4>{a.X + b.X, a.Y + b.Y, a.Z + b.Z, a.W + b.W}; }
	template<typename Type> constexpr Vec<Type, 2> operator + (Type a, const Vec<Type, 2>& b) { return Vec<Type, 2>{a + b.X, a + b.Y}; }
	template<typename Type> constexpr Vec<Type, 3> operator + (Type a, const Vec<Type, 3>& b) { return Vec<Type, 3>{a + b.X, a + b.Y, a + b.Z}; }
	template<typename Type> constexpr Vec<Type, 4> operator + (Type a, const Vec<Type, 4>& b) { return Vec<Type, 4>{a + b.X, a + b.Y, a + b.Z, a + b.W}; }
	template<typename Type> constexpr Vec<Type, 2> operator + (const Vec<Type, 2>& a, Type b) { return Vec<Type, 2>{a.X + b, a.Y + b}; }
	template<typename Type> constexpr Vec<Type, 3> operator + (const Vec<Type, 3>& a, Type b) { return Vec<Type, 3>{a.X + b, a.Y + b, a.Z + b}; }
	template<typename Type> constexpr Vec<Type, 4> operator + (const Vec<Type, 4>& a, Type b) { return Vec<Type, 4>{a.X + b, a.Y + b, a.Z + b, a.W + b}; }
	template<typename Type> constexpr Vec<Type, 2> operator - (const Vec<Type, 2>& a, const Vec<Type, 2>& b) { return Vec<Type, 2>{a.X - b.X, a.Y - b.Y}; }
	template<typename Type> constexpr Vec<Type, 3> operator - (const Vec<Type, 3>& a, const Vec<Type, 3>& b) { return Vec<Type, 3>{a.X - b.X, a.Y - b.Y, a.Z - b.Z}; }
	template<typename Type> constexpr Vec<Type, 4> operator - (const Vec<Type, 4>& a, const Vec<Type, 4>& b) { return Vec<Type, 4>{a.X - b.X, a.Y - b.Y, a.Z - b.Z, a.W - b.W}; }
	template<typename Type> constexpr Vec<Type, 2> operator - (Type a, const Vec<Type, 2>& b) { return Vec<Type, 2>{a - b.X, a - b.Y}; }
	template<typename Type> constexpr Vec<Type, 3> operator - (Type a, const Vec<Type, 3>& b) { return Vec<Type, 3>{a - b.X, a - b.Y, a - b.Z}; }
	template<typename Type> constexpr Vec<Type, 4> operator - (Type a, const Vec<Type, 4>& b) { return Vec<Type, 4>{a - b.X, a - b.Y, a - b.Z, a - b.W}; }
	template<typename Type> constexpr Vec<Type, 2> operator - (const Vec<Type, 2>& a, Type b) { return Vec<Type, 2>{a.X - b, a.Y - b}; }
	template<typename Type> constexpr Vec<Type, 3> operator - (const Vec<Type, 3>& a, Type b) { return Vec<Type, 3>{a.X - b, a.Y - b, a.Z - b}; }
	template<typename Type> constexpr Vec<Type, 4> operator - (const Vec<Type, 4>& a, Type b) { return Vec<Type, 4>{a.X - b, a.Y - b, a.Z - b, a.W - b}; }
	template<typename Type> constexpr Vec<Type, 2> operator * (const Vec<Type, 2>& a, const Vec<Type, 2>& b) { return Vec<Type, 2>{a.X* b.X, a.Y* b.Y}; }
	template<typename Type> constexpr Vec<Type, 3> operator * (const Vec<Type, 3>& a, const Vec<Type, 3>& b) { return Vec<Type, 3>{a.X* b.X, a.Y* b.Y, a.Z* b.Z}; }
	template<typename Type> constexpr Vec<Type, 4> operator * (const Vec<Type, 4>& a, const Vec<Type, 4>& b) { return Vec<Type, 4>{a.X* b.X, a.Y* b.Y, a.Z* b.Z, a.W* b.W}; }
	template<typename Type> constexpr Vec<Type, 2> operator * (Type a, const Vec<Type, 2>& b) { return Vec<Type, 2>{a* b.X, a* b.Y}; }
	template<typename Type> constexpr Vec<Type, 3> operator * (Type a, const Vec<Type, 3>& b) { return Vec<Type, 3>{a* b.X, a* b.Y, a* b.Z}; }
	template<typename Type> constexpr Vec<Type, 4> operator * (Type a, const Vec<Type, 4>& b) { return Vec<Type, 4>{a* b.X, a* b.Y, a* b.Z, a* b.W}; }
	template<typename Type> constexpr Vec<Type, 2> operator * (const Vec<Type, 2>& a, Type b) { return Vec<Type, 2>{a.X* b, a.Y* b}; }
	template<typename Type> constexpr Vec<Type, 3> operator * (const Vec<Type, 3>& a, Type b) { return Vec<Type, 3>{a.X* b, a.Y* b, a.Z* b}; }
	template<typename Type> constexpr Vec<Type, 4> operator * (const Vec<Type, 4>& a, Type b) { return Vec<Type, 4>{a.X* b, a.Y* b, a.Z* b, a.W* b}; }
	template<typename Type> constexpr Vec<Type, 2> operator / (const Vec<Type, 2>& a, const Vec<Type, 2>& b) { return Vec<Type, 2>{a.X / b.X, a.Y / b.Y}; }
	template<typename Type> constexpr Vec<Type, 3> operator / (const Vec<Type, 3>& a, const Vec<Type, 3>& b) { return Vec<Type, 3>{a.X / b.X, a.Y / b.Y, a.Z / b.Z}; }
	template<typename Type> constexpr Vec<Type, 4> operator / (const Vec<Type, 4>& a, const Vec<Type, 4>& b) { return Vec<Type, 4>{a.X / b.X, a.Y / b.Y, a.Z / b.Z, a.W / b.W}; }
	template<typename Type> constexpr Vec<Type, 2> operator / (Type a, const Vec<Type, 2>& b) { return Vec<Type, 2>{a / b.X, a / b.Y}; }
	template<typename Type> constexpr Vec<Type, 3> operator / (Type a, const Vec<Type, 3>& b) { return Vec<Type, 3>{a / b.X, a / b.Y, a / b.Z}; }
	template<typename Type> constexpr Vec<Type, 4> operator / (Type a, const Vec<Type, 4>& b) { return Vec<Type, 4>{a / b.X, a / b.Y, a / b.Z, a / b.W}; }
	template<typename Type> constexpr Vec<Type, 2> operator / (const Vec<Type, 2>& a, Type b) { return Vec<Type, 2>{a.X / b, a.Y / b}; }
	template<typename Type> constexpr Vec<Type, 3> operator / (const Vec<Type, 3>& a, Type b) { return Vec<Type, 3>{a.X / b, a.Y / b, a.Z / b}; }
	template<typename Type> constexpr Vec<Type, 4> operator / (const Vec<Type, 4>& a, Type b) { return Vec<Type, 4>{a.X / b, a.Y / b, a.Z / b, a.W / b}; }
	template<typename Type> constexpr Vec<Type, 2> operator & (const Vec<Type, 2>& a, const Vec<Type, 2>& b) { return Vec<Type, 2>{a.X& b.X, a.Y& b.Y}; }
	template<typename Type> constexpr Vec<Type, 3> operator & (const Vec<Type, 3>& a, const Vec<Type, 3>& b) { return Vec<Type, 3>{a.X& b.X, a.Y& b.Y, a.Z& b.Z}; }
	template<typename Type> constexpr Vec<Type, 4> operator & (const Vec<Type, 4>& a, const Vec<Type, 4>& b) { return Vec<Type, 4>{a.X& b.X, a.Y& b.Y, a.Z& b.Z, a.W& b.W}; }
	template<typename Type> constexpr Vec<Type, 2> operator & (Type a, const Vec<Type, 2>& b) { return Vec<Type, 2>{a& b.X, a& b.Y}; }
	template<typename Type> constexpr Vec<Type, 3> operator & (Type a, const Vec<Type, 3>& b) { return Vec<Type, 3>{a& b.X, a& b.Y, a& b.Z}; }
	template<typename Type> constexpr Vec<Type, 4> operator & (Type a, const Vec<Type, 4>& b) { return Vec<Type, 4>{a& b.X, a& b.Y, a& b.Z, a& b.W}; }
	template<typename Type> constexpr Vec<Type, 2> operator & (const Vec<Type, 2>& a, Type b) { return Vec<Type, 2>{a.X& b, a.Y& b}; }
	template<typename Type> constexpr Vec<Type, 3> operator & (const Vec<Type, 3>& a, Type b) { return Vec<Type, 3>{a.X& b, a.Y& b, a.Z& b}; }
	template<typename Type> constexpr Vec<Type, 4> operator & (const Vec<Type, 4>& a, Type b) { return Vec<Type, 4>{a.X& b, a.Y& b, a.Z& b, a.W& b}; }
	template<typename Type> constexpr Vec<Type, 2> operator | (const Vec<Type, 2>& a, const Vec<Type, 2>& b) { return Vec<Type, 2>{a.X | b.X, a.Y | b.Y}; }
	template<typename Type> constexpr Vec<Type, 3> operator | (const Vec<Type, 3>& a, const Vec<Type, 3>& b) { return Vec<Type, 3>{a.X | b.X, a.Y | b.Y, a.Z | b.Z}; }
	template<typename Type> constexpr Vec<Type, 4> operator | (const Vec<Type, 4>& a, const Vec<Type, 4>& b) { return Vec<Type, 4>{a.X | b.X, a.Y | b.Y, a.Z | b.Z, a.W | b.W}; }
	template<typename Type> constexpr Vec<Type, 2> operator | (Type a, const Vec<Type, 2>& b) { return Vec<Type, 2>{a | b.X, a | b.Y}; }
	template<typename Type> constexpr Vec<Type, 3> operator | (Type a, const Vec<Type, 3>& b) { return Vec<Type, 3>{a | b.X, a | b.Y, a | b.Z}; }
	template<typename Type> constexpr Vec<Type, 4> operator | (Type a, const Vec<Type, 4>& b) { return Vec<Type, 4>{a | b.X, a | b.Y, a | b.Z, a | b.W}; }
	template<typename Type> constexpr Vec<Type, 2> operator | (const Vec<Type, 2>& a, Type b) { return Vec<Type, 2>{a.X | b, a.Y | b}; }
	template<typename Type> constexpr Vec<Type, 3> operator | (const Vec<Type, 3>& a, Type b) { return Vec<Type, 3>{a.X | b, a.Y | b, a.Z | b}; }
	template<typename Type> constexpr Vec<Type, 4> operator | (const Vec<Type, 4>& a, Type b) { return Vec<Type, 4>{a.X | b, a.Y | b, a.Z | b, a.W | b}; }
	template<typename Type> constexpr Vec<Type, 2> operator ^ (const Vec<Type, 2>& a, const Vec<Type, 2>& b) { return Vec<Type, 2>{a.X^ b.X, a.Y^ b.Y}; }
	template<typename Type> constexpr Vec<Type, 3> operator ^ (const Vec<Type, 3>& a, const Vec<Type, 3>& b) { return Vec<Type, 3>{a.X^ b.X, a.Y^ b.Y, a.Z^ b.Z}; }
	template<typename Type> constexpr Vec<Type, 4> operator ^ (const Vec<Type, 4>& a, const Vec<Type, 4>& b) { return Vec<Type, 4>{a.X^ b.X, a.Y^ b.Y, a.Z^ b.Z, a.W^ b.W}; }
	template<typename Type> constexpr Vec<Type, 2> operator ^ (Type a, const Vec<Type, 2>& b) { return Vec<Type, 2>{a^ b.X, a^ b.Y}; }
	template<typename Type> constexpr Vec<Type, 3> operator ^ (Type a, const Vec<Type, 3>& b) { return Vec<Type, 3>{a^ b.X, a^ b.Y, a^ b.Z}; }
	template<typename Type> constexpr Vec<Type, 4> operator ^ (Type a, const Vec<Type, 4>& b) { return Vec<Type, 4>{a^ b.X, a^ b.Y, a^ b.Z, a^ b.W}; }
	template<typename Type> constexpr Vec<Type, 2> operator ^ (const Vec<Type, 2>& a, Type b) { return Vec<Type, 2>{a.X^ b, a.Y^ b}; }
	template<typename Type> constexpr Vec<Type, 3> operator ^ (const Vec<Type, 3>& a, Type b) { return Vec<Type, 3>{a.X^ b, a.Y^ b, a.Z^ b}; }
	template<typename Type> constexpr Vec<Type, 4> operator ^ (const Vec<Type, 4>& a, Type b) { return Vec<Type, 4>{a.X^ b, a.Y^ b, a.Z^ b, a.W^ b}; }



	template<typename Type> Vec<Type, 2>& operator += (Vec<Type, 2>& a, const Vec<Type, 2>& b) { a.X += b.X; a.Y += b.Y; return a; }
	template<typename Type> Vec<Type, 3>& operator += (Vec<Type, 3>& a, const Vec<Type, 3>& b) { a.X += b.X; a.Y += b.Y; a.Z += b.Z; return a; }
	template<typename Type> Vec<Type, 4>& operator += (Vec<Type, 4>& a, const Vec<Type, 4>& b) { a.X += b.X; a.Y += b.Y; a.Z += b.Z; a.W += b.W; return a; }
	template<typename Type> Vec<Type, 2>& operator += (Vec<Type, 2>& a, Type b) { a.X += b; a.Y += b; return a; }
	template<typename Type> Vec<Type, 3>& operator += (Vec<Type, 3>& a, Type b) { a.X += b; a.Y += b; a.Z += b; return a; }
	template<typename Type> Vec<Type, 4>& operator += (Vec<Type, 4>& a, Type b) { a.X += b; a.Y += b; a.Z += b; a.W += b; return a; }
	template<typename Type> Vec<Type, 2>& operator -= (Vec<Type, 2>& a, const Vec<Type, 2>& b) { a.X -= b.X; a.Y -= b.Y; return a; }
	template<typename Type> Vec<Type, 3>& operator -= (Vec<Type, 3>& a, const Vec<Type, 3>& b) { a.X -= b.X; a.Y -= b.Y; a.Z -= b.Z; return a; }
	template<typename Type> Vec<Type, 4>& operator -= (Vec<Type, 4>& a, const Vec<Type, 4>& b) { a.X -= b.X; a.Y -= b.Y; a.Z -= b.Z; a.W -= b.W; return a; }
	template<typename Type> Vec<Type, 2>& operator -= (Vec<Type, 2>& a, Type b) { a.X -= b; a.Y -= b; return a; }
	template<typename Type> Vec<Type, 3>& operator -= (Vec<Type, 3>& a, Type b) { a.X -= b; a.Y -= b; a.Z -= b; return a; }
	template<typename Type> Vec<Type, 4>& operator -= (Vec<Type, 4>& a, Type b) { a.X -= b; a.Y -= b; a.Z -= b; a.W -= b; return a; }
	template<typename Type> Vec<Type, 2>& operator *= (Vec<Type, 2>& a, const Vec<Type, 2>& b) { a.X *= b.X; a.Y *= b.Y; return a; }
	template<typename Type> Vec<Type, 3>& operator *= (Vec<Type, 3>& a, const Vec<Type, 3>& b) { a.X *= b.X; a.Y *= b.Y; a.Z *= b.Z; return a; }
	template<typename Type> Vec<Type, 4>& operator *= (Vec<Type, 4>& a, const Vec<Type, 4>& b) { a.X *= b.X; a.Y *= b.Y; a.Z *= b.Z; a.W *= b.W; return a; }
	template<typename Type> Vec<Type, 2>& operator *= (Vec<Type, 2>& a, Type b) { a.X *= b; a.Y *= b; return a; }
	template<typename Type> Vec<Type, 3>& operator *= (Vec<Type, 3>& a, Type b) { a.X *= b; a.Y *= b; a.Z *= b; return a; }
	template<typename Type> Vec<Type, 4>& operator *= (Vec<Type, 4>& a, Type b) { a.X *= b; a.Y *= b; a.Z *= b; a.W *= b; return a; }
	template<typename Type> Vec<Type, 2>& operator /= (Vec<Type, 2>& a, const Vec<Type, 2>& b) { a.X /= b.X; a.Y /= b.Y; return a; }
	template<typename Type> Vec<Type, 3>& operator /= (Vec<Type, 3>& a, const Vec<Type, 3>& b) { a.X /= b.X; a.Y /= b.Y; a.Z /= b.Z; return a; }
	template<typename Type> Vec<Type, 4>& operator /= (Vec<Type, 4>& a, const Vec<Type, 4>& b) { a.X /= b.X; a.Y /= b.Y; a.Z /= b.Z; a.W /= b.W; return a; }
	template<typename Type> Vec<Type, 2>& operator /= (Vec<Type, 2>& a, Type b) { a.X /= b; a.Y /= b; return a; }
	template<typename Type> Vec<Type, 3>& operator /= (Vec<Type, 3>& a, Type b) { a.X /= b; a.Y /= b; a.Z /= b; return a; }
	template<typename Type> Vec<Type, 4>& operator /= (Vec<Type, 4>& a, Type b) { a.X /= b; a.Y /= b; a.Z /= b; a.W /= b; return a; }
	template<typename Type> Vec<Type, 2>& operator &= (Vec<Type, 2>& a, const Vec<Type, 2>& b) { a.X &= b.X; a.Y &= b.Y; return a; }
	template<typename Type> Vec<Type, 3>& operator &= (Vec<Type, 3>& a, const Vec<Type, 3>& b) { a.X &= b.X; a.Y &= b.Y; a.Z &= b.Z; return a; }
	template<typename Type> Vec<Type, 4>& operator &= (Vec<Type, 4>& a, const Vec<Type, 4>& b) { a.X &= b.X; a.Y &= b.Y; a.Z &= b.Z; a.W &= b.W; return a; }
	template<typename Type> Vec<Type, 2>& operator &= (Vec<Type, 2>& a, Type b) { a.X &= b; a.Y &= b; return a; }
	template<typename Type> Vec<Type, 3>& operator &= (Vec<Type, 3>& a, Type b) { a.X &= b; a.Y &= b; a.Z &= b; return a; }
	template<typename Type> Vec<Type, 4>& operator &= (Vec<Type, 4>& a, Type b) { a.X &= b; a.Y &= b; a.Z &= b; a.W &= b; return a; }
	template<typename Type> Vec<Type, 2>& operator |= (Vec<Type, 2>& a, const Vec<Type, 2>& b) { a.X |= b.X; a.Y |= b.Y; return a; }
	template<typename Type> Vec<Type, 3>& operator |= (Vec<Type, 3>& a, const Vec<Type, 3>& b) { a.X |= b.X; a.Y |= b.Y; a.Z |= b.Z; return a; }
	template<typename Type> Vec<Type, 4>& operator |= (Vec<Type, 4>& a, const Vec<Type, 4>& b) { a.X |= b.X; a.Y |= b.Y; a.Z |= b.Z; a.W |= b.W; return a; }
	template<typename Type> Vec<Type, 2>& operator |= (Vec<Type, 2>& a, Type b) { a.X |= b; a.Y |= b; return a; }
	template<typename Type> Vec<Type, 3>& operator |= (Vec<Type, 3>& a, Type b) { a.X |= b; a.Y |= b; a.Z |= b; return a; }
	template<typename Type> Vec<Type, 4>& operator |= (Vec<Type, 4>& a, Type b) { a.X |= b; a.Y |= b; a.Z |= b; a.W |= b; return a; }
	template<typename Type> Vec<Type, 2>& operator ^= (Vec<Type, 2>& a, const Vec<Type, 2>& b) { a.X ^= b.X; a.Y ^= b.Y; return a; }
	template<typename Type> Vec<Type, 3>& operator ^= (Vec<Type, 3>& a, const Vec<Type, 3>& b) { a.X ^= b.X; a.Y ^= b.Y; a.Z ^= b.Z; return a; }
	template<typename Type> Vec<Type, 4>& operator ^= (Vec<Type, 4>& a, const Vec<Type, 4>& b) { a.X ^= b.X; a.Y ^= b.Y; a.Z ^= b.Z; a.W ^= b.W; return a; }
	template<typename Type> Vec<Type, 2>& operator ^= (Vec<Type, 2>& a, Type b) { a.X ^= b; a.Y ^= b; return a; }
	template<typename Type> Vec<Type, 3>& operator ^= (Vec<Type, 3>& a, Type b) { a.X ^= b; a.Y ^= b; a.Z ^= b; return a; }
	template<typename Type> Vec<Type, 4>& operator ^= (Vec<Type, 4>& a, Type b) { a.X ^= b; a.Y ^= b; a.Z ^= b; a.W ^= b; return a; }


	template<typename Type> constexpr Vec<bool, 2> operator == (const Vec<Type, 2>& a, const Vec<Type, 2>& b) { return Vec<bool, 2>{a.X == b.X, a.Y == b.Y}; }
	template<typename Type> constexpr Vec<bool, 3> operator == (const Vec<Type, 3>& a, const Vec<Type, 3>& b) { return Vec<bool, 3>{a.X == b.X, a.Y == b.Y, a.Z == b.Z}; }
	template<typename Type> constexpr Vec<bool, 4> operator == (const Vec<Type, 4>& a, const Vec<Type, 4>& b) { return Vec<bool, 4>{a.X == b.X, a.Y == b.Y, a.Z == b.Z, a.W == b.W}; }
	template<typename Type> constexpr Vec<bool, 2> operator == (Type a, const Vec<Type, 2>& b) { return Vec<bool, 2>{a == b.X, a == b.Y}; }
	template<typename Type> constexpr Vec<bool, 3> operator == (Type a, const Vec<Type, 3>& b) { return Vec<bool, 3>{a == b.X, a == b.Y, a == b.Z}; }
	template<typename Type> constexpr Vec<bool, 4> operator == (Type a, const Vec<Type, 4>& b) { return Vec<bool, 4>{a == b.X, a == b.Y, a == b.Z, a == b.W}; }
	template<typename Type> constexpr Vec<bool, 2> operator == (const Vec<Type, 2>& a, Type b) { return Vec<bool, 2>{a.X == b, a.Y == b}; }
	template<typename Type> constexpr Vec<bool, 3> operator == (const Vec<Type, 3>& a, Type b) { return Vec<bool, 3>{a.X == b, a.Y == b, a.Z == b}; }
	template<typename Type> constexpr Vec<bool, 4> operator == (const Vec<Type, 4>& a, Type b) { return Vec<bool, 4>{a.X == b, a.Y == b, a.Z == b, a.W == b}; }
	template<typename Type> constexpr Vec<bool, 2> operator != (const Vec<Type, 2>& a, const Vec<Type, 2>& b) { return Vec<bool, 2>{a.X != b.X, a.Y != b.Y}; }
	template<typename Type> constexpr Vec<bool, 3> operator != (const Vec<Type, 3>& a, const Vec<Type, 3>& b) { return Vec<bool, 3>{a.X != b.X, a.Y != b.Y, a.Z != b.Z}; }
	template<typename Type> constexpr Vec<bool, 4> operator != (const Vec<Type, 4>& a, const Vec<Type, 4>& b) { return Vec<bool, 4>{a.X != b.X, a.Y != b.Y, a.Z != b.Z, a.W != b.W}; }
	template<typename Type> constexpr Vec<bool, 2> operator != (Type a, const Vec<Type, 2>& b) { return Vec<bool, 2>{a != b.X, a != b.Y}; }
	template<typename Type> constexpr Vec<bool, 3> operator != (Type a, const Vec<Type, 3>& b) { return Vec<bool, 3>{a != b.X, a != b.Y, a != b.Z}; }
	template<typename Type> constexpr Vec<bool, 4> operator != (Type a, const Vec<Type, 4>& b) { return Vec<bool, 4>{a != b.X, a != b.Y, a != b.Z, a != b.W}; }
	template<typename Type> constexpr Vec<bool, 2> operator != (const Vec<Type, 2>& a, Type b) { return Vec<bool, 2>{a.X != b, a.Y != b}; }
	template<typename Type> constexpr Vec<bool, 3> operator != (const Vec<Type, 3>& a, Type b) { return Vec<bool, 3>{a.X != b, a.Y != b, a.Z != b}; }
	template<typename Type> constexpr Vec<bool, 4> operator != (const Vec<Type, 4>& a, Type b) { return Vec<bool, 4>{a.X != b, a.Y != b, a.Z != b, a.W != b}; }
	template<typename Type> constexpr Vec<bool, 2> operator < (const Vec<Type, 2>& a, const Vec<Type, 2>& b) { return Vec<bool, 2>{a.X < b.X, a.Y < b.Y}; }
	template<typename Type> constexpr Vec<bool, 3> operator < (const Vec<Type, 3>& a, const Vec<Type, 3>& b) { return Vec<bool, 3>{a.X < b.X, a.Y < b.Y, a.Z < b.Z}; }
	template<typename Type> constexpr Vec<bool, 4> operator < (const Vec<Type, 4>& a, const Vec<Type, 4>& b) { return Vec<bool, 4>{a.X < b.X, a.Y < b.Y, a.Z < b.Z, a.W < b.W}; }
	template<typename Type> constexpr Vec<bool, 2> operator < (Type a, const Vec<Type, 2>& b) { return Vec<bool, 2>{a < b.X, a < b.Y}; }
	template<typename Type> constexpr Vec<bool, 3> operator < (Type a, const Vec<Type, 3>& b) { return Vec<bool, 3>{a < b.X, a < b.Y, a < b.Z}; }
	template<typename Type> constexpr Vec<bool, 4> operator < (Type a, const Vec<Type, 4>& b) { return Vec<bool, 4>{a < b.X, a < b.Y, a < b.Z, a < b.W}; }
	template<typename Type> constexpr Vec<bool, 2> operator < (const Vec<Type, 2>& a, Type b) { return Vec<bool, 2>{a.X < b, a.Y < b}; }
	template<typename Type> constexpr Vec<bool, 3> operator < (const Vec<Type, 3>& a, Type b) { return Vec<bool, 3>{a.X < b, a.Y < b, a.Z < b}; }
	template<typename Type> constexpr Vec<bool, 4> operator < (const Vec<Type, 4>& a, Type b) { return Vec<bool, 4>{a.X < b, a.Y < b, a.Z < b, a.W < b}; }
	template<typename Type> constexpr Vec<bool, 2> operator > (const Vec<Type, 2>& a, const Vec<Type, 2>& b) { return Vec<bool, 2>{a.X > b.X, a.Y > b.Y}; }
	template<typename Type> constexpr Vec<bool, 3> operator > (const Vec<Type, 3>& a, const Vec<Type, 3>& b) { return Vec<bool, 3>{a.X > b.X, a.Y > b.Y, a.Z > b.Z}; }
	template<typename Type> constexpr Vec<bool, 4> operator > (const Vec<Type, 4>& a, const Vec<Type, 4>& b) { return Vec<bool, 4>{a.X > b.X, a.Y > b.Y, a.Z > b.Z, a.W > b.W}; }
	template<typename Type> constexpr Vec<bool, 2> operator > (Type a, const Vec<Type, 2>& b) { return Vec<bool, 2>{a > b.X, a > b.Y}; }
	template<typename Type> constexpr Vec<bool, 3> operator > (Type a, const Vec<Type, 3>& b) { return Vec<bool, 3>{a > b.X, a > b.Y, a > b.Z}; }
	template<typename Type> constexpr Vec<bool, 4> operator > (Type a, const Vec<Type, 4>& b) { return Vec<bool, 4>{a > b.X, a > b.Y, a > b.Z, a > b.W}; }
	template<typename Type> constexpr Vec<bool, 2> operator > (const Vec<Type, 2>& a, Type b) { return Vec<bool, 2>{a.X > b, a.Y > b}; }
	template<typename Type> constexpr Vec<bool, 3> operator > (const Vec<Type, 3>& a, Type b) { return Vec<bool, 3>{a.X > b, a.Y > b, a.Z > b}; }
	template<typename Type> constexpr Vec<bool, 4> operator > (const Vec<Type, 4>& a, Type b) { return Vec<bool, 4>{a.X > b, a.Y > b, a.Z > b, a.W > b}; }
	template<typename Type> constexpr Vec<bool, 2> operator <= (const Vec<Type, 2>& a, const Vec<Type, 2>& b) { return Vec<bool, 2>{a.X <= b.X, a.Y <= b.Y}; }
	template<typename Type> constexpr Vec<bool, 3> operator <= (const Vec<Type, 3>& a, const Vec<Type, 3>& b) { return Vec<bool, 3>{a.X <= b.X, a.Y <= b.Y, a.Z <= b.Z}; }
	template<typename Type> constexpr Vec<bool, 4> operator <= (const Vec<Type, 4>& a, const Vec<Type, 4>& b) { return Vec<bool, 4>{a.X <= b.X, a.Y <= b.Y, a.Z <= b.Z, a.W <= b.W}; }
	template<typename Type> constexpr Vec<bool, 2> operator <= (Type a, const Vec<Type, 2>& b) { return Vec<bool, 2>{a <= b.X, a <= b.Y}; }
	template<typename Type> constexpr Vec<bool, 3> operator <= (Type a, const Vec<Type, 3>& b) { return Vec<bool, 3>{a <= b.X, a <= b.Y, a <= b.Z}; }
	template<typename Type> constexpr Vec<bool, 4> operator <= (Type a, const Vec<Type, 4>& b) { return Vec<bool, 4>{a <= b.X, a <= b.Y, a <= b.Z, a <= b.W}; }
	template<typename Type> constexpr Vec<bool, 2> operator <= (const Vec<Type, 2>& a, Type b) { return Vec<bool, 2>{a.X <= b, a.Y <= b}; }
	template<typename Type> constexpr Vec<bool, 3> operator <= (const Vec<Type, 3>& a, Type b) { return Vec<bool, 3>{a.X <= b, a.Y <= b, a.Z <= b}; }
	template<typename Type> constexpr Vec<bool, 4> operator <= (const Vec<Type, 4>& a, Type b) { return Vec<bool, 4>{a.X <= b, a.Y <= b, a.Z <= b, a.W <= b}; }
	template<typename Type> constexpr Vec<bool, 2> operator >= (const Vec<Type, 2>& a, const Vec<Type, 2>& b) { return Vec<bool, 2>{a.X >= b.X, a.Y >= b.Y}; }
	template<typename Type> constexpr Vec<bool, 3> operator >= (const Vec<Type, 3>& a, const Vec<Type, 3>& b) { return Vec<bool, 3>{a.X >= b.X, a.Y >= b.Y, a.Z >= b.Z}; }
	template<typename Type> constexpr Vec<bool, 4> operator >= (const Vec<Type, 4>& a, const Vec<Type, 4>& b) { return Vec<bool, 4>{a.X >= b.X, a.Y >= b.Y, a.Z >= b.Z, a.W >= b.W}; }
	template<typename Type> constexpr Vec<bool, 2> operator >= (Type a, const Vec<Type, 2>& b) { return Vec<bool, 2>{a >= b.X, a >= b.Y}; }
	template<typename Type> constexpr Vec<bool, 3> operator >= (Type a, const Vec<Type, 3>& b) { return Vec<bool, 3>{a >= b.X, a >= b.Y, a >= b.Z}; }
	template<typename Type> constexpr Vec<bool, 4> operator >= (Type a, const Vec<Type, 4>& b) { return Vec<bool, 4>{a >= b.X, a >= b.Y, a >= b.Z, a >= b.W}; }
	template<typename Type> constexpr Vec<bool, 2> operator >= (const Vec<Type, 2>& a, Type b) { return Vec<bool, 2>{a.X >= b, a.Y >= b}; }
	template<typename Type> constexpr Vec<bool, 3> operator >= (const Vec<Type, 3>& a, Type b) { return Vec<bool, 3>{a.X >= b, a.Y >= b, a.Z >= b}; }
	template<typename Type> constexpr Vec<bool, 4> operator >= (const Vec<Type, 4>& a, Type b) { return Vec<bool, 4>{a.X >= b, a.Y >= b, a.Z >= b, a.W >= b}; }


	template <typename Type, Uint32 N>
	Type Dot(const Vec<Type, N>& a, const Vec<Type, N>& b) {
		Type result{ 0 };
		for (Uint32 Index = 0; Index < N; ++Index)
			result += a[Index] * b[Index];

		return result;
	}

	template <typename Type>
	constexpr Type Dot(const Vec<Type, 2>& a, const Vec<Type, 2>& b) { return a.X * b.X + a.Y * b.Y; }

	template <typename Type>
	constexpr Type Dot(const Vec<Type, 3>& a, const Vec<Type, 3>& b) { return a.X * b.X + a.Y * b.Y + a.Z * b.Z; }

	template <typename Type>
	constexpr Type Dot(const Vec<Type, 4>& a, const Vec<Type, 4>& b) { return a.X * b.X + a.Y * b.Y + a.Z * b.Z + a.W * b.W; }


	template <typename Type, Uint32 N>
	Type LengthSquared(const Vec<Type, N>& a) { return Dot(a, a); }

	template <typename Type, Uint32 N>
	Type Length(const Vec<Type, N>& a) { return Sqrt(LengthSquared(a)); }










}