#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE

PARTING_SUBMODULE(Utility, Concept)

PARTING_EXPORT std;

#else 
#pragma once
#include "Core/ModuleBuild.h"

//Global

#include<concepts>

#endif // PARTING_MODULE_BUILD

PARTING_EXPORT template<typename Type>
concept Integral = std::integral<Type>;

PARTING_EXPORT template<typename Type>
concept UnsignedIntegtal = std::unsigned_integral<Type>;

PARTING_EXPORT template<typename Type>
concept SignedIntegral = std::signed_integral<Type>;

PARTING_EXPORT template<typename Type>
concept FloatingPoint = std::floating_point<Type>;

PARTING_EXPORT template<typename Type>
concept TotallyOrder = std::totally_ordered<Type>;

PARTING_EXPORT template<class _From, class _To>
concept ConvertibleTo = std::convertible_to<_From, _To>;

PARTING_EXPORT template<typename Type>
concept Trivial = requires(Type t) {
	{ std::is_trivially_constructible_v<Type> };
	{ std::is_trivially_destructible_v<Type> };
	{ std::is_trivially_copyable_v<Type> };
};

PARTING_EXPORT template<typename Type>
concept StandardLayout = requires{
	{std::is_standard_layout_v<Type>};
};

PARTING_EXPORT template<typename Type>
concept POD = Trivial<Type> && StandardLayout<Type>;
