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

	public:
		Vector<String> FindScenes(IFileSystem& fs, const Path& path);

		void Set_AsyncLoad(bool isAsync) { this->m_IsAsyncLoad = isAsync; }

		// searches for a given substring in the list of scenes, returns that name if found;
		// if not found, returns the first scene in the list.
		static String FindPreferredScene(const Vector<String>& available, const String& preferred);


	public:


		virtual void BeginLoadingScene(SharedPtr<IFileSystem> fs, const Path& scenePath);

		virtual bool LoadScene(SharedPtr<IFileSystem> fs, const Path& sceneFileName) = 0;


	protected:
		virtual void SceneUnloading(void) { this->m_SceneLoaded = false; }
		virtual void SceneLoaded(void);



	protected:
		bool m_SceneLoaded{ false };
		bool m_AllTexturesFinalized{ false };
		bool m_IsAsyncLoad{ false };

		SharedPtr<TextureCache<APITag>> m_TextureCache;
		SharedPtr<CommonRenderPasses<APITag>> m_CommonPasses;

		UniquePtr<Thread> m_SceneLoadingThread;

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

	template<RHI::APITagConcept APITag>
	inline String ApplicationBase<APITag>::FindPreferredScene(const Vector<String>& available, const String& preferred) {
		if (available.empty())
			return String{};

		for (const auto& s : available)
			if (s.find(preferred) != String::npos)
				return s;

		return available.front();
	}

	template<RHI::APITagConcept APITag>
	inline void ApplicationBase<APITag>::BeginLoadingScene(SharedPtr<IFileSystem> fs, const Path& scenePath) {
		if (this->m_SceneLoaded)
			this->SceneUnloading();

		this->m_AllTexturesFinalized = false;

		if (nullptr != this->m_TextureCache)
			this->m_TextureCache->Reset();

		this->m_DeviceManager->Get_Device()->WaitForIdle();
		this->m_DeviceManager->Get_Device()->RunGarbageCollection();

		if (this->m_IsAsyncLoad)
			this->m_SceneLoadingThread = MakeUnique<Thread>(
				[this, fs, scenePath](void) {
					this->m_SceneLoaded = this->LoadScene(fs, scenePath);
				}
			);
		else {
			this->m_SceneLoaded = this->LoadScene(fs, scenePath);
			this->SceneLoaded();
		}

	}

	template<RHI::APITagConcept APITag>
	inline void ApplicationBase<APITag>::SceneLoaded(void) {
		if (nullptr != this->m_TextureCache) {
			this->m_TextureCache->ProcessRenderingThreadCommands(*this->m_CommonPasses, 0.0f);

			this->m_TextureCache->LoadingFinished();
		}

		this->m_SceneLoaded = true;
	}

}