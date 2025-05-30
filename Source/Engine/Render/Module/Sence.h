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
#include "Engine/Render/Module/SceneTypes.h"
#include "Engine/Engine/Module/SceneGraph.h"
#include "Engine/Render/Module/GLTFImporter.h"

#include "Shader/material_cb.h"
#include "Shader/bindless.h"
#include "Shader/skinning_cb.h"

#endif // PARTING_MODULE_BUILD

namespace Parting {

	void AppendBufferRange(RHI::RHIBufferRange& range, Uint64 size, Uint64& currentBufferSize) {
		range.Offset = currentBufferSize;
		range.ByteSize = Math::Align(size, static_cast<Uint64>(16));
		currentBufferSize += range.ByteSize;
	}

	template<RHI::APITagConcept APITag>
	class Scene final {
		using Imp_Device = typename RHI::RHITypeTraits<APITag>::Imp_Device;
		using Imp_Shader = typename RHI::RHITypeTraits<APITag>::Imp_Shader;
		using Imp_Buffer = typename RHI::RHITypeTraits<APITag>::Imp_Buffer;
		using Imp_InputLayout = typename RHI::RHITypeTraits<APITag>::Imp_InputLayout;
		using Imp_BindingLayout = typename RHI::RHITypeTraits<APITag>::Imp_BindingLayout;
		using Imp_ComputePipeline = typename RHI::RHITypeTraits<APITag>::Imp_ComputePipeline;
		using Imp_CommandList = typename RHI::RHITypeTraits<APITag>::Imp_CommandList;

	private:
		struct Resource final {
			Vector<MaterialConstants> MaterialData;
			Vector<GeometryData> GeometryData;
			Vector<InstanceData> InstanceData;
		};

	public:
		Scene(Imp_Device* device, ShaderFactory<APITag>& shaderFactory, SharedPtr<IFileSystem> fs, SharedPtr<TextureCache<APITag>> textureCache);
		~Scene(void) = default;

	public:

		STDNODISCARD SharedPtr<SceneGraph<APITag>> Get_SceneGraph(void) const { return this->m_SceneGraph; }
		STDNODISCARD Imp_Buffer* Get_MaterialBuffer(void) const { return this->m_MaterialBuffer; }
		STDNODISCARD Imp_Buffer* Get_GeometryBuffer(void) const { return this->m_GeometryBuffer; }
		STDNODISCARD Imp_Buffer* Get_InstanceBuffer(void) const { return this->m_InstanceBuffer; }

	public:

		bool Load(const Path& JsonFileName);

		bool LoadWithExecutor(const Path& sceneFileName, tf::Executor* executor);

		void LoadModelAsync(Uint32 Index, const Path& FileName, tf::Executor* excutor);

		void FinishedLoading(Uint32 frameIndex);

		void CreateMeshBuffers(Imp_CommandList* commandList);

		auto CreateGeometryBuffer(void) -> RHI::RefCountPtr<Imp_Buffer>;

		auto CreateMaterialBuffer(void) -> RHI::RefCountPtr<Imp_Buffer>;

		auto CreateInstanceBuffer(void) -> RHI::RefCountPtr<Imp_Buffer>;

		auto CreateMaterialConstantBuffer(const String& debugName) -> RHI::RefCountPtr<Imp_Buffer>;

		void UpdateMaterial(const SharedPtr<Material<APITag>>& material) { material->FillConstantBuffer(this->m_Resources->MaterialData[material->MaterialID]); }

