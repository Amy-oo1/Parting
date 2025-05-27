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


PARTING_MODULE(DrawStrategy)


#else
#pragma once

#include "Core/ModuleBuild.h"


#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"
//Global

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#include "Core/Algorithm/Module/Algorithm.h"
#include "Core/Container/Module/Container.h"
#include "Core/VectorMath/Module/VectorMath.h"
#include "Core/Logger/Module/Logger.h"


#include "Engine/Engine/Module/CommonRenderPasses.h"
#include "Engine/Engine/Module/FrameBufferFactory.h"
#include "Engine/Engine/Module/ShaderFactory.h"
#include "Engine/Render/Module/MaterialBindingCache.h"
#include "Engine/Render/Module/View.h"


#include "Engine/Engine/Module/SceneGraph.h"

#endif // PARTING_MODULE_BUILD

namespace Parting {

	template<RHI::APITagConcept APITag>
	struct DrawItem final {
		const MeshInstance<APITag>* Instance;
		const MeshInfo<APITag>* Mesh;
		const MeshGeometry<APITag>* Geometry;
		const Material<APITag>* Material;
		const BufferGroup<APITag>* Buffers;
		float DistanceToCamera;
		RHI::RHIRasterCullMode CullMode;

		static bool CompareDrawItemsOpaque(const DrawItem<APITag>* a, const DrawItem<APITag>* b) {
			if (a->Material != b->Material)
				return a->Material < b->Material;

			if (a->Buffers != b->Buffers)
				return a->Buffers < b->Buffers;

			if (a->Mesh != b->Mesh)
				return a->Mesh < b->Mesh;

			return a->Instance < b->Instance;
		}

	};

	template<RHI::APITagConcept APITag>
	class IDrawStrategy {
	public:
		IDrawStrategy(void) = default;
		virtual ~IDrawStrategy(void) = default;

	public:
		virtual void PrepareForView(const SharedPtr<SceneGraphNode<APITag>>& rootNode, const IView& view) = 0;

		virtual const DrawItem<APITag>* Get_NextItem(void) = 0;
	};

	template<RHI::APITagConcept APITag>
	class PassthroughDrawStrategy final : public IDrawStrategy<APITag> {
	public:
		PassthroughDrawStrategy(void) = default;
		~PassthroughDrawStrategy(void) = default;

	public:
		void Set_Data(const DrawItem<APITag>* data, Uint64 count) {
			this->m_Data = data;
			this->m_Count = count;
		}

	private:
		const DrawItem<APITag>* m_Data{ nullptr };
		Uint64 m_Count{ 0 };

	private:
		void PrepareForView(const SharedPtr<SceneGraphNode<APITag>>& rootNode, const IView& view) override {}

		const DrawItem<APITag>* Get_NextItem(void) override {
			if (this->m_Count > 0) {
				--this->m_Count;
				return this->m_Data++;
			}
			else
				return this->m_Data = nullptr;
		}

	};

	template<RHI::APITagConcept APITag>
	class InstancedOpaqueDrawStrategy final : public IDrawStrategy<APITag> {
	public:
		InstancedOpaqueDrawStrategy(void) = default;
		~InstancedOpaqueDrawStrategy(void) = default;

	public:
		void FillChunk(void);

		STDNODISCARD Uint64 Get_ChunkSize(void) const { return this->m_ChunkSize; }
		void Set_ChunkSize(Uint64 size) { this->m_ChunkSize = Math::Max<Uint64>(size, 1u); }

	private:
		Math::Frustum m_ViewFrustum;
		SceneGraphIterator<APITag> m_Walker;
		Vector<DrawItem<APITag>> m_InstanceChunk;
		Vector<const DrawItem<APITag>*> m_InstancePtrChunk;
		Uint64 m_ReadPtr{ 0 };
		Uint64 m_ChunkSize{ 128 };

	private:
		void PrepareForView(const SharedPtr<SceneGraphNode<APITag>>& rootNode, const IView& view) override;

		const DrawItem<APITag>* Get_NextItem(void) override;

	};

	template<RHI::APITagConcept APITag>
	class TransparentDrawStrategy : public IDrawStrategy<APITag> {
	public:
		TransparentDrawStrategy(void) = default;
		~TransparentDrawStrategy(void) = default;

	public:
		bool DrawDoubleSidedMaterialsSeparately{ true };

