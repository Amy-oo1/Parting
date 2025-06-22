#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"
PARTING_GLOBAL_MODULE

PARTING_SUBMODULE(RHI, Heap)

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;
PARTING_IMPORT String;
PARTING_IMPORT Logger;

PARTING_SUBMODE_IMPORT(Resource)

#else
#pragma once

#include "Core/ModuleBuild.h"

#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"
//Global

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#include "Core/String/Module/String.h"
#include "Core/Logger/Module/Logger.h"

#include "RHI/Module/RHI-Resource.h"

#endif // PARTING_MODULE_BUILD

namespace RHI {

	PARTING_EXPORT enum class RHIHeapType :Uint8 {
		DeviceLocal,
		Upload,
		Readback
	};

	PARTING_EXPORT struct RHIHeapDesc final {
		RHIHeapType Type{ RHIHeapType::DeviceLocal };
		Uint64 Size{ 0 };
		WString DebugName;
	};

	PARTING_EXPORT template<typename Derived>
	class RHIHeap :public RHIResource<Derived> {
		friend class RHIResource<Derived>;
	protected:
		RHIHeap(void) = default;
		PARTING_VIRTUAL ~RHIHeap(void) = default;

	public:
		const RHIHeapDesc& Get_Desc(void)const { return this->Get_Derived()->Imp_Get_Desc(); }
	
	private:
		STDNODISCARD Derived* Get_Derived(void)noexcept { return static_cast<Derived*>(this); }
		STDNODISCARD const Derived* Get_Derived(void)const noexcept { return static_cast<const Derived*>(this); }
	private:
		const RHIHeapDesc& Imp_Get_Desc(void) const { LOG_ERROR("No Imp"); return  RHIHeapDesc{}; }

	};

	PARTING_EXPORT struct RHIMemoryRequirements final {
		Uint64 Size;
		Uint64 Alignment{ 0 };//In D3D12 This wiil useed Default
	};

}