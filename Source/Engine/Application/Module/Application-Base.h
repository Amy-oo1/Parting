#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"

PARTING_SUBMODULE(DeviceManager, Base)

PARTING_IMPORT GLFWWrapper;

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;
PARTING_IMPORT Container;
PARTING_IMPORT Logger;

PARTING_IMPORT RHI;


#else
#pragma once

#include "Core/ModuleBuild.h"


#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"
//Global
#include "Engine/Application/Module/GLFWWrapper.h"

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#include "Core/Container/Module/Container.h"
#include "Core/VFS/Module/VFS.h"

#include "Core/Logger/Module/Logger.h"

#include "RHI/Module/RHI.h"

#include "Core/Algorithm/Module/Algorithm.h"
#include "Core/VectorMath/Module/VectorMath.h"

#include "Engine/Engine/Module/TextureCache.h"
#include "Engine/Engine/Module/CommonRenderPasses.h"

#include "Engine/Application/Module/DeviceManager.h"

#endif // PARTING_MODULE_BUILD

namespace Parting {

	template<RHI::APITagConcept APITag>
	class ApplicationBase : public IRenderPass<APITag> {
	protected:
		ApplicationBase(typename ManageTypeTraits<APITag>::DeviceManager* deviceManager) :
			IRenderPass<APITag>{ deviceManager } {}

	protected:
		Vector<String> FindScenes(IFileSystem& fs, const Path& path);


	protected:
		bool m_SceneLoaded{ false };
		bool m_AllTexturesFinalized{ false };
		bool m_IsAsyncLoad{ false };

		SharedPtr<TextureCache<APITag>> m_TextureCache;
		SharedPtr<CommonRenderPasses<APITag>> m_CommonPasses;

	private:



	};
	template<RHI::APITagConcept APITag>
	inline Vector<String> ApplicationBase<APITag>::FindScenes(IFileSystem& fs, const Path& path) {
		static Vector<String> sceneExtensions{ ".scene.json", ".gltf", ".glb" };

		Vector<String> scenes;

		Deque<Path> searchList;
		searchList.push_back(path);

		while (!searchList.empty()) {
			Path currentPath{ searchList.front() };//must copy
			searchList.pop_front();

			// search current directory
			fs.EnumerateFiles(
				currentPath,
				sceneExtensions,
				[&scenes, &currentPath](StringView name) {
					scenes.push_back((currentPath / name).generic_string());
				}
			);

			// search subdirectories
			fs.EnumerateDirectories(
				currentPath,
				[&searchList, &currentPath](StringView name) {
					if (name != "glTF-Draco")// jump Google Draco 
						searchList.push_back(currentPath / name);
				}
			);
		}

		return scenes;
	}
}