		void UpdateGeometry(const SharedPtr<MeshInfo<APITag>>& mesh) {
			for (const auto& geometry : mesh->Geometries) {
				Uint32 indexOffset{ mesh->IndexOffset + geometry->IndexOffsetInMesh };
				Uint32 vertexOffset{ mesh->VertexOffset + geometry->VertexOffsetInMesh };

				GeometryData& gdata{ this->m_Resources->GeometryData[geometry->GlobalGeometryIndex] };
				gdata.NumIndices = geometry->NumIndices;
				gdata.NumVertices = geometry->NumVertices;
				gdata.IndexOffset = indexOffset * sizeof(Uint32);
				gdata.PositionOffset = mesh->Buffers->HasAttribute(RHI::RHIVertexAttribute::Position) ? Uint32(vertexOffset * sizeof(Math::VecF3) + mesh->Buffers->Get_VertexBufferRange(RHI::RHIVertexAttribute::Position).Offset) : ~0u;
				gdata.PrevPositionOffset = mesh->Buffers->HasAttribute(RHI::RHIVertexAttribute::PrevPosition) ? Uint32(vertexOffset * sizeof(Math::VecF3) + mesh->Buffers->Get_VertexBufferRange(RHI::RHIVertexAttribute::PrevPosition).Offset) : ~0u;
				gdata.TexCoord1Offset = mesh->Buffers->HasAttribute(RHI::RHIVertexAttribute::TexCoord1) ? Uint32(vertexOffset * sizeof(Math::VecF2) + mesh->Buffers->Get_VertexBufferRange(RHI::RHIVertexAttribute::TexCoord1).Offset) : ~0u;
				gdata.TexCoord2Offset = mesh->Buffers->HasAttribute(RHI::RHIVertexAttribute::TexCoord2) ? Uint32(vertexOffset * sizeof(Math::VecF2) + mesh->Buffers->Get_VertexBufferRange(RHI::RHIVertexAttribute::TexCoord2).Offset) : ~0u;
				gdata.NormalOffset = mesh->Buffers->HasAttribute(RHI::RHIVertexAttribute::Normal) ? Uint32(vertexOffset * sizeof(Uint32) + mesh->Buffers->Get_VertexBufferRange(RHI::RHIVertexAttribute::Normal).Offset) : ~0u;
				gdata.TangentOffset = mesh->Buffers->HasAttribute(RHI::RHIVertexAttribute::Tangent) ? Uint32(vertexOffset * sizeof(Uint32) + mesh->Buffers->Get_VertexBufferRange(RHI::RHIVertexAttribute::Tangent).Offset) : ~0u;
				gdata.CurveRadiusOffset = mesh->Buffers->HasAttribute(RHI::RHIVertexAttribute::CurveRadius) ? Uint32(vertexOffset * sizeof(float) + mesh->Buffers->Get_VertexBufferRange(RHI::RHIVertexAttribute::CurveRadius).Offset) : ~0u;
				gdata.MaterialIndex = nullptr != geometry->Material ? geometry->Material->MaterialID : ~0u;
			}
		}

		void UpdateInstance(const SharedPtr<MeshInstance<APITag>>& instance) {
			SceneGraphNode<APITag>* node{ instance->Get_Node() };
			if (nullptr == node)
				return;

			InstanceData& idata{ this->m_Resources->InstanceData[instance->Get_InstanceIndex()] };
			Math::AffineToColumnMajor(node->Get_LocalToWorldTransformFloat(), idata.Transform);
			Math::AffineToColumnMajor(node->Get_PrevLocalToWorldTransformFloat(), idata.PrevTransform);

			const auto& mesh{ instance->Get_Mesh() };
			idata.FirstGeometryInstanceIndex = instance->Get_GeometryInstanceIndex();
			idata.FirstGeometryIndex = mesh->Geometries[0]->GlobalGeometryIndex;
			idata.NumGeometries = static_cast<Uint32>(mesh->Geometries.size());
			idata.Flags = 0u;

			if (mesh->Type == MeshType::CurveDisjointOrthogonalTriangleStrips)
				idata.Flags |= InstanceFlags_CurveDisjointOrthogonalTriangleStrips;
		}

		void UpdateSkinnedMeshes(Imp_CommandList* commandList, Uint32 frameIndex);

		void WriteMaterialBuffer(Imp_CommandList* commandList) const {
			commandList->WriteBuffer(
				this->m_MaterialBuffer.Get(),
				this->m_Resources->MaterialData.data(),
				this->m_Resources->MaterialData.size() * sizeof(typename decltype(this->m_Resources->MaterialData)::value_type)
			);
		}

		void WriteGeometryBuffer(Imp_CommandList* commandList) const {
			commandList->WriteBuffer(
				this->m_GeometryBuffer.Get(),
				this->m_Resources->GeometryData.data(),
				this->m_Resources->GeometryData.size() * sizeof(typename decltype(this->m_Resources->GeometryData)::value_type)
			);
		}

		void WriteInstanceBuffer(Imp_CommandList* commandList) const {
			commandList->WriteBuffer(
				this->m_InstanceBuffer.Get(),
				this->m_Resources->InstanceData.data(),
				this->m_Resources->InstanceData.size() * sizeof(typename decltype(this->m_Resources->InstanceData)::value_type)
			);
		}





