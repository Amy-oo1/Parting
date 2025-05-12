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
		


	protected:
		RHI::RefCountPtr<Imp_Device> m_Device{ nullptr };

	};
}

 