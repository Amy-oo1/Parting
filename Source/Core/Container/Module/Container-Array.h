#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE

PARTING_SUBMODULE(Container, Array)

PARTING_IMPORT std;

#else
#pragma once

#include "Core/ModuleBuild.h"

//Global
#include<array>
#include<algorithm>
#include<xutility>

#include "Core/Utility/Module/Utility.h"

#endif // PARTING_MODULE_BUILD

//NOTEL :Size==0 ,std has a specialization
PARTING_EXPORT template<typename Type, size_t _Size>
using Array = std::array<Type, _Size>;

PARTING_EXPORT template<typename _Type, size_t _Size>
STDNODISCARD constexpr bool ArrayEqual(const Array<_Type, _Size>& Lhs, const uint32_t LhsUsed, const Array<_Type, _Size>& Rhs, const uint32_t RhsUsed) { return std::equal(Lhs.begin(), Lhs.begin() + LhsUsed, Rhs.begin(), Rhs.begin() + RhsUsed); }

PARTING_EXPORT template<typename _Type, size_t _Size>
	requires(_Size > 0 && _Size < 32)
STDNODISCARD constexpr uint32_t Array32DifferenceMask(const Array<_Type, _Size>& Lhs, const Array<_Type, _Size>& Rhs) {
	uint32_t Mask{ 0 };
	for (uint32_t Index = 0; Index < _Size; ++Index)
		if (Lhs[Index] != Rhs[Index])
			Mask |= (1 << Index);

	return Mask;
}
