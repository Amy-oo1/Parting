#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE

PARTING_SUBMODULE(Platform, Type)

PARTING_IMPORT std;

#else 
#pragma once

#include "Core/ModuleBuild.h"

#include<cstdint>
#include <limits>

#endif // PARTING_MODULE


PARTING_EXPORT using Uint8 = std::uint8_t;
PARTING_EXPORT using Int8 = std::int8_t;
PARTING_EXPORT using Uint16 = std::uint16_t;
PARTING_EXPORT using Int16 = std::int16_t;
PARTING_EXPORT using Uint32 = std::uint32_t;
PARTING_EXPORT using Int32 = std::int32_t;
PARTING_EXPORT using Uint64 = std::uint64_t;
PARTING_EXPORT using Int64 = std::int64_t;

PARTING_EXPORT using Nullptr_T = std::nullptr_t;

PARTING_EXPORT HEADER_INLINE constexpr auto Min_Uint8{ std::numeric_limits<Uint8>::min() };
PARTING_EXPORT HEADER_INLINE constexpr auto Min_Int8{ std::numeric_limits<Int8>::min() };
PARTING_EXPORT HEADER_INLINE constexpr auto Min_Uint16{ std::numeric_limits<Uint16>::min() };
PARTING_EXPORT HEADER_INLINE constexpr auto Min_Int16{ std::numeric_limits<Int16>::min() };
PARTING_EXPORT HEADER_INLINE constexpr auto Min_Uint32{ std::numeric_limits<Uint32>::min() };
PARTING_EXPORT HEADER_INLINE constexpr auto Min_Int32{ std::numeric_limits<Int32>::min() };
PARTING_EXPORT HEADER_INLINE constexpr auto Min_Uint64{ std::numeric_limits<Uint64>::min() };
PARTING_EXPORT HEADER_INLINE constexpr auto Min_Int64{ std::numeric_limits<Int64>::min() };
PARTING_EXPORT HEADER_INLINE constexpr auto Min_Float{ std::numeric_limits<float>::min() };
PARTING_EXPORT HEADER_INLINE constexpr auto Min_Double{ std::numeric_limits<double>::min() };

PARTING_EXPORT HEADER_INLINE constexpr auto Max_Uint8{ std::numeric_limits<Uint8>::max() };
PARTING_EXPORT HEADER_INLINE constexpr auto Max_Int8{ std::numeric_limits<Int8>::max() };
PARTING_EXPORT HEADER_INLINE constexpr auto Max_Uint16{ std::numeric_limits<Uint16>::max() };
PARTING_EXPORT HEADER_INLINE constexpr auto Max_Int16{ std::numeric_limits<Int16>::max() };
PARTING_EXPORT HEADER_INLINE constexpr auto Max_Uint32{ std::numeric_limits<Uint32>::max() };
PARTING_EXPORT HEADER_INLINE constexpr auto Max_Int32{ std::numeric_limits<Int32>::max() };
PARTING_EXPORT HEADER_INLINE constexpr auto Max_Uint64{ std::numeric_limits<Uint64>::max() };
PARTING_EXPORT HEADER_INLINE constexpr auto Max_Int64{ std::numeric_limits<Int64>::max() };
PARTING_EXPORT HEADER_INLINE constexpr auto Max_Float{ std::numeric_limits<float>::max() };
PARTING_EXPORT HEADER_INLINE constexpr auto Max_Double{ std::numeric_limits<double>::max() };