	private:
		Vector<DrawItem<APITag>> m_InstancesToDraw;
		Vector<const DrawItem<APITag>*> m_InstancePtrsToDraw;
		Uint64 m_ReadPtr{ 0 };



	private:
		void PrepareForView(const SharedPtr<SceneGraphNode<APITag>>& rootNode, const IView& view) override;

		const DrawItem<APITag>* Get_NextItem(void) override;
	};



	template<RHI::APITagConcept APITag>
	inline void InstancedOpaqueDrawStrategy<APITag>::FillChunk(void) {
		this->m_InstanceChunk.resize(this->m_ChunkSize);

		DrawItem<APITag>* writePtr{ this->m_InstanceChunk.data() };
		Uint64 itemCount{ 0 };

		while (this->m_Walker && itemCount < this->m_ChunkSize) {
			auto relevantContentFlags{ SceneContentFlags::OpaqueMeshes | SceneContentFlags::AlphaTestedMeshes };
			bool subgraphContentRelevant{ (this->m_Walker->Get_SubgraphContentFlags() & relevantContentFlags) != SceneContentFlags::None };
			bool nodeContentsRelevant{ (this->m_Walker->Get_LeafContentFlags() & relevantContentFlags) != SceneContentFlags::None };

			bool nodeVisible{ false };
			if (subgraphContentRelevant) {
				nodeVisible = this->m_ViewFrustum.IntersectsWith(this->m_Walker->Get_GlobalBoundingBox());

				if (nodeVisible && nodeContentsRelevant) {
					if (auto meshInstance{ dynamic_cast<MeshInstance<APITag>*>(this->m_Walker->Get_Leaf().get()) }; nullptr != meshInstance) {
						const auto mesh{ meshInstance->Get_Mesh().get() };

						Uint64 requiredChunkSize{ itemCount + mesh->Geometries.size() };
						if (this->m_InstanceChunk.size() < requiredChunkSize) {
							this->m_InstanceChunk.resize(requiredChunkSize);
							writePtr = this->m_InstanceChunk.data() + itemCount;
						}

						for (const auto& geometry : mesh->Geometries) {
							auto domain{ geometry->Material->Domain };
							if (domain != MaterialDomain::Opaque || domain != MaterialDomain::AlphaTested)//TODO :
								continue;

							if (mesh->Geometries.size() > 1 && nullptr == mesh->SkinPrototype) {
								Math::BoxF3 geometryGlobalBoundingBox{ geometry->ObjectSpaceBounds * this->m_Walker->Get_LocalToWorldTransformFloat() };
								if (!this->m_ViewFrustum.IntersectsWith(geometryGlobalBoundingBox))
									continue;
							}

							auto& item{ *writePtr };
							item.Instance = meshInstance;
							item.Mesh = mesh;
							item.Geometry = geometry.get();
							item.Material = geometry->Material.get();
							item.Buffers = item.Mesh->Buffers.get();
							item.CullMode = (item.Material->DoubleSided) ? RHI::RHIRasterCullMode::None : RHI::RHIRasterCullMode::Back;
							item.DistanceToCamera = 0; // don't care

							++writePtr;
							++itemCount;
						}
					}
				}
			}

			m_Walker.Next(nodeVisible);
		}

		this->m_InstanceChunk.resize(itemCount);
		this->m_InstancePtrChunk.resize(itemCount);

		for (Uint64 Index = 0; Index < itemCount; ++Index)
			this->m_InstancePtrChunk[Index] = &this->m_InstanceChunk[Index];

		if (itemCount > 1)//TODO :
			::Sort(this->m_InstancePtrChunk.data(), this->m_InstancePtrChunk.data() + this->m_InstancePtrChunk.size(), DrawItem<APITag>::CompareDrawItemsOpaque);


		this->m_ReadPtr = 0;
	}

	template<RHI::APITagConcept APITag>
	inline void InstancedOpaqueDrawStrategy<APITag>::PrepareForView(const SharedPtr<SceneGraphNode<APITag>>& rootNode, const IView& view) {
		this->m_Walker = SceneGraphIterator<APITag>{ rootNode.get() };
		this->m_ViewFrustum = view.Get_ViewFrustum();
		this->m_InstanceChunk.clear();
		this->m_ReadPtr = 0;
	}

	template<RHI::APITagConcept APITag>
	inline const DrawItem<APITag>* InstancedOpaqueDrawStrategy<APITag>::Get_NextItem(void) {
		if (this->m_ReadPtr >= this->m_InstancePtrChunk.size())
			this->FillChunk();

		if (this->m_InstancePtrChunk.empty())
			return nullptr;

		return this->m_InstancePtrChunk[this->m_ReadPtr++];
	}

