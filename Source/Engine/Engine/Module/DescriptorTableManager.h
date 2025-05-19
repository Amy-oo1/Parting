#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"

PARTING_MODULE(DescriptorTableManager)

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;


#else
#pragma once

#include "Core/ModuleBuild.h"


#include "Core/Utility/Include/UtilityMacros.h"
//Global

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#include "Core/Concurrent/Module/Concurrent.h"
#include "Core/Container/Module/Container.h"

#include "RHI/Module/RHI.h"
#include "D3D12RHI/Module/D3D12RHI.h"

#include "Engine/Engine/Module/BindingCache.h"

#endif // PARTING_MODULE_BUILD

namespace Parting {


	template<RHI::APITagConcept APITag>
	class DescriptorTableManager final :public EnableSharedFromThis<DescriptorTableManager<APITag>> {
		using Imp_Device = typename RHI::RHITypeTraits<APITag>::Imp_Device;

	public:
		using DescriptorIndex = Int32;

	protected:
		RHI::RefCountPtr<Imp_Device> m_Device{ nullptr };

	};


	template<RHI::APITagConcept APITag>
	class DescriptorHandle final :public  MoveAbleOnly {
	public:
		using DescriptorIndex = typename DescriptorTableManager<APITag>::DescriptorIndex;

	public:
		DescriptorHandle(void) = default;
		DescriptorHandle(SharedPtr<DescriptorTableManager<APITag>> manager, DescriptorIndex index) :
			m_Manager{ manager },
			m_DescriptorIndex{ index } {
		}
		~DescriptorHandle(void) = default;

	public:
		DescriptorIndex Get(void)const { return this->m_DescriptorIndex; }


	private:
		WeakPtr<DescriptorTableManager<APITag>> m_Manager;
		DescriptorIndex m_DescriptorIndex{ -1 };//TODO set a Invalid value

	};
}

