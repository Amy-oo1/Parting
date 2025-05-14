#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"


PARTING_IMPORT Platform;
PARTING_IMPORT Utility;
PARTING_IMPORT Algorithm;
PARTING_IMPORT Container;
PARTING_IMPORT VectorMath;
PARTING_IMPORT Logger;


PARTING_SUBMODULE(Parting, SSAOPass)


#else
#pragma once

#include "Core/ModuleBuild.h"


#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"
//Global

#include "ThirdParty/taskflow/taskflow.hpp"

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#include "Core/Algorithm/Module/Algorithm.h"
#include "Core/Container/Module/Container.h"
#include "Core/VectorMath/Module/VectorMath.h"
#include "Core/Logger/Module/Logger.h"

#include "RHI/Module/RHI.h"
#include "D3D12RHI/Module/D3D12RHI.h"

#include "Engine/Engine/Module/ShaderFactory.h"
#include "Engine/Engine/Module/DescriptorTableManager.h"
#include "Engine/Engine/Module/TextureCache.h"
#include "Engine/Engine/Module/SceneGraph.h"
#include "Engine/Render/Module/GLTFImporter.h"

#include "Shader/material_cb.h"
#include "Shader/bindless.h"
#include "Shader/skinning_cb.h"

#endif // PARTING_MODULE_BUILD

namespace Parting {

	template<RHI::APITagConcept APITag>
	class Scene final {
		using Imp_Device = typename RHI::RHITypeTraits<APITag>::Imp_Device;
		using Imp_Shader = typename RHI::RHITypeTraits<APITag>::Imp_Shader;
		using Imp_InputLayout = typename RHI::RHITypeTraits<APITag>::Imp_InputLayout;
		using Imp_BindingLayout = typename RHI::RHITypeTraits<APITag>::Imp_BindingLayout;
		using Imp_ComputePipeline = typename RHI::RHITypeTraits<APITag>::Imp_ComputePipeline;

	private:
		struct Resource final {
			Vector<MaterialConstants> MaterialData;
			Vector<GeometryData> GeometryData;
			Vector<InstanceData> InstanceData;
		};

	public:
		Scene(Imp_Device* device, ShaderFactory<APITag>& shaderFactory, SharedPtr<IFileSystem> fs, SharedPtr<TextureCache<APITag>> textureCache, SharedPtr<DescriptorTableManager<APITag>> descriptorTable, SharedPtr<SceneTypeFactory<APITag>> sceneTypeFactory);

		bool Load(const Path& JsonFileName);

		bool LoadWithExecutor(const Path& sceneFileName, tf::Executor* executor);

		void LoadModelAsync(Uint32 Index, const Path& FileName, tf::Executor* excutor);

	private:
		RHI::RefCountPtr<Imp_Device> m_Device;
		RHI::RefCountPtr<Imp_Shader> m_SkinningShader;
		RHI::RefCountPtr<Imp_BindingLayout> m_SkinningBindingLayout;
		RHI::RefCountPtr<Imp_ComputePipeline> m_SkinningPipeline;

		SharedPtr<IFileSystem> m_FS;
		SharedPtr<TextureCache<APITag>> m_TextureCache;
		SharedPtr<DescriptorTableManager<APITag>> m_DescriptorTable;
		SharedPtr<SceneTypeFactory<APITag>> m_SceneTypeFactory;

		SharedPtr<Scene::Resource> m_Resource;

		SharedPtr<GLTFImporter<APITag>> m_GLTFImporter;
		SharedPtr<SceneGraph<APITag>> m_SceneGraph;
		Vector<SceneImportResult<APITag>> m_Models;

		bool m_EnableBindlessResources{ false };


	};