	template<RHI::APITagConcept APITag>
	inline void TransparentDrawStrategy<APITag>::PrepareForView(const SharedPtr<SceneGraphNode<APITag>>& rootNode, const IView& view) {
		this->m_ReadPtr = 0;

		this->m_InstancesToDraw.clear();
		this->m_InstancePtrsToDraw.clear();

		Math::VecF3 viewOrigin{ view.Get_ViewOrigin() };
		Math::Frustum viewFrustum{ view.Get_ViewFrustum() };

		SceneGraphIterator walker{ rootNode.get() };
		while (walker) {
			auto relevantContentFlags{ SceneContentFlags::BlendedMeshes };
			bool subgraphContentRelevant{ (walker->Get_SubgraphContentFlags() & relevantContentFlags) != SceneContentFlags::None };
			bool nodeContentsRelevant{ (walker->Get_LeafContentFlags() & relevantContentFlags) != SceneContentFlags::None };

			bool nodeVisible{ false };
			if (subgraphContentRelevant) {
				nodeVisible = viewFrustum.IntersectsWith(walker->Get_GlobalBoundingBox());

				if (nodeVisible && nodeContentsRelevant) {
					if (auto meshInstance{ dynamic_cast<MeshInstance<APITag>*>(walker->Get_Leaf().get()) }; nullptr != meshInstance) {
						const auto mesh{ meshInstance->Get_Mesh().get() };
						for (const auto& geometry : mesh->Geometries) {
							const auto& material{ geometry->Material };
							if (material->Domain == MaterialDomain::Opaque || material->Domain == MaterialDomain::AlphaTested)
								continue;

							Math::BoxF3 geometryGlobalBoundingBox;
							if (mesh->Geometries.size() > 1 && nullptr != mesh->SkinPrototype) {
								geometryGlobalBoundingBox = geometry->ObjectSpaceBounds * walker->Get_LocalToWorldTransformFloat();
								if (!viewFrustum.IntersectsWith(geometryGlobalBoundingBox))
									continue;
							}
							else
								geometryGlobalBoundingBox = walker->Get_GlobalBoundingBox();

							DrawItem<APITag> item{};//TODO
							item.Instance = meshInstance;
							item.Mesh = mesh;
							item.Geometry = geometry.get();
							item.Material = geometry->Material.get();
							item.Buffers = mesh->Buffers.get();
							item.DistanceToCamera = Math::Length(geometryGlobalBoundingBox.Get_Center() - viewOrigin);
							if (material->DoubleSided) {
								if (this->DrawDoubleSidedMaterialsSeparately) {
									item.CullMode = RHI::RHIRasterCullMode::Front;
									this->m_InstancesToDraw.push_back(item);
									item.CullMode = RHI::RHIRasterCullMode::Back;
									this->m_InstancesToDraw.push_back(item);
								}
								else {
									item.CullMode = RHI::RHIRasterCullMode::None;
									this->m_InstancesToDraw.push_back(item);
								}
							}
							else {
								item.CullMode = RHI::RHIRasterCullMode::Back;
								this->m_InstancesToDraw.push_back(item);
							}
						}
					}
				}
			}

			walker.Next(nodeVisible);
		}

		if (this->m_InstancesToDraw.empty())
			return;

		this->m_InstancePtrsToDraw.resize(this->m_InstancesToDraw.size());

		for (Uint64 Index = 0; Index < this->m_InstancesToDraw.size(); ++Index)
			this->m_InstancePtrsToDraw[Index] = &this->m_InstancesToDraw[Index];

		if (this->m_InstancePtrsToDraw.size() > 1)
			::Sort(this->m_InstancePtrsToDraw.data(), this->m_InstancePtrsToDraw.data() + this->m_InstancePtrsToDraw.size(), DrawItem<APITag>::CompareDrawItemsOpaque);
	}

	template<RHI::APITagConcept APITag>
	inline const DrawItem<APITag>* TransparentDrawStrategy<APITag>::Get_NextItem(void) {
		if (this->m_ReadPtr >= this->m_InstancePtrsToDraw.size()) {
			this->m_InstancesToDraw.clear();
			this->m_InstancePtrsToDraw.clear();
			return nullptr;
		}

		return this->m_InstancePtrsToDraw[this->m_ReadPtr++];
	}

}