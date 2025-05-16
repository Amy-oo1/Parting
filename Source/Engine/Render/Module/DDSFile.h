#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"

PARTING_MODULE(BindingCache)

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;


#else
#pragma once

#include "Core/ModuleBuild.h"


#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"
//Global

#include "ThirdParty/ShaderMake/include/ShaderMake/ShaderBlob.h"

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#include "Core/Concurrent/Module/Concurrent.h"
#include "Core/Container/Module/Container.h"
#include "Core/String/Module/String.h"
#include "Core/VFS/Module/VFS.h"
#include "Core/Logger/Module/Logger.h"

#include "RHI/Module/RHI.h"
#include "D3D12RHI/Module/D3D12RHI.h"

#include "Engine/Render/Module/SceneTypes.h"
#include "Engine/Engine/Module/TextureCache.h"

#endif // PARTING_MODULE_BUILD

namespace Parting {

	//template<RHI::APITagConcept APITag>
	//bool LoadDDSTextureFromMemory(TextureData<APITag>& textureInfo);

	//// Creates a texture based on DDS data in memory
	//template<RHI::APITagConcept APITag>
	//auto CreateDDSTextureFromMemory(  :IDevice* device, nvrhi::ICommandList* commandList, std::shared_ptr<vfs::IBlob> data, const char* debugName = nullptr, bool forceSRGB = false);

	//std::shared_ptr<vfs::IBlob> SaveStagingTextureAsDDS(nvrhi::IDevice* device, nvrhi::IStagingTexture* stagingTexture);

}