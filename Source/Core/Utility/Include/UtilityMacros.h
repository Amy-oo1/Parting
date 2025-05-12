#pragma once

#include <cassert>
#include <type_traits>//NOTE you want to use __Comp;ie __Comp for the compiler version


#define _W(x) L##x
#define _u8(x) u8##x


#define STDNODISCARD  [[nodiscard]]

#ifdef _DEBUG
#define PARTING_VIRTUAL /*virtual*/
#else 
#define PARTING_VIRTUAL
#endif // _DEBUG


#ifdef _DEBUG
#define ASSERT(x) if (!(x)) { assert(false); };

#else 
#define ASSERT(x)

#endif // _DEBUG

#define	TRIVIAL_FUNCTION(x) \
	public:\
		x(void) noexcept = default; \
		x(const x&) noexcept = default; \
		x(x&&) noexcept = default; \
		x& operator=(const x&) noexcept = default; \
		x& operator=(x&&) noexcept = default; \
		~x(void) noexcept = default;

#define CONSTEXPR_TRIVIAL_FUNCTION(x)\
	public:\
		constexpr x(void) noexcept = default; \
		constexpr x(const x&) noexcept = default; \
		constexpr x(x&&) noexcept = default; \
		constexpr x& operator=(const x&) noexcept = default; \
		constexpr x& operator=(x&&) noexcept = default; \
		~x(void) noexcept = default;


#define ASSERT_HAS_FUNCTION(Class, Func) \
static_assert(requires(Class& obj) { obj.Func; }, \
"Class " #Class " missing function: " #Func)

#define ASSERT_HAS_NESTED_TYPE(Class, TypeName) \
static_assert(requires { typename Class::TypeName; }, \
"Class " #Class " missing nested type: " #TypeName)

#define ASSERT_HAS_MEMBER_VARIABLE(Class, Member) \
    static_assert(requires(Class& obj) { obj.Member; }, \
"Class " #Class " missing member variable: " #Member)


#define ENUM_CLASS_OPERATORS(EnumType) \
STDNODISCARD inline constexpr EnumType operator| (EnumType lhs, EnumType rhs) noexcept { return static_cast<EnumType>(static_cast<UnderlyingType<EnumType>>(lhs) | static_cast<UnderlyingType<EnumType>>(rhs)); }\
STDNODISCARD inline constexpr EnumType operator& (EnumType lhs, EnumType rhs) noexcept { return static_cast<EnumType>(static_cast<UnderlyingType<EnumType>>(lhs) & static_cast<UnderlyingType<EnumType>>(rhs)); }\
STDNODISCARD inline constexpr EnumType operator^ (EnumType lhs, EnumType rhs) noexcept { return static_cast<EnumType>(static_cast<UnderlyingType<EnumType>>(lhs) ^ static_cast<UnderlyingType<EnumType>>(rhs)); }\
inline constexpr EnumType& operator|= (EnumType& lhs, EnumType rhs) noexcept { return lhs = lhs | rhs; }\
inline constexpr EnumType& operator&= (EnumType& lhs, EnumType rhs) noexcept { return lhs = lhs & rhs; }\
inline constexpr EnumType& operator^= (EnumType& lhs, EnumType rhs) noexcept { return lhs = lhs ^ rhs; }

#define EXPORT_ENUM_CLASS_OPERATORS(EnumType) \
PARTING_EXPORT STDNODISCARD inline constexpr EnumType operator| (EnumType lhs, EnumType rhs) noexcept { return static_cast<EnumType>(static_cast<UnderlyingType<EnumType>>(lhs) | static_cast<UnderlyingType<EnumType>>(rhs)); }\
PARTING_EXPORT STDNODISCARD inline constexpr EnumType operator& (EnumType lhs, EnumType rhs) noexcept { return static_cast<EnumType>(static_cast<UnderlyingType<EnumType>>(lhs) & static_cast<UnderlyingType<EnumType>>(rhs)); }\
PARTING_EXPORT STDNODISCARD inline constexpr EnumType operator^ (EnumType lhs, EnumType rhs) noexcept { return static_cast<EnumType>(static_cast<UnderlyingType<EnumType>>(lhs) ^ static_cast<UnderlyingType<EnumType>>(rhs)); }\
PARTING_EXPORT inline constexpr EnumType& operator|= (EnumType& lhs, EnumType rhs) noexcept { return lhs = lhs | rhs; }\
PARTING_EXPORT inline constexpr EnumType& operator&= (EnumType& lhs, EnumType rhs) noexcept { return lhs = lhs & rhs; }\
PARTING_EXPORT inline constexpr EnumType& operator^= (EnumType& lhs, EnumType rhs) noexcept { return lhs = lhs ^ rhs; }