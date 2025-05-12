#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"

PARTING_SUBMODULE(D3D12RHI, Heap)

PARTING_IMPORT DirectX12Wrapper;

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;
PARTING_IMPORT Container;

PARTING_IMPORT RHI;

PARTING_SUBMODE_IMPORT(Traits)
PARTING_SUBMODE_IMPORT(Common)
PARTING_SUBMODE_IMPORT(Format)

#else
#pragma once

#include "Core/ModuleBuild.h"

#include "Core/Utility/Include/UtilityMacros.h"
//Global
#include "D3D12RHI/Module/DirectX12Wrapper.h"

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#include "Core/Container/Module/Container.h"

#include "RHI/Module/RHI.h"

#include "D3D12RHI/Module/D3D12RHI-Traits.h"
#include "D3D12RHI/Module/D3D12RHI-Common.h"

#endif // PARTING_MODULE_BUILD

namespace RHI::D3D12 {
	
	class Heap :public RHIHeap<Heap> {
		friend class RHIResource<Heap>;
		friend class RHIHeap<Heap>;

		friend class Device;
	public:
		Heap(void) = default;
		~Heap(void) = default;

	private:
		RefCountPtr<ID3D12Heap> m_Heap{ nullptr };
		RHIHeapDesc m_Desc;

	private:
		const RHIHeapDesc& Imp_Get_Desc(void) const { return this->m_Desc; }

	};

}