		void RefreshSceneGraph(Uint32 frameIndex) {
			this->m_SceneStructureChanged = this->m_SceneGraph->HasPendingStructureChanges();
			this->m_SceneTransformsChanged = this->m_SceneGraph->HasPendingTransformChanges();
			this->m_SceneGraph->Refresh(frameIndex);
		}

		void RefreshBuffers(Imp_CommandList* commandList, Uint32 frameIndex);

		void Refresh(Imp_CommandList* commandList, Uint32 frameIndex) {
			this->RefreshSceneGraph(frameIndex);
			this->RefreshBuffers(commandList, frameIndex);
		}

	private:
		RHI::RefCountPtr<Imp_Device> m_Device;
		RHI::RefCountPtr<Imp_Shader> m_SkinningShader;
		RHI::RefCountPtr<Imp_BindingLayout> m_SkinningBindingLayout;
		RHI::RefCountPtr<Imp_ComputePipeline> m_SkinningPipeline;

		RHI::RefCountPtr<Imp_Buffer> m_MaterialBuffer;
		RHI::RefCountPtr<Imp_Buffer> m_GeometryBuffer;
		RHI::RefCountPtr<Imp_Buffer> m_InstanceBuffer;

		SharedPtr<IFileSystem> m_FS;
		SharedPtr<TextureCache<APITag>> m_TextureCache;

		SharedPtr<Scene::Resource> m_Resources;

		SharedPtr<GLTFImporter<APITag>> m_GLTFImporter;
		SharedPtr<SceneGraph<APITag>> m_SceneGraph;
		Vector<SceneImportResult<APITag>> m_Models;

		bool m_EnableBindlessResources{ false };
		bool m_SceneTransformsChanged{ false };
		bool m_SceneStructureChanged{ false };


	};

	template<RHI::APITagConcept APITag>
	inline Scene<APITag>::Scene(Imp_Device* device, ShaderFactory<APITag>& shaderFactory, SharedPtr<IFileSystem> fs, SharedPtr<TextureCache<APITag>> textureCache) :
		m_Device{ device },
		m_FS{ ::MoveTemp(fs) },
		m_TextureCache{ ::MoveTemp(textureCache) } {

		this->m_SkinningShader = shaderFactory.CreateShader("Parting/skinning_cs", "main", nullptr, RHI::RHIShaderType::Compute);

		this->m_SkinningBindingLayout = this->m_Device->CreateBindingLayout(RHI::RHIBindingLayoutDescBuilder{}
			.Set_Visibility(RHI::RHIShaderType::Compute)
			.AddBinding(RHI::RHIBindingLayoutItem::PushConstants(0, sizeof(SkinningConstants)))
			.AddBinding(RHI::RHIBindingLayoutItem::RawBuffer_SRV(0))
			.AddBinding(RHI::RHIBindingLayoutItem::RawBuffer_SRV(1))
			.AddBinding(RHI::RHIBindingLayoutItem::RawBuffer_UAV(0))
			.Build()
		);

		this->m_SkinningPipeline = this->m_Device->CreateComputePipeline(RHI::RHIComputePipelineDescBuilder<APITag>{}
		.Set_CS(this->m_SkinningShader)
			.AddBindingLayout(this->m_SkinningBindingLayout)
			.Build()
			);

		this->m_Resources = MakeShared<Resource>();

		this->m_GLTFImporter = MakeShared<GLTFImporter<APITag>>(this->m_FS);
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

			if (nullptr == this->m_Models[0].RootNode)
				return false;

			this->m_SceneGraph->Set_RootNode(this->m_Models[0].RootNode);
		}
		else {
			LOG_ERROR("Unknown file type");
			return false;
		}

