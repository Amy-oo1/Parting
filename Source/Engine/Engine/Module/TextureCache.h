#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"

PARTING_MODULE(TextureCahe)

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;


#else
#pragma once

#include "Core/ModuleBuild.h"


#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"
//Global

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#include "Core/Concurrent/Module/Concurrent.h"
#include "Core/Container/Module/Container.h"
#include "Core/VFS/Module/VFS.h"
#include "Core/String/Module/String.h"
#include "Core/Logger/Module/Logger.h"

#include "RHI/Module/RHI.h"
#include "D3D12RHI/Module/D3D12RHI.h"

#include "Engine/Engine/Module/DescriptorTableManager.h"

#endif // PARTING_MODULE_BUILD

namespace Parting {
	template<RHI::APITagConcept APITag>
	class TextureCache final {
		using Imp_Device = typename RHI::RHITypeTraits<APITag>::Imp_Device;
		using Imp_Texture = typename RHI::RHITypeTraits<APITag>::Imp_Texture;
	public:
		TextureCache(RHI::RefCountPtr<Imp_Device> device, SharedPtr<IFileSystem> fs, SharedPtr<DescriptorTableManager<APITag>> descriptorTableManager) :
			m_Device{ device },
			m_FS{ fs },
			m_DescriptorTableManager{ descriptorTableManager }
		{}
		~TextureCache(void) = default;

	private:
		RHI::RefCountPtr<Imp_Device> m_Device;
		SharedPtr<IFileSystem> m_FS;
		SharedPtr<DescriptorTableManager<APITag>> m_DescriptorTableManager;

	};
}