	template<RHI::APITagConcept APITag>
	inline Scene<APITag>::Scene(Imp_Device* device, ShaderFactory<APITag>& shaderFactory, SharedPtr<IFileSystem> fs, SharedPtr<TextureCache<APITag>> textureCache, SharedPtr<DescriptorTableManager<APITag>> descriptorTable, SharedPtr<SceneTypeFactory<APITag>> sceneTypeFactory) :
		m_Device{ device },
		m_FS{ MoveTemp(fs) },
		m_TextureCache{ MoveTemp(textureCache) },
		m_DescriptorTable{ MoveTemp(descriptorTable) },
		m_SceneTypeFactory{ MoveTemp(sceneTypeFactory) } {

		this->m_SkinningShader = shaderFactory.CreateShader("Parting/skinning_cs", "main", nullptr, RHI::RHIShaderType::Compute);

		this->m_SkinningBindingLayout = this->m_Device->CreateBindingLayout(RHI::RHIBindingLayoutDescBuilder{}
			.Set_Visibility(RHI::RHIShaderType::Compute)
			.AddBinding(RHI::RHIBindingLayoutItem::BuildPushConstants(0, sizeof(SkinningConstants)))
			.AddBinding(RHI::RHIBindingLayoutItem::BuildRawBuffer_SRV(0))
			.AddBinding(RHI::RHIBindingLayoutItem::BuildRawBuffer_SRV(1))
			.AddBinding(RHI::RHIBindingLayoutItem::BuildRawBuffer_UAV(0))
			.Build()
		);

		this->m_SkinningPipeline = this->m_Device->CreateComputePipeline(RHI::RHIComputePipelineDescBuilder<APITag>{}
		.Set_CS(this->m_SkinningShader)
			.AddBindingLayout(this->m_SkinningBindingLayout)
			.Build()
			);

		this->m_Resource = MakeShared<Resource>();
		if (nullptr == this->m_SceneTypeFactory)
			this->m_SceneTypeFactory = MakeShared<SceneTypeFactory<APITag>>();

		this->m_GLTFImporter = MakeShared<GLTFImporter<APITag>>(this->m_FS, this->m_SceneTypeFactory);


	}

	template<RHI::APITagConcept APITag>
	bool Scene<APITag>::Load(const Path& JsonFileName) {
		tf::Executor executor;
		/*return this->LoadWithExecutor(JsonFileName, &executor);*/
		return this->LoadWithExecutor(JsonFileName, nullptr);
	}


	template<RHI::APITagConcept APITag>
	inline bool Scene<APITag>::LoadWithExecutor(const Path& sceneFileName, tf::Executor* executor) {
		g_LoadingStats.ObjectsLoaded = 0;
		g_LoadingStats.ObjectsTotal = 0;

		this->m_SceneGraph = MakeShared<SceneGraph<APITag>>();

		if (".gltf" == sceneFileName.extension() || ".glb" == sceneFileName.extension()) {
			++g_LoadingStats.ObjectsTotal;

			this->m_Models.resize(1);

			this->LoadModelAsync(0, sceneFileName, executor);
			if (nullptr != executor)
				executor->wait_for_all();
			//auto 

			return true;

		}
		else {
			LOG_ERROR("Unknown file type");
			return false;
		}


	}

	template<RHI::APITagConcept APITag>
	inline void Scene<APITag>::LoadModelAsync(Uint32 Index, const Path& FileName, tf::Executor* excutor) {
		if (nullptr != excutor)
			excutor->async(
				[this, Index, excutor, FileName](void) {
					SceneImportResult<APITag> Re{};
					this->m_GLTFImporter->Load(FileName, *this->m_TextureCache, g_LoadingStats, excutor, Re);
					++g_LoadingStats.ObjectsLoaded;
					this->m_Models[Index] = Re;
				}
			);
		else {
			SceneImportResult<APITag> Re{};
			this->m_GLTFImporter->Load(FileName, *this->m_TextureCache, g_LoadingStats, nullptr, Re);
			++g_LoadingStats.ObjectsLoaded;
			this->m_Models[Index] = Re;
		}
	}

}