		return true;
	}

	template<RHI::APITagConcept APITag>
	inline void Scene<APITag>::LoadModelAsync(Uint32 Index, const Path& FileName, tf::Executor* excutor) {
		if (nullptr != excutor)
			excutor->async(
				[this, Index, excutor, FileName](void) {
					SceneImportResult<APITag> Re;
					this->m_GLTFImporter->Load(FileName, *this->m_TextureCache, g_LoadingStats, excutor, Re);
					++g_LoadingStats.ObjectsLoaded;
					this->m_Models[Index] = Re;
				}
			);
		else {
			SceneImportResult<APITag> Re;
			this->m_GLTFImporter->Load(FileName, *this->m_TextureCache, g_LoadingStats, nullptr, Re);
			++g_LoadingStats.ObjectsLoaded;
			this->m_Models[Index] = Re;
		}
	}

	template<RHI::APITagConcept APITag>
	inline void Scene<APITag>::FinishedLoading(Uint32 frameIndex) {
		RHI::RefCountPtr<Imp_CommandList> commandList{ this->m_Device->CreateCommandList() };
		commandList->Open();

		this->CreateMeshBuffers(commandList);
		this->Refresh(commandList, frameIndex);

		commandList->Close();
		this->m_Device->ExecuteCommandList(commandList);
	}

	template<RHI::APITagConcept APITag>
	inline void Scene<APITag>::CreateMeshBuffers(Imp_CommandList* commandList) {
		for (const auto& mesh : this->m_SceneGraph->Get_Meshes()) {
			const SharedPtr<BufferGroup<APITag>>& buffers{ mesh->Buffers };

			if (nullptr == buffers)
				continue;

			if (!buffers->IndexData.empty() && nullptr == buffers->IndexBuffer) {
				RHI::RHIBufferDesc bufferDesc{
					.ByteSize{ buffers->IndexData.size() * sizeof(typename decltype(buffers->IndexData)::value_type) },
					.DebugName{ _W("IndexBuffer") },
					.Format{ RHI::RHIFormat::R32_UINT },
					.CanHaveTypedViews{ true },
					.CanHaveRawViews{ true },
					.IsIndexBuffer{ true }
				};

				buffers->IndexBuffer = this->m_Device->CreateBuffer(bufferDesc);

				commandList->BeginTrackingBufferState(buffers->IndexBuffer, RHI::RHIResourceState::Common);

				commandList->WriteBuffer(buffers->IndexBuffer, buffers->IndexData.data(), buffers->IndexData.size() * sizeof(typename decltype(buffers->IndexData)::value_type));
				buffers->IndexData.clear();//NOTE : do not use "swap" func... gcc/clang/msvc is euqal to "clear" 
				buffers->IndexData.shrink_to_fit();

				RHI::RHIResourceState state{ RHI::RHIResourceState::IndexBuffer | RHI::RHIResourceState::ShaderResource };

				commandList->SetPermanentBufferState(buffers->IndexBuffer, state);
				commandList->CommitBarriers();
			}

			if (nullptr == buffers->VertexBuffer) {
				RHI::RHIBufferDesc bufferDesc{
					/*	.ByteSize{ 0 },*/
						.DebugName{ _W("VertexBuffer") },
						.CanHaveTypedViews{ true },
						.CanHaveRawViews{ true },
						.IsVertexBuffer{ true }
				};

				if (!buffers->PositionData.empty()) {
					AppendBufferRange(
						buffers->Get_VertexBufferRange(RHI::RHIVertexAttribute::Position),
						buffers->PositionData.size() * sizeof(typename decltype(buffers->PositionData)::value_type),//NOTE : makesure no cv descriptor in here 
						bufferDesc.ByteSize
					);
				}

				if (!buffers->NormalData.empty()) {
					AppendBufferRange(
						buffers->Get_VertexBufferRange(RHI::RHIVertexAttribute::Normal),
						buffers->NormalData.size() * sizeof(typename decltype(buffers->NormalData)::value_type),
						bufferDesc.ByteSize
					);
				}

				if (!buffers->TangentData.empty()) {
					AppendBufferRange(
						buffers->Get_VertexBufferRange(RHI::RHIVertexAttribute::Tangent),
						buffers->TangentData.size() * sizeof(typename decltype(buffers->TangentData)::value_type),
						bufferDesc.ByteSize
					);
				}

				if (!buffers->Texcoord1Data.empty()) {
					AppendBufferRange(
						buffers->Get_VertexBufferRange(RHI::RHIVertexAttribute::TexCoord1),
						buffers->Texcoord1Data.size() * sizeof(typename decltype(buffers->Texcoord1Data)::value_type),
						bufferDesc.ByteSize
					);
				}

				if (!buffers->Texcoord2Data.empty()) {
					AppendBufferRange(
						buffers->Get_VertexBufferRange(RHI::RHIVertexAttribute::TexCoord2),
						buffers->Texcoord2Data.size() * sizeof(typename decltype(buffers->Texcoord2Data)::value_type),
						bufferDesc.ByteSize
					);
				}

				if (!buffers->WeightData.empty()) {
					AppendBufferRange(
						buffers->Get_VertexBufferRange(RHI::RHIVertexAttribute::JointWeights),
						buffers->WeightData.size() * sizeof(typename decltype(buffers->WeightData)::value_type),
						bufferDesc.ByteSize
					);
				}

				if (!buffers->JointData.empty()) {
					AppendBufferRange(
						buffers->Get_VertexBufferRange(RHI::RHIVertexAttribute::JointIndices),
						buffers->JointData.size() * sizeof(typename decltype(buffers->JointData)::value_type),
						bufferDesc.ByteSize
					);
				}

				if (!buffers->RadiusData.empty()) {
					AppendBufferRange(
						buffers->Get_VertexBufferRange(RHI::RHIVertexAttribute::CurveRadius),
						buffers->RadiusData.size() * sizeof(typename decltype(buffers->RadiusData)::value_type),
						bufferDesc.ByteSize
					);
				}

				buffers->VertexBuffer = this->m_Device->CreateBuffer(bufferDesc);

				commandList->BeginTrackingBufferState(buffers->VertexBuffer, RHI::RHIResourceState::Common);

				if (!buffers->PositionData.empty()) {
					const RHI::RHIBufferRange& range{ buffers->Get_VertexBufferRange(RHI::RHIVertexAttribute::Position) };
					commandList->WriteBuffer(buffers->VertexBuffer, buffers->PositionData.data(), range.ByteSize, range.Offset);
					buffers->PositionData.clear();
					buffers->PositionData.shrink_to_fit();
				}

				if (!buffers->NormalData.empty()) {
					const RHI::RHIBufferRange& range{ buffers->Get_VertexBufferRange(RHI::RHIVertexAttribute::Normal) };
					commandList->WriteBuffer(buffers->VertexBuffer, buffers->NormalData.data(), range.ByteSize, range.Offset);
					buffers->NormalData.clear();
					buffers->NormalData.shrink_to_fit();
				}

				if (!buffers->TangentData.empty()) {
					const RHI::RHIBufferRange& range{ buffers->Get_VertexBufferRange(RHI::RHIVertexAttribute::Tangent) };
					commandList->WriteBuffer(buffers->VertexBuffer, buffers->TangentData.data(), range.ByteSize, range.Offset);
					buffers->TangentData.clear();
					buffers->TangentData.shrink_to_fit();
				}

				if (!buffers->Texcoord1Data.empty()) {
					const RHI::RHIBufferRange& range{ buffers->Get_VertexBufferRange(RHI::RHIVertexAttribute::TexCoord1) };
					commandList->WriteBuffer(buffers->VertexBuffer, buffers->Texcoord1Data.data(), range.ByteSize, range.Offset);
					buffers->Texcoord1Data.clear();
					buffers->Texcoord1Data.shrink_to_fit();
				}

				if (!buffers->Texcoord2Data.empty()) {
					const RHI::RHIBufferRange& range{ buffers->Get_VertexBufferRange(RHI::RHIVertexAttribute::TexCoord2) };
					commandList->WriteBuffer(buffers->VertexBuffer, buffers->Texcoord2Data.data(), range.ByteSize, range.Offset);
					buffers->Texcoord2Data.clear();
					buffers->Texcoord2Data.shrink_to_fit();
				}

				if (!buffers->WeightData.empty()) {
					const RHI::RHIBufferRange& range{ buffers->Get_VertexBufferRange(RHI::RHIVertexAttribute::JointWeights) };
					commandList->WriteBuffer(buffers->VertexBuffer, buffers->WeightData.data(), range.ByteSize, range.Offset);
					buffers->WeightData.clear();
					buffers->WeightData.shrink_to_fit();
				}

				if (!buffers->JointData.empty()) {
					const RHI::RHIBufferRange& range{ buffers->Get_VertexBufferRange(RHI::RHIVertexAttribute::JointIndices) };
					commandList->WriteBuffer(buffers->VertexBuffer, buffers->JointData.data(), range.ByteSize, range.Offset);
					buffers->JointData.clear();
					buffers->JointData.shrink_to_fit();
				}

				if (!buffers->RadiusData.empty()) {
					const RHI::RHIBufferRange& range{ buffers->Get_VertexBufferRange(RHI::RHIVertexAttribute::CurveRadius) };
					commandList->WriteBuffer(buffers->VertexBuffer, buffers->RadiusData.data(), range.ByteSize, range.Offset);
					buffers->RadiusData.clear();
					buffers->RadiusData.shrink_to_fit();
				}

				RHI::RHIResourceState state{ RHI::RHIResourceState::VertexBuffer | RHI::RHIResourceState::ShaderResource };

				commandList->SetPermanentBufferState(buffers->VertexBuffer, state);
				commandList->CommitBarriers();
			}
		}

		for (const auto& skinnedInstance : this->m_SceneGraph->Get_SkinnedMeshInstances()) {
			const auto& skinnedMesh{ skinnedInstance->Get_Mesh() };

			if (nullptr == skinnedMesh->Buffers) {
				skinnedMesh->Buffers = MakeShared<BufferGroup<APITag>>();

				Uint32 totalVertices{ skinnedMesh->TotalVertices };

				skinnedMesh->Buffers->IndexBuffer = skinnedInstance->Get_PrototypeMesh()->Buffers->IndexBuffer;

				const auto& prototypeBuffers{ skinnedInstance->Get_PrototypeMesh()->Buffers };
				const auto& skinnedBuffers{ skinnedMesh->Buffers };

				Uint64 skinnedVertexBufferSize{ 0 };
				ASSERT(prototypeBuffers->HasAttribute(RHI::RHIVertexAttribute::Position));

				AppendBufferRange(
					skinnedBuffers->Get_VertexBufferRange(RHI::RHIVertexAttribute::Position),
					totalVertices * sizeof(Math::VecF3),//TODO :type
					skinnedVertexBufferSize
				);

				AppendBufferRange(
					skinnedBuffers->Get_VertexBufferRange(RHI::RHIVertexAttribute::PrevPosition),
					totalVertices * sizeof(Math::VecF3),
					skinnedVertexBufferSize
				);

				if (prototypeBuffers->HasAttribute(RHI::RHIVertexAttribute::Normal)) {
					AppendBufferRange(
						skinnedBuffers->Get_VertexBufferRange(RHI::RHIVertexAttribute::Normal),
						totalVertices * sizeof(Uint32),
						skinnedVertexBufferSize
					);
				}

				if (prototypeBuffers->HasAttribute(RHI::RHIVertexAttribute::Tangent)) {
					AppendBufferRange(
						skinnedBuffers->Get_VertexBufferRange(RHI::RHIVertexAttribute::Tangent),
						totalVertices * sizeof(Uint32),
						skinnedVertexBufferSize
					);
				}

				if (prototypeBuffers->HasAttribute(RHI::RHIVertexAttribute::TexCoord1)) {
					AppendBufferRange(
						skinnedBuffers->Get_VertexBufferRange(RHI::RHIVertexAttribute::TexCoord1),
						totalVertices * sizeof(Math::VecF2),
						skinnedVertexBufferSize
					);
				}

				if (prototypeBuffers->HasAttribute(RHI::RHIVertexAttribute::TexCoord2)) {
					AppendBufferRange(
						skinnedBuffers->Get_VertexBufferRange(RHI::RHIVertexAttribute::TexCoord2),
						totalVertices * sizeof(Math::VecF2),
						skinnedVertexBufferSize
					);
				}

				RHI::RHIBufferDesc bufferDesc{
					.ByteSize{ skinnedVertexBufferSize },
					.DebugName{ _W("SkinnedVertexBuffer") },
					.CanHaveUAVs{ true },
					.CanHaveTypedViews{ true },
					.CanHaveRawViews{ true },
					.IsVertexBuffer{ true },
					.InitialState{ RHI::RHIResourceState::Common },
					.KeepInitialState{ true }
				};

				skinnedBuffers->VertexBuffer = this->m_Device->CreateBuffer(bufferDesc);
			}

			if (nullptr == skinnedInstance->JointBuffer) {
				RHI::RHIBufferDesc jointBufferDesc{
					.ByteSize{ skinnedInstance->Joints.size() * sizeof(typename decltype(skinnedInstance->Joints)::value_type)  },
					.DebugName{ _W("JointBuffer") },
					.CanHaveRawViews{ true },
					.InitialState{ RHI::RHIResourceState::ShaderResource },
					.KeepInitialState{ true }
				};
				skinnedInstance->JointBuffer = this->m_Device->CreateBuffer(jointBufferDesc);
			}

			if (nullptr == skinnedInstance->SkinningBindingSet) {
				const auto& prototypeBuffers{ skinnedInstance->Get_PrototypeMesh()->Buffers };
				const auto& skinnedBuffers{ skinnedInstance->Get_Mesh()->Buffers };

				skinnedInstance->SkinningBindingSet = this->m_Device->CreateBindingSet(
					RHI::RHIBindingSetDescBuilder<APITag>{}
				.AddBinding(RHI::RHIBindingSetItem<APITag>::PushConstants(0, sizeof(SkinningConstants)))
					.AddBinding(RHI::RHIBindingSetItem<APITag>::RawBuffer_SRV(0, prototypeBuffers->VertexBuffer))
					.AddBinding(RHI::RHIBindingSetItem<APITag>::RawBuffer_SRV(1, skinnedInstance->JointBuffer))
					.AddBinding(RHI::RHIBindingSetItem<APITag>::RawBuffer_UAV(0, skinnedBuffers->VertexBuffer))
					.Build(),
					this->m_SkinningBindingLayout);
			}
		}
	}

	template<RHI::APITagConcept APITag>
	inline auto Scene<APITag>::CreateGeometryBuffer(void) -> RHI::RefCountPtr<Imp_Buffer> {
		RHI::RHIBufferDesc bufferDesc{
			.ByteSize{ sizeof(typename decltype(this->m_Resources->GeometryData)::value_type) * this->m_Resources->GeometryData.size() },
			.StructStride { sizeof(typename decltype(this->m_Resources->GeometryData)::value_type) },
			.DebugName{ _W("BindlessGeometry") },
			.CanHaveUAVs{ true },
			.CanHaveRawViews{ true },
			.InitialState{ RHI::RHIResourceState::ShaderResource },
			.KeepInitialState{ true }
		};

		return this->m_Device->CreateBuffer(bufferDesc);
	}

	template<RHI::APITagConcept APITag>
	inline auto Scene<APITag>::CreateMaterialBuffer(void) -> RHI::RefCountPtr<Imp_Buffer> {
		RHI::RHIBufferDesc bufferDesc{
			.ByteSize{ sizeof(typename decltype(this->m_Resources->MaterialData)::value_type) * this->m_Resources->MaterialData.size() },
			.StructStride { sizeof(typename decltype(this->m_Resources->MaterialData)::value_type) },
			.DebugName{ _W("BindlessMaterials") },
			.CanHaveUAVs{ true },
			.CanHaveRawViews{ true },
			.InitialState{ RHI::RHIResourceState::ShaderResource },
			.KeepInitialState{ true }
		};

		return this->m_Device->CreateBuffer(bufferDesc);
	}

	template<RHI::APITagConcept APITag>
	inline auto Scene<APITag>::CreateInstanceBuffer(void) -> RHI::RefCountPtr<Imp_Buffer> {
		RHI::RHIBufferDesc bufferDesc{
			.ByteSize{ sizeof(typename decltype(this->m_Resources->InstanceData)::value_type) * this->m_Resources->InstanceData.size() },
			.StructStride { sizeof(typename decltype(this->m_Resources->InstanceData)::value_type) },
			.DebugName{ _W("Instances") },
			.CanHaveUAVs{ true },
			.CanHaveRawViews{ true },
			.InitialState{ RHI::RHIResourceState::ShaderResource },
			.KeepInitialState{ true }
		};

		return this->m_Device->CreateBuffer(bufferDesc);
	}

	template<RHI::APITagConcept APITag>
	inline auto Scene<APITag>::CreateMaterialConstantBuffer(const String& debugName) -> RHI::RefCountPtr<Imp_Buffer> {
		RHI::RHIBufferDesc bufferDesc{
			.ByteSize{ sizeof(typename decltype(this->m_Resources->MaterialData)::value_type) },
			/*	.DebugName{ debugName },*/
				.IsConstantBuffer{ true },
				.InitialState{ RHI::RHIResourceState::ShaderResource },
				.KeepInitialState{ true }
		};
		return this->m_Device->CreateBuffer(bufferDesc);
	}




	template<RHI::APITagConcept APITag>
	inline void Scene<APITag>::UpdateSkinnedMeshes(Imp_CommandList* commandList, Uint32 frameIndex) {

		bool skinningMarkerPlaced{ false };

		Vector<Math::MatF44> jointMatrices;
		for (const auto& skinnedInstance : this->m_SceneGraph->Get_SkinnedMeshInstances()) {

			ASSERT(false);
		}
	}



	template<RHI::APITagConcept APITag>
	inline void Scene<APITag>::RefreshBuffers(Imp_CommandList* commandList, Uint32 frameIndex) {
		bool materialsChanged{ false };

		if (this->m_SceneStructureChanged)
			this->CreateMeshBuffers(commandList);

		const Uint32 allocationGranularity{ 1024 };
		bool arraysAllocated{ false };

		if (this->m_EnableBindlessResources && this->m_SceneGraph->Get_GeometryCount() > this->m_Resources->GeometryData.size()) {
			this->m_Resources->GeometryData.resize(Math::Align<Uint64>(this->m_SceneGraph->Get_GeometryCount(), allocationGranularity));
			this->m_GeometryBuffer = this->CreateGeometryBuffer();
			arraysAllocated = true;
		}

		if (this->m_SceneGraph->Get_Materials().size() > this->m_Resources->MaterialData.size()) {
			this->m_Resources->MaterialData.resize(Math::Align<Uint64>(this->m_SceneGraph->Get_Materials().size(), allocationGranularity));
			if (this->m_EnableBindlessResources)
				this->m_MaterialBuffer = this->CreateMaterialBuffer();
			arraysAllocated = true;
		}

		if (this->m_SceneGraph->Get_MeshInstances().size() > this->m_Resources->InstanceData.size()) {
			this->m_Resources->InstanceData.resize(Math::Align<Uint64>(this->m_SceneGraph->Get_MeshInstances().size(), allocationGranularity));
			this->m_InstanceBuffer = this->CreateInstanceBuffer();
			arraysAllocated = true;
		}

		for (const auto& material : m_SceneGraph->Get_Materials()) {
			if (material->Dirty || this->m_SceneStructureChanged || arraysAllocated)
				this->UpdateMaterial(material);

			if (nullptr == material->MaterialConstants) {
				material->MaterialConstants = this->CreateMaterialConstantBuffer(material->Name);
				material->Dirty = true;
			}

			if (material->Dirty) {
				commandList->WriteBuffer(material->MaterialConstants,
					&this->m_Resources->MaterialData[material->MaterialID],
					sizeof(typename decltype(this->m_Resources->MaterialData)::value_type)
				);

				material->Dirty = false;
				materialsChanged = true;
			}
		}

		if (!this->m_Resources->GeometryData.empty()) {
			Uint32 geometryResourceIndex{ 0 };
			for (const auto& mesh : this->m_SceneGraph->Get_Meshes()) {
				if (arraysAllocated)
					break;

				for (const auto& geometry : mesh->Geometries) {
					if (geometry->NumIndices != this->m_Resources->GeometryData[geometryResourceIndex].NumIndices) {
						arraysAllocated = true;
						break;
					}
					++geometryResourceIndex;
				}
			}
		}

		if (this->m_SceneStructureChanged || arraysAllocated) {
			for (const auto& mesh : this->m_SceneGraph->Get_Meshes()) {
				mesh->Buffers->InstanceBuffer = this->m_InstanceBuffer;

				if (this->m_EnableBindlessResources)
					this->UpdateGeometry(mesh);
			}

			if (this->m_EnableBindlessResources)
				this->WriteGeometryBuffer(commandList);
		}

		if (this->m_SceneStructureChanged || this->m_SceneTransformsChanged || arraysAllocated) {
			for (const auto& instance : this->m_SceneGraph->Get_MeshInstances())
				this->UpdateInstance(instance);

			this->WriteInstanceBuffer(commandList);
		}

		if (this->m_EnableBindlessResources && (materialsChanged || this->m_SceneStructureChanged || arraysAllocated))
			this->WriteMaterialBuffer(commandList);

		this->UpdateSkinnedMeshes(commandList, frameIndex);
	}
}