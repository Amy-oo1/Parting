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
#include "Core/VFS/Module/VFS.h"
#include "Core/Json/Module/Json.h"

#include "RHI/Module/RHI.h"
#include "D3D12RHI/Module/D3D12RHI.h"

#include "Engine/Render/Module/SceneTypes.h"

#include "Shader/light_cb.h"

#endif // PARTING_MODULE_BUILD#pragma once

namespace Parting {


	//shodwmap
	template<RHI::APITagConcept APITag>
	class IShadowMap;

	enum struct SceneContentFlags : Uint32 {
		None = 0,
		OpaqueMeshes = 0x01,
		AlphaTestedMeshes = 0x02,
		BlendedMeshes = 0x04,
		Lights = 0x08,
		Cameras = 0x10,
		Animations = 0x20
	};
	EXPORT_ENUM_CLASS_OPERATORS(SceneContentFlags);



	template<RHI::APITagConcept APITag>
	class SceneGraph;

	template<RHI::APITagConcept APITag>
	class SceneGraphNode;



	template<RHI::APITagConcept APITag>
	class SceneTypeFactory final {
	public:
		SceneTypeFactory(void) = default;
		~SceneTypeFactory(void) = default;

	private:

	};

	template<RHI::APITagConcept APITag>
	class SceneGraphLeaf :public NonCopyAndMoveAble {
		friend class SceneGraph<APITag>;
		friend class SceneGraphNode<APITag>;
	public:
		SceneGraphLeaf(void) = default;
		virtual ~SceneGraphLeaf(void) = default;

	public:
		STDNODISCARD SceneGraphNode<APITag>* Get_Node(void) const { return this->m_Node.lock().get(); }

		STDNODISCARD SharedPtr<SceneGraphNode<APITag>> Get_NodeSharedPtr(void) const { return this->m_Node.lock(); }

		STDNODISCARD const String& Get_Name(void) const {
			if (auto Node{ this->Get_Node() }; nullptr != Node)
				return Node->Get_Name();
			else {
				static const String EmptyName{};
				return EmptyName;
			}
		}

		void Set_Name(const String& name) const {
			if (auto Node{ this->Get_Node() }; nullptr != Node)
				Node->Set_Name(name);
			else
				LOG_ERROR("SceneGraphLeaf::Set_Name() - Node is nullptr");
		}
		/*	virtual void Load(const Json::Value& node) {}*/
		/*	virtual bool SetProperty(const std::string& name, const dm::float4& value) { return false; }*/

	public:
		STDNODISCARD virtual Math::BoxF3 Get_LocalBoundingBox(void) { return Math::BoxF3::Empty(); }

		STDNODISCARD virtual SceneContentFlags Get_ContentFlags(void) const { return SceneContentFlags::None; }

		virtual void Load(const JSON::Value& node) { LOG_ERROR("empty Imp"); }

		virtual bool Set_Property(const String& name, const Math::VecF4& value) { return false; }

		STDNODISCARD virtual SharedPtr<SceneGraphLeaf<APITag>> Clone(void) = 0;

	private:
		WeakPtr<SceneGraphNode<APITag>> m_Node;
	};

	template<RHI::APITagConcept APITag>
	class MeshInstance : public SceneGraphLeaf<APITag> {
		friend class SceneGraph<APITag>;
	public:
		explicit MeshInstance(SharedPtr<MeshInfo<APITag>> mesh) :
			SceneGraphLeaf<APITag>{},
			m_Mesh{ MoveTemp(mesh) }{
		}
		virtual ~MeshInstance(void) = default;

	public:
		STDNODISCARD const SharedPtr<MeshInfo<APITag>>& Get_Mesh(void) const { return this->m_Mesh; }

		STDNODISCARD Int32 Get_InstanceIndex(void) const { return this->m_InstanceIndex; }
		STDNODISCARD Int32 Get_GeometryInstanceIndex(void) const { return this->m_GeometryInstanceIndex; }
		STDNODISCARD Math::BoxF3 Get_LocalBoundingBox(void) override { return this->m_Mesh->ObjectSpaceBounds; }

	public:
		STDNODISCARD SharedPtr<SceneGraphLeaf<APITag>> Clone(void) override { return nullptr; }
		STDNODISCARD SceneContentFlags Get_ContentFlags(void) const override {
			/*using enum SceneContentFlags;*/
			if (nullptr == this->m_Mesh)
				return SceneContentFlags::None;

			SceneContentFlags flags{ SceneContentFlags::None };

			for (const auto& geometry : this->m_Mesh->Geometries) {
				if (nullptr == geometry->Material)
					continue;

				switch (geometry->Material->Domain) {
				case MaterialDomain::Opaque:flags |= SceneContentFlags::OpaqueMeshes; break;
				case MaterialDomain::AlphaTested:flags |= SceneContentFlags::AlphaTestedMeshes; break;
				default:flags |= SceneContentFlags::BlendedMeshes; break;
				}
			}

			return flags;
		}
		bool Set_Property(const String& name, const Math::VecF4& value) override { return false; }

	protected:
		SharedPtr<MeshInfo<APITag>> m_Mesh;

	private:
		Int32 m_InstanceIndex{ -1 };
		Int32 m_GeometryInstanceIndex{ -1 };
	};

	template<RHI::APITagConcept APITag>
	struct SkinnedMeshJoint final {
		WeakPtr<SceneGraphNode<APITag>> Node;
		Math::MatF44 InverseBindMatrix;
	};

	template<RHI::APITagConcept APITag>
	class SkinnedMeshInstance final : public MeshInstance<APITag> {
		using Imp_Buffer = typename RHI::RHITypeTraits<APITag>::Imp_Buffer;
		using Imp_BindingSet = typename RHI::RHITypeTraits<APITag>::Imp_BindingSet;
		friend class SceneGraph<APITag>;
	public:
		explicit SkinnedMeshInstance(SharedPtr<MeshInfo<APITag>> prototypeMesh) :
			MeshInstance<APITag>{ nullptr },
			m_PrototypeMesh{ MoveTemp(prototypeMesh) } {

			auto skinnedMesh{ MakeShared<MeshInfo<APITag>>() };
			skinnedMesh->Name = this->m_PrototypeMesh->Name;
			skinnedMesh->SkinPrototype = this->m_PrototypeMesh;

			skinnedMesh->ObjectSpaceBounds = this->m_PrototypeMesh->ObjectSpaceBounds;
			skinnedMesh->IndexOffset = this->m_PrototypeMesh->IndexOffset;
			skinnedMesh->TotalIndices = this->m_PrototypeMesh->TotalIndices;
			skinnedMesh->TotalVertices = this->m_PrototypeMesh->TotalVertices;

			skinnedMesh->Geometries.reserve(this->m_PrototypeMesh->Geometries.size());
			for (const auto& geometry : this->m_PrototypeMesh->Geometries) {
				SharedPtr<MeshGeometry<APITag>> newGeometry{ MakeShared<MeshGeometry<APITag>>() };
				*newGeometry = *geometry;
				skinnedMesh->Geometries.push_back(newGeometry);
			}

			this->m_Mesh = skinnedMesh;//TODO : move temp value
		}
		~SkinnedMeshInstance(void) = default;
	public:
		Vector<SkinnedMeshJoint<APITag>> Joints;
		RHI::RefCountPtr<Imp_Buffer> JointBuffer;
		RHI::RefCountPtr<Imp_BindingSet> SkinningBindingSet;
		bool SkinningInitialized{ false };

	private:
		SharedPtr<MeshInfo<APITag>> m_PrototypeMesh;
		Uint32 m_LastUpdateFrameIndex{ 0 };
		SharedPtr<SceneTypeFactory<APITag>> m_SceneTypeFactory;

	public:
		STDNODISCARD const SharedPtr<MeshInfo<APITag>>& Get_PrototypeMesh(void) const { return this->m_PrototypeMesh; }
		STDNODISCARD Uint32 Get_LastUpdateFrameIndex(void) const { return this->m_LastUpdateFrameIndex; }
		STDNODISCARD SharedPtr<SceneGraphLeaf<APITag>> Clone(void) override { return nullptr; }
	};


	// This leaf is attached to the joint nodes for a skeleton, and it makes them point at the mesh.
	// When the bones are updated, the mesh is flagged for rebuild.
	// Cannot do this through the graph because the skeleton can be separate from the mesh instance node.
	template<RHI::APITagConcept APITag>
	class SkinnedMeshReference final : public SceneGraphLeaf<APITag> {
		friend class SceneGraph<APITag>;
	public:
		explicit SkinnedMeshReference(SharedPtr<SkinnedMeshInstance<APITag>> instance) :
			SceneGraphLeaf<APITag>{},
			m_Instance{ instance } {
		}

	private:
		WeakPtr<SkinnedMeshInstance<APITag>> m_Instance;

	private:
		STDNODISCARD SharedPtr<SceneGraphLeaf<APITag>> Clone(void) override { return nullptr; }

	};



	template<RHI::APITagConcept APITag>
	class SceneCamera : public SceneGraphLeaf<APITag> {
	public:
		SceneCamera(void) = default;
		virtual ~SceneCamera(void) = default;

	public:
		STDNODISCARD Math::AffineF3 Get_ViewToWorldMatrix(void) const {
			if (auto Node{ this->Get_Node() }; nullptr != Node)
				return Math::Scaling(Math::VecF3{ 1.f, 1.f, -1.f }) * Math::AffineF3{ Node->Get_LocalToWorldTransform() };
			else {
				LOG_ERROR("SceneCamera::Get_ViewToWorldMatrix() - Node is nullptr");

				return Math::AffineF3::Identity();
			}
		}
		STDNODISCARD Math::AffineF3 Get_WorldToViewMatrix(void) const {
			if (auto Node{ this->Get_Node() }; nullptr != Node)
				return  Math::AffineF3{ Math::Inverse(Node->Get_LocalToWorldTransform()) } *Math::Scaling(Math::VecF3{ 1.f, 1.f, -1.f });
			else {
				LOG_ERROR("SceneCamera::Get_WorldToViewMatrix() - Node is nullptr");

				return Math::AffineF3::Identity();
			}
		}

	public:
		STDNODISCARD SceneContentFlags Get_ContentFlags(void) const override { return SceneContentFlags::Cameras; }
	};

	template<RHI::APITagConcept APITag>
	class PerspectiveCamera final : public SceneCamera<APITag> {
	public:
		PerspectiveCamera(void) = default;
		~PerspectiveCamera(void) = default;

	public:
		float ZNear{ 1.f };
		float VerticalFOV{ 1.f }; // in radians
		Optional<float> ZFar; // use reverse infinite projection if not specified
		Optional<float> AspectRatio;

	public:
		void Load(const JSON::Value& node) override {}
		bool Set_Property(const String& name, const Math::VecF4& value) override { return false; }

		STDNODISCARD SharedPtr<SceneGraphLeaf<APITag>> Clone(void) override {
			return nullptr;
		}

	};

	template<RHI::APITagConcept APITag>
	class OrthographicCamera final : public SceneCamera<APITag> {
	public:
		OrthographicCamera(void) = default;
		~OrthographicCamera(void) = default;

	public:
		float ZNear{ 0.f };
		float ZFar{ 1.f };
		float XMag{ 1.f };
		float YMag{ 1.f };

	public:
		void Load(const JSON::Value& node) override {}
		bool Set_Property(const String& name, const Math::VecF4& value) override { return false; }

		STDNODISCARD SharedPtr<SceneGraphLeaf<APITag>> Clone(void) override {
			return nullptr;
		}

	};



	template<RHI::APITagConcept APITag>
	class Light : public SceneGraphLeaf<APITag> {
	public:
		Light(void) = default;
		virtual ~Light(void) = default;

	public:
		SharedPtr<IShadowMap<APITag>> ShadowMap;
		Int32 ShadowChannel{ -1 };
		Math::VecF3 LightColor{ Color::White() };//NOTE : float3 to ignore alpha

	public:

		STDNODISCARD Math::VecD3  Get_Position(void) const;
		STDNODISCARD Math::VecD3 Get_Direction(void) const;

		void Set_Position(const Math::VecD3& position) const;
		void Set_Direction(const Math::VecD3& direction) const;

	public:
		STDNODISCARD SceneContentFlags Get_ContentFlags(void) const override { return SceneContentFlags::Lights; }
		bool Set_Property(const String& name, const Math::VecF4& value) override { return false; }

	public:
		STDNODISCARD  virtual Int32 Get_LightType(void) const = 0;

		virtual void FillLightConstants(LightConstants& lightConstants) const;
		virtual void Store(JSON::Value& node) const { LOG_ERROR("Empty Imp"); }

	};

	template<RHI::APITagConcept APITag>
	class DirectionalLight final : public Light<APITag> {
	public:
		DirectionalLight(void) = default;
		~DirectionalLight(void) = default;

	public:
		float Irradiance{ 1.f }; // Target illuminance (lm/m2) of surfaces lit by this light; multiplied by `color`.
		float AngularSize{ 0.f }; // Angular size of the light source, in degrees.
		Vector<SharedPtr<IShadowMap<APITag>>> PerObjectShadows;

	public:
		STDNODISCARD SharedPtr<SceneGraphLeaf<APITag>> Clone(void) override {
			return nullptr;
		}

		bool Set_Property(const String& name, const Math::VecF4& value) override { return false; }
		void Load(const JSON::Value& node) override {}

	public:
		STDNODISCARD Int32 Get_LightType(void) const override { return LightType_Directional; }

		void FillLightConstants(LightConstants& lightConstants) const override;

		void Store(JSON::Value& node) const override {}

	};

	template<RHI::APITagConcept APITag>
	class SpotLight final : public Light<APITag> {
	public:
		SpotLight(void) = default;
		~SpotLight(void) = default;

	public:
		float Intensity{ 1.f };			// Luminous intensity of the light (lm/sr) in its primary direction; multiplied by `color`.
		float Radius{ 0.f };			// Radius of the light sphere, in world units.
		float Range{ 0.f };				// Range of influence for the light. 0 means infinite range.
		float InnerAngle{ 180.f };		// Apex angle of the full-bright cone, in degrees; constant intensity inside the inner cone, smooth falloff between inside and outside.
		float OuterAngle{ 180.f };		// Apex angle of the light cone, in degrees - everything outside of that cone is dark.

	public:
		STDNODISCARD SharedPtr<SceneGraphLeaf<APITag>> Clone(void) override {
			return nullptr;
		}
		bool Set_Property(const String& name, const Math::VecF4& value) override { return false; }
		void Load(const JSON::Value& node) override {}

	public:
		STDNODISCARD Int32 Get_LightType(void) const override { return LightType_Spot; }

		void FillLightConstants(LightConstants& lightConstants) const override {}

		void Store(JSON::Value& node) const override {}

	};

	template<RHI::APITagConcept APITag>
	class PointLight final : public Light<APITag> {
	public:
		PointLight(void) = default;
		~PointLight(void) = default;

	public:
		float Intensity{ 1.f };		// Luminous intensity of the light (lm/sr); multiplied by `color`.
		float Radius{ 0.f };		// Radius of the light sphere, in world units.
		float Range{ 0.f };			// Range of influence for the light. 0 means infinite range.

	public:
		STDNODISCARD SharedPtr<SceneGraphLeaf<APITag>> Clone(void) override {
			return nullptr;
		}

		bool Set_Property(const String& name, const Math::VecF4& value) override { return false; }

		void Load(const JSON::Value& node) override {}


	public:

		STDNODISCARD Int32 Get_LightType(void) const override { return LightType_Point; }

		void FillLightConstants(LightConstants& lightConstants) const override {}

		void Store(JSON::Value& node) const override {}

	};


	enum class AnimationAttribute : Uint32 {
		Undefined,
		Scaling,
		Rotation,
		Translation,
		LeafProperty
	};

	template<RHI::APITagConcept APITag>
	class SceneGraphAnimation : public SceneGraphLeaf<APITag> {
	public:
		SceneGraphAnimation(void) = default;
		~SceneGraphAnimation(void) = default;

	public:


	private:
		STDNODISCARD SharedPtr<SceneGraphLeaf<APITag>> Clone(void) override {
			return nullptr;
		}
	};





	template<RHI::APITagConcept APITag>
	class SceneGraphNode final : public EnableSharedFromThis<SceneGraphNode<APITag>> {
		friend class SceneGraph<APITag>;
	public://TODO : out of class ....
		enum class DirtyFlags : Uint32 {
			None = 0,
			LocalTransform = 0x01,
			PrevTransform = 0x02,
			Leaf = 0x04,
			SubgraphStructure = 0x08,
			SubgraphTransforms = 0x10,
			SubgraphPrevTransforms = 0x20,
			SubgraphContentUpdate = 0x40,
			SubgraphMask = (SubgraphStructure | SubgraphTransforms | SubgraphPrevTransforms | SubgraphContentUpdate)
		};
		FRIEDND_ENUM_CLASS_OPERATORS(DirtyFlags)

	public:
		SceneGraphNode(void) = default;
		~SceneGraphNode(void) = default;

	public:
		STDNODISCARD const String& Get_Name(void) const { return this->m_Name; }

		STDNODISCARD SharedPtr<SceneGraph<APITag>> Get_Graph(void) const { return this->m_Graph.lock(); }
		STDNODISCARD SceneGraphNode<APITag>* Get_Parent(void) const { return this->m_Parent; }
		STDNODISCARD SceneGraphNode<APITag>* Get_Child(Uint64 index) const { return (index < this->m_Children.size()) ? this->m_Children[index].get() : nullptr; }
		STDNODISCARD Uint64 Get_NumChildren(void) const { return this->m_Children.size(); }
		STDNODISCARD const SharedPtr<SceneGraphLeaf<APITag>>& Get_Leaf(void) const { return this->m_Leaf; }

		/*STDNODISCARD const dm::dquat& GetRotation() const { return m_Rotation; }*/
		STDNODISCARD const Math::VecD3& Get_Scaling(void) const { return this->m_Scaling; }
		STDNODISCARD const Math::VecD3& Get_Translation(void) const { return this->m_Translation; }

		STDNODISCARD const Math::AffineD3& Get_LocalToParentTransform(void) const { return this->m_LocalTransform; }
		STDNODISCARD const Math::AffineD3& Get_LocalToWorldTransform(void) const { return this->m_GlobalTransform; }
		STDNODISCARD const Math::AffineF3& Get_LocalToWorldTransformFloat(void) const { return this->m_GlobalTransformFloat; }

		STDNODISCARD const Math::AffineD3& Get_PrevLocalToParentTransform(void) const { return this->m_PrevLocalTransform; }
		STDNODISCARD const Math::AffineD3& Get_PrevLocalToWorldTransform(void) const { return this->m_PrevGlobalTransform; }
		STDNODISCARD const Math::AffineF3& Get_PrevLocalToWorldTransformFloat(void) const { return this->m_PrevGlobalTransformFloat; }

		STDNODISCARD const Math::BoxF3& Get_GlobalBoundingBox(void) const { return this->m_GlobalBoundingBox; }

		STDNODISCARD DirtyFlags Get_DirtyFlags(void) const { return this->m_Dirty; }

		STDNODISCARD SceneContentFlags Get_LeafContentFlags(void) const { return this->m_LeafContent; }
		STDNODISCARD SceneContentFlags Get_SubgraphContentFlags(void) const { return this->m_SubgraphContent; }




		void Set_Name(const String& name) { this->m_Name = name; }

		void Set_Transform(const Math::VecD3* translation, const Math::QuatD* rotation, const Math::VecD3* scaling) {
			if (nullptr != rotation)
				this->m_Rotation = *rotation;
			if (nullptr != translation)
				this->m_Translation = *translation;
			if (nullptr != scaling)
				this->m_Scaling = *scaling;

			this->m_Dirty |= DirtyFlags::LocalTransform;
			this->m_HasLocalTransform = true;
			this->PropagateDirtyFlags(DirtyFlags::SubgraphTransforms);
		}
		void Set_Scaling(const Math::VecD3& scaling) { this->Set_Transform(nullptr, nullptr, &scaling); }
		void Set_Rotation(const Math::QuatD& rotation) { this->Set_Transform(nullptr, &rotation, nullptr); }
		void Set_Translation(const Math::VecD3& translation) { this->Set_Transform(&translation, nullptr, nullptr); }

		void Set_Leaf(const SharedPtr<SceneGraphLeaf<APITag>>& leaf);


		void UpdateLocalTransform(void) {
			Math::AffineD3 transform{ Math::Scaling(this->m_Scaling) };
			transform *= this->m_Rotation.ToAffine();
			transform *= Math::Translation(this->m_Translation);
			this->m_LocalTransform = transform;
		}
		void PropagateDirtyFlags(SceneGraphNode::DirtyFlags flags);


	private:


	private:
		String m_Name;

		WeakPtr<SceneGraph<APITag>> m_Graph;
		SceneGraphNode<APITag>* m_Parent{ nullptr };
		Vector<SharedPtr<SceneGraphNode<APITag>>> m_Children;
		SharedPtr<SceneGraphLeaf<APITag>> m_Leaf;
		DirtyFlags m_Dirty{ DirtyFlags::None };
		SceneContentFlags m_LeafContent{ SceneContentFlags::None };
		SceneContentFlags m_SubgraphContent{ SceneContentFlags::None };


		Math::AffineD3 m_LocalTransform{ Math::AffineD3::Identity() };
		Math::AffineD3 m_GlobalTransform{ Math::AffineD3::Identity() };
		Math::AffineF3 m_GlobalTransformFloat{ Math::AffineF3::Identity() };
		Math::AffineD3 m_PrevLocalTransform{ Math::AffineD3::Identity() };
		Math::AffineD3 m_PrevGlobalTransform{ Math::AffineD3::Identity() };
		Math::AffineF3 m_PrevGlobalTransformFloat{ Math::AffineF3::Identity() };
		Math::QuatD m_Rotation{ Math::QuatD::Identity() };
		Math::VecD3 m_Scaling{ 1.0 };
		Math::VecD3 m_Translation{ 0.0 };
		Math::BoxF3 m_GlobalBoundingBox{ Math::BoxF3::Empty() };

		bool m_HasLocalTransform{ false };
	};

	template<RHI::APITagConcept APITag>
	struct SceneImportResult final {
		SharedPtr<SceneGraphNode<APITag>> RootNode;
	};

	// A container that tracks unique resources of the same type used by some entity, for example unique meshes used in a scene graph.
   // It works by putting the resource shared pointers into a map and associating a reference count with each resource.
   // When the resource is added and released an equal number of times, its refrence count reaches zero, and it's removed from the container.
	template<typename Type>
	class ResourceTracker final {
		using UnderlyingConstIterator = typename UnorderedMap<SharedPtr<Type>, Uint32>::const_iterator;
	public:
		class ConstIterator final {
		public:
			ConstIterator(UnderlyingConstIterator iter) : m_Iter{ MoveTemp(iter) } {}
			ConstIterator& operator++(void) { ++this->m_Iter; return *this; }
			ConstIterator operator++(int) { ConstIterator res{ *this }; ++this->m_Iter; return res; }
			bool operator==(const ConstIterator&) const  noexcept = default;
			bool operator!=(const ConstIterator&) const noexcept = default;
			const SharedPtr<Type>& operator*(void) { return this->m_Iter->first; }
		private:
			UnderlyingConstIterator m_Iter;
		};

		// Adds a reference to the specified resource.
		// Returns true if this is the first reference, i.e. if the resource has just been added to the tracker.
		bool AddRef(const SharedPtr<Type>& resource) {
			if (nullptr == resource)
				return false;

			Uint32 refCount{ ++this->m_Map[resource] };
			return (refCount == 1);
		}

		// Removes a reference from the specified resource.
		// Returns true if this was the last reference, i.e. if the resource has just been removed from the tracker.
		bool Release(const SharedPtr<Type>& resource) {
			if (nullptr == resource)
				return false;

			auto it = this->m_Map.find(resource);
			if (it == this->m_Map.end())
				ASSERT(false); // trying to release an object not owned by this tracker
			if (it->second == 0)
				ASSERT(false); // zero-reference entries should not be possible; might indicate concurrency issues


			--it->second;

			if (it->second == 0) {
				this->m_Map.erase(it);
				return true;
			}
			return false;
		}

		//NOTE :std name
		STDNODISCARD ConstIterator begin() const { return ConstIterator{ this->m_Map.cbegin() }; }
		STDNODISCARD ConstIterator end() const { return ConstIterator{ this->m_Map.cend() }; }
		STDNODISCARD bool empty() const { return this->m_Map.empty(); }
		STDNODISCARD Uint64 size() const { return this->m_Map.size(); }

	private:
		UnorderedMap<SharedPtr<Type>, Uint32> m_Map;


	};

	template<typename Type>
	using SceneResourceCallback = Function<void(const SharedPtr<Type>&)>;



	template<RHI::APITagConcept APITag>
	class SceneGraph final : public EnableSharedFromThis<SceneGraph<APITag>> {
		friend class SceneGraphNode<APITag>;
	public:
		SceneGraph(void) = default;
		~SceneGraph(void) = default;

	public:

		STDNODISCARD const SharedPtr<SceneGraphNode<APITag>>& Get_RootNode(void) const { return this->m_Root; }
		STDNODISCARD const ResourceTracker<Material<APITag>>& Get_Materials(void) const { return this->m_Materials; }
		STDNODISCARD const ResourceTracker<MeshInfo<APITag>>& Get_Meshes(void) const { return this->m_Meshes; }

		STDNODISCARD const Uint64 Get_GeometryCount(void) const { return this->m_GeometryCount; }
		STDNODISCARD const Uint64 Get_MaxGeometryCountPerMesh(void) const { return this->m_MaxGeometryCountPerMesh; }
		STDNODISCARD const Uint64 Get_GeometryInstancesCount(void) const { return this->m_GeometryInstancesCount; }

		STDNODISCARD const Vector<SharedPtr<MeshInstance<APITag>>>& Get_MeshInstances(void) const { return this->m_MeshInstances; }
		STDNODISCARD const Vector<SharedPtr<SkinnedMeshInstance<APITag>>>& Get_SkinnedMeshInstances(void) const { return this->m_SkinnedMeshInstances; }
		STDNODISCARD const Vector<SharedPtr<SceneGraphAnimation<APITag>>>& Get_Animations(void) const { return this->m_Animations; }
		STDNODISCARD const Vector<SharedPtr<SceneCamera<APITag>>>& Get_Cameras(void) const { return this->m_Cameras; }
		STDNODISCARD const Vector<SharedPtr<Light<APITag>>>& Get_Lights(void) const { return this->m_Lights; }

		STDNODISCARD bool HasPendingStructureChanges(void) const {
			using enum SceneGraphNode<APITag>::DirtyFlags;//here has a bug...//TODO
			return
				nullptr != this->m_Root &&
				(this->m_Root->m_Dirty & SceneGraphNode<APITag>::DirtyFlags::SubgraphStructure) != SceneGraphNode<APITag>::DirtyFlags::None;
		}
		STDNODISCARD bool HasPendingTransformChanges(void) const {
			using enum SceneGraphNode<APITag>::DirtyFlags;
			return
				nullptr != this->m_Root &&
				(this->m_Root->m_Dirty & (SceneGraphNode<APITag>::DirtyFlags::SubgraphTransforms | SceneGraphNode<APITag>::DirtyFlags::SubgraphPrevTransforms)) != SceneGraphNode<APITag>::DirtyFlags::None;
		}


	public:



		SharedPtr<SceneGraphNode<APITag>> Set_RootNode(const SharedPtr<SceneGraphNode<APITag>>& root);

		// Attaches a node and its subgraph to the parent.
	   // If the node is already attached to this or other graph, a deep copy of the subgraph is made first.
		SharedPtr<SceneGraphNode<APITag>> Attach(const SharedPtr<SceneGraphNode<APITag>>& parent, const SharedPtr<SceneGraphNode<APITag>>& child);

		// Creates a node holding the provided leaf and attaches it to the parent.
		SharedPtr<SceneGraphNode<APITag>> AttachLeafNode(const SharedPtr<SceneGraphNode<APITag>>& parent, const SharedPtr<SceneGraphLeaf<APITag>>& leaf);

		// Removes the node and its subgraph from the graph.
		// When preserveOrder is 'false', the order of node's siblings may be changed during this operation to improve performance.
		SharedPtr<SceneGraphNode<APITag>> Detach(const SharedPtr<SceneGraphNode<APITag>>& node, bool preserveOrder = false);


		void RegisterLeaf(const SharedPtr<SceneGraphLeaf<APITag>>& leaf);

		void UnregisterLeaf(const SharedPtr<SceneGraphLeaf<APITag>>& leaf);

		void Refresh(Uint32 frameIndex);

	private:


	private:
		SharedPtr<SceneGraphNode<APITag>> m_Root;

		ResourceTracker<Material<APITag>> m_Materials;
		ResourceTracker<MeshInfo<APITag>> m_Meshes;

		Uint64 m_GeometryCount{ 0 };
		Uint64 m_MaxGeometryCountPerMesh{ 0 };
		Uint64 m_GeometryInstancesCount{ 0 };

		Vector<SharedPtr<MeshInstance<APITag>>> m_MeshInstances;
		Vector<SharedPtr<SkinnedMeshInstance<APITag>>> m_SkinnedMeshInstances;
		Vector<SharedPtr<SceneGraphAnimation<APITag>>> m_Animations;
		Vector<SharedPtr<SceneCamera<APITag>>> m_Cameras;
		Vector<SharedPtr<Light<APITag>>> m_Lights;

		SceneResourceCallback<MeshInfo<APITag>> OnMeshAdded;
		SceneResourceCallback<MeshInfo<APITag>> OnMeshRemoved;
		SceneResourceCallback<Material<APITag>> OnMaterialAdded;
		SceneResourceCallback<Material<APITag>> OnMaterialRemoved;

	};

	template<RHI::APITagConcept APITag>
	class SceneGraphIterator final {
	public:
		SceneGraphIterator(void) = default;
		explicit SceneGraphIterator(SceneGraphNode<APITag>* scope) :
			m_Current{ scope },
			m_Scope{ scope } {
		}
		SceneGraphIterator(SceneGraphNode<APITag>* current, SceneGraphNode<APITag>* scope) :
			m_Current{ current },
			m_Scope{ scope } {
		}

		~SceneGraphIterator(void) = default;

	public:
		operator bool(void) { return this->m_Current != nullptr; }
		SceneGraphNode<APITag>* operator->(void) { return this->m_Current; }

	public:
		SceneGraphNode<APITag>* Get(void) const { return this->m_Current; }

		// Moves the pointer to the first child of the current node, if it exists, and if allowChildren = true.
		// Otherwise, moves the pointer to the next sibling of the current node, if it exists.
		// Otherwise, goes up and tries to find the next sibiling up the hierarchy.
		// Returns the depth of the new node relative to the current node.
		Int32 Next(bool allowChildren);

		// Moves the pointer to the parent of the current node, up to the scope.
		// Note that using Up and Next together may result in an infinite loop.
		// Returns the depth of the new node relative to the current node.
		Int32 Up(void);


	private:
		SceneGraphNode<APITag>* m_Current{ nullptr };
		SceneGraphNode<APITag>* m_Scope{ nullptr };
		Vector<Uint64> m_ChildIndices;

	};




	template<RHI::APITagConcept APITag>
	inline Int32 SceneGraphIterator<APITag>::Next(bool allowChildren) {
		ASSERT(nullptr != this->m_Current);//NOTE : assertion ,user to makeuse of this function

		// Try to move down to the children of the current node
		if (allowChildren && this->m_Current->Get_NumChildren() > 0) {
			this->m_ChildIndices.push_back(0);
			this->m_Current = this->m_Current->Get_Child(0);
			return 1;
		}

		Int32 depth{ 0 };

		// No chlidren or not allowed to use them - try the next sibling or go up and try parent's sibling, etc.
		while (nullptr != this->m_Current) {
			if (this->m_Current == this->m_Scope) {
				this->m_Current = nullptr;
				return depth;
			}

			if (!this->m_ChildIndices.empty()) {
				Uint64& siblingIndex{ this->m_ChildIndices.back() };
				++siblingIndex;

				SceneGraphNode<APITag>* parent{ this->m_Current->Get_Parent() };
				if (siblingIndex < parent->Get_NumChildren()) {
					this->m_Current = parent->Get_Child(siblingIndex);
					return depth;
				}

				this->m_ChildIndices.pop_back();
			}

			this->m_Current = this->m_Current->Get_Parent();
			--depth;
		}

		return depth;
	}

	template<RHI::APITagConcept APITag>
	inline Int32 SceneGraphIterator<APITag>::Up(void) {
		ASSERT(nullptr != this->m_Current);

		if (this->m_Current == this->m_Scope) {
			this->m_Current = nullptr;
			return 0;
		}

		if (!this->m_ChildIndices.empty())
			this->m_ChildIndices.pop_back();

		this->m_Current = this->m_Current->Get_Parent();

		return -1;
	}












	template<RHI::APITagConcept APITag>
	inline SharedPtr<SceneGraphNode<APITag>> SceneGraph<APITag>::Set_RootNode(const SharedPtr<SceneGraphNode<APITag>>& root) {
		SharedPtr<SceneGraphNode<APITag>> OldRoot{ this->m_Root };

		if (nullptr != this->m_Root)
			this->Detach(this->m_Root);

		this->Attach(nullptr, root);

		return OldRoot;
	}

	template<RHI::APITagConcept APITag>
	inline SharedPtr<SceneGraphNode<APITag>> SceneGraph<APITag>::Attach(const SharedPtr<SceneGraphNode<APITag>>& parent, const SharedPtr<SceneGraphNode<APITag>>& child) {
		auto parentGraph{ nullptr != parent ? parent->m_Graph.lock() : EnableSharedFromThis<SceneGraph<APITag>>::shared_from_this() };
		auto childGraph{ child->m_Graph.lock() };

		if (nullptr == parentGraph && nullptr == childGraph) {
			// operating on an orphaned subgraph - do not copy or register anything
			ASSERT(nullptr != parent);

			parent->m_Children.push_back(child);
			child->m_Parent = parent.get();
			return child;
		}

		ASSERT(parentGraph.get() == this);
		SharedPtr<SceneGraphNode<APITag>> attachedChild;

		if (nullptr != childGraph) {
			// attaching a subgraph that already belongs to a graph - this one or another
			// copy the subgraph first

			// keep a mapping of old nodes to new nodes to patch the copied animations
			UnorderedMap<SceneGraphNode<APITag>*, SharedPtr<SceneGraphNode<APITag>>> nodeMap;

			SceneGraphNode<APITag>* currentParent{ parent.get() };
			SceneGraphIterator<APITag> walker{ child.get() };
			while (walker) {
				// make a copy of the current node
				SharedPtr<SceneGraphNode<APITag>>copy{ MakeShared<SceneGraphNode<APITag>>() };
				nodeMap[walker.Get()] = copy;

				copy->m_Name = walker->m_Name;
				copy->m_Parent = currentParent;
				copy->m_Graph = EnableSharedFromThis<SceneGraph<APITag>>::weak_from_this();
				copy->m_Dirty = walker->m_Dirty;

				if (walker->m_HasLocalTransform)
					copy->Set_Transform(&walker->m_Translation, &walker->m_Rotation, &walker->m_Scaling);

				if (nullptr != walker->m_Leaf) {
					auto leafCopy{ walker->m_Leaf->Clone() };
					copy->Set_Leaf(leafCopy);
				}

				// attach the copy to the new parent
				if (nullptr != currentParent)
					currentParent->m_Children.push_back(copy);
				else
					this->m_Root = copy;

				// if it's the first node we copied, store it as the new root
				if (nullptr == attachedChild)
					attachedChild = copy;

				// go to the next node
				auto deltaDepth{ walker.Next(true) };

				if (deltaDepth > 0)
					currentParent = copy.get();
				else {
					while (deltaDepth++ < 0)						// go up the new tree
						currentParent = currentParent->m_Parent;
				}
			}

			// go over the new nodes and patch the cloned animations and skinned groups to use the *new* nodes
			/*walker = SceneGraphIterator<APITag>{ attachedChild.get() };
			while (walker) {
				if (auto animation = dynamic_cast<SceneGraphAnimation*>(walker->m_Leaf.get()); nullptr != animation)
					for (const auto& channel : animation->Get_Channels()) {
						if (auto newNode = nodeMap[channel->Get_TargetNode().get()]; nullptr != newNode)
							channel->Set_TargetNode(newNode);
					}
				else if (auto skinnedInstance = dynamic_cast<SkinnedMeshInstance*>(walker->m_Leaf.get()); nullptr != skinnedInstance)
					for (auto& joint : skinnedInstance->joints) {
						auto jointNode{ joint.node.lock() };
						auto newNode{ nodeMap[jointNode.get()] };
						if (nullptr != newNode)
							joint.node = newNode;
					}
				else if (auto meshReference = dynamic_cast<SkinnedMeshReference*>(walker->m_Leaf.get()); nullptr != meshReference)
					if (auto instance = meshReference->m_Instance.lock(); nullptr != instance) {
						auto oldNode{ instance->Get_Node() };
						auto newNode{ nodeMap[oldNode] };

						if (nullptr != newNode)
							meshReference->m_Instance = std::dynamic_pointer_cast<SkinnedMeshInstance>(newNode->m_Leaf);
						else
							meshReference->m_Instance.reset();
					}

				walker.Next(true);
			}*/
		}
		else {// attaching a subgraph that has been detached from another graph (or never attached)
			SceneGraphIterator<APITag> walker{ child.get() };
			while (walker) {
				walker->m_Graph = EnableSharedFromThis<SceneGraph<APITag>>::weak_from_this();
				if (auto leaf{ walker->Get_Leaf() }; nullptr != leaf)
					this->RegisterLeaf(leaf);
				walker.Next(true);
			}

			child->m_Parent = parent.get();

			if (nullptr != parent)
				parent->m_Children.push_back(child);
			else
				this->m_Root = child;

			attachedChild = child;
		}

		attachedChild->PropagateDirtyFlags(SceneGraphNode<APITag>::DirtyFlags::SubgraphStructure | (child->m_Dirty & SceneGraphNode<APITag>::DirtyFlags::SubgraphMask));

		return attachedChild;
	}

	template<RHI::APITagConcept APITag>
	inline SharedPtr<SceneGraphNode<APITag>> SceneGraph<APITag>::AttachLeafNode(const SharedPtr<SceneGraphNode<APITag>>& parent, const SharedPtr<SceneGraphLeaf<APITag>>& leaf) {
		auto node{ MakeShared<SceneGraphNode<APITag>>() };
		if (nullptr != leaf->Get_Node())
			node->Set_Leaf(leaf->Clone());
		else
			node->Set_Leaf(leaf);
		return this->Attach(parent, node);
	}

	template<RHI::APITagConcept APITag>
	inline SharedPtr<SceneGraphNode<APITag>> SceneGraph<APITag>::Detach(const SharedPtr<SceneGraphNode<APITag>>& node, bool preserveOrder) {
		SharedPtr<SceneGraph<APITag>> NodeGraph{ node->m_Graph.lock() };

		if (nullptr != NodeGraph) {
			ASSERT(NodeGraph.get() != this);

			SceneGraphIterator<APITag> It{ node.get() };
			while (It) {
				It->m_Graph.reset();
				if (const auto& Leaf{ It->Get_Leaf() }; nullptr != Leaf)
					It.Next(true);
			}
		}

		//TODO

		return node;
	}

	template<RHI::APITagConcept APITag>
	inline void SceneGraph<APITag>::RegisterLeaf(const SharedPtr<SceneGraphLeaf<APITag>>& leaf) {
		if (nullptr == leaf)
			return;

		if (auto meshInstance{ DynamicPointerCast<MeshInstance<APITag>>(leaf) }; nullptr != meshInstance) {
			if (const auto& mesh{ meshInstance->Get_Mesh() }; nullptr != mesh) {
				Uint64 geometryCount{ 0 };
				if (this->m_Meshes.AddRef(mesh)) {
					geometryCount += mesh->Geometries.size();
					this->m_GeometryCount += mesh->Geometries.size();
					if (nullptr != this->OnMeshAdded)
						this->OnMeshAdded(mesh);
				}

				for (const auto& geometry : mesh->Geometries)
					if (this->m_Materials.AddRef(geometry->Material) && nullptr != this->OnMaterialAdded)
						this->OnMaterialAdded(geometry->Material);

				if (nullptr != mesh->SkinPrototype) {
					if (this->m_Meshes.AddRef(mesh->SkinPrototype)) {
						geometryCount += mesh->SkinPrototype->Geometries.size();
						this->m_GeometryCount += mesh->SkinPrototype->Geometries.size();
						if (nullptr != this->OnMeshAdded)
							this->OnMeshAdded(mesh->SkinPrototype);
					}
				}

				this->m_MaxGeometryCountPerMesh = Math::Max(this->m_MaxGeometryCountPerMesh, geometryCount);
			}
			this->m_MeshInstances.push_back(meshInstance);

			if (auto skinnedInstance{ DynamicPointerCast<SkinnedMeshInstance<APITag>>(leaf) }; nullptr != skinnedInstance)
				this->m_SkinnedMeshInstances.push_back(skinnedInstance);

			return;
		}

		if (auto animation{ DynamicPointerCast<SceneGraphAnimation<APITag>>(leaf) }; nullptr != animation) {
			this->m_Animations.push_back(animation);
			return;
		}

		if (auto camera{ DynamicPointerCast<SceneCamera<APITag>>(leaf) }; nullptr != camera) {
			this->m_Cameras.push_back(camera);
			return;
		}

		if (auto light{ DynamicPointerCast<Light<APITag>>(leaf) }; nullptr != light) {
			this->m_Lights.push_back(light);
			return;
		}
	}

	template<RHI::APITagConcept APITag>
	inline void SceneGraph<APITag>::UnregisterLeaf(const SharedPtr<SceneGraphLeaf<APITag>>& leaf) {
		if (nullptr == leaf)
			return;

		if (auto meshInstance{ DynamicPointerCast<MeshInstance<APITag>>(leaf) }; nullptr != meshInstance) {
			if (const auto& mesh{ meshInstance->Get_Mesh() }; nullptr != mesh) {
				if (this->m_Meshes.Release(mesh)) {
					this->m_GeometryCount -= mesh->Geometries.size();
					if (nullptr != this->OnMeshRemoved)
						OnMeshRemoved(mesh);
				}

				for (const auto& geometry : mesh->Geometries)
					if (this->m_Materials.Release(geometry->Material) && nullptr != this->OnMaterialRemoved)
						this->OnMaterialRemoved(geometry->Material);

				if (nullptr != mesh->SkinPrototype)
					if (this->m_Meshes.Release(mesh->SkinPrototype)) {
						this->m_GeometryCount -= mesh->SkinPrototype->Geometries.size();
						if (nullptr != OnMeshRemoved)
							this->OnMeshRemoved(mesh->SkinPrototype);
					}
			}

			if (auto it = STDFind(this->m_MeshInstances.begin(), this->m_MeshInstances.end(), meshInstance); it != this->m_MeshInstances.end())
				this->m_MeshInstances.erase(it);//std::find_if ... not wapper...
			return;
		}

		if (auto skinnedInstance{ DynamicPointerCast<SkinnedMeshInstance<APITag>>(leaf) }; nullptr != skinnedInstance) {

			if (auto it{ STDFind(this->m_SkinnedMeshInstances.begin(), this->m_SkinnedMeshInstances.end(), skinnedInstance) }; it != this->m_SkinnedMeshInstances.end())
				this->m_SkinnedMeshInstances.erase(it);
			return;
		}

		if (auto animation{ DynamicPointerCast<SceneGraphAnimation<APITag>>(leaf) }; nullptr != animation) {

			if (auto it{ STDFind(this->m_Animations.begin(), this->m_Animations.end(), animation) }; it != this->m_Animations.end())
				this->m_Animations.erase(it);
			return;
		}

		if (auto camera{ DynamicPointerCast<SceneCamera<APITag>>(leaf) }; nullptr != camera) {

			if (auto it{ STDFind(this->m_Cameras.begin(), this->m_Cameras.end(), camera) }; it != this->m_Cameras.end())
				this->m_Cameras.erase(it);
			return;
		}

		if (auto light{ DynamicPointerCast<Light<APITag>>(leaf) }; nullptr != light) {

			if (auto it{ STDFind(this->m_Lights.begin(), this->m_Lights.end(), light) }; it != this->m_Lights.end())
				this->m_Lights.erase(it);
			return;
		}
	}

	template<RHI::APITagConcept APITag>
	inline void SceneGraph<APITag>::Refresh(Uint32 frameIndex) {
		struct StackItem final {
			bool SupergraphTransformUpdated{ false };
			bool SupergraphContentUpdate{ false };
		};

		bool structureDirty{ this->HasPendingStructureChanges() };

		StackItem context;
		Vector<StackItem> stack;

		SceneGraphIterator<APITag> walker{ this->m_Root.get() };
		while (walker) {
			auto current{ walker.Get() };
			auto parent{ current->Get_Parent() };

			// save the current local/global transforms as previous
			current->m_PrevLocalTransform = current->m_LocalTransform;
			current->m_PrevGlobalTransform = current->m_GlobalTransform;
			current->m_PrevGlobalTransformFloat = current->m_GlobalTransformFloat;

			bool currentTransformUpdated = (current->m_Dirty & SceneGraphNode<APITag>::DirtyFlags::LocalTransform) != SceneGraphNode<APITag>::DirtyFlags::None;
			bool currentContentUpdated = (current->m_Dirty & SceneGraphNode<APITag>::DirtyFlags::SubgraphContentUpdate) != SceneGraphNode<APITag>::DirtyFlags::None;

			if (currentTransformUpdated)
				current->UpdateLocalTransform();

			// update the global transform of the current node
			if (nullptr != parent) {
				current->m_GlobalTransform =
					current->m_HasLocalTransform
					? current->m_LocalTransform * parent->m_GlobalTransform
					: parent->m_GlobalTransform;
			}
			else
				current->m_GlobalTransform = current->m_LocalTransform;
			current->m_GlobalTransformFloat = Math::AffineF3{ current->m_GlobalTransform };

			// initialize the global bbox of the current node, start with the leaf (or an empty box if there is no leaf)
			if ((current->m_Dirty & (SceneGraphNode<APITag>::DirtyFlags::SubgraphStructure | SceneGraphNode<APITag>::DirtyFlags::SubgraphTransforms)) != SceneGraphNode<APITag>::DirtyFlags::None || context.SupergraphTransformUpdated) {
				current->m_GlobalBoundingBox = Math::BoxF3::Empty();
				if (nullptr != current->m_Leaf) {
					Math::BoxF3 localBoundingBox{ current->m_Leaf->Get_LocalBoundingBox() };
					if (!localBoundingBox.Is_Empty())
						current->m_GlobalBoundingBox = localBoundingBox * current->m_GlobalTransformFloat;
				}
			}

			// initialize the content flags of the current node
			if (context.SupergraphContentUpdate || (current->m_Dirty & (SceneGraphNode<APITag>::DirtyFlags::SubgraphStructure | SceneGraphNode<APITag>::DirtyFlags::SubgraphContentUpdate)) != SceneGraphNode<APITag>::DirtyFlags::None) {
				if (nullptr != current->m_Leaf)
					current->m_LeafContent = current->m_Leaf->Get_ContentFlags();
				else
					current->m_LeafContent = SceneContentFlags::None;

				current->m_SubgraphContent = current->m_LeafContent;
			}

			// store the update frame number for skinned groups
			if (auto meshReference = dynamic_cast<SkinnedMeshReference<APITag>*>(current->m_Leaf.get()); nullptr != meshReference) {
				if ((current->m_Dirty & SceneGraphNode<APITag>::DirtyFlags::LocalTransform) != SceneGraphNode<APITag>::DirtyFlags::None)
					if (auto instance{ meshReference->m_Instance.lock() }; nullptr != instance)
						instance->m_LastUpdateFrameIndex = frameIndex;
			}

			// advance to the next node
			bool subgraphNeedsRefresh{ (current->m_Dirty & SceneGraphNode<APITag>::DirtyFlags::SubgraphMask) != SceneGraphNode<APITag>::DirtyFlags::None };
			Int32 deltaDepth{ walker.Next(subgraphNeedsRefresh || context.SupergraphTransformUpdated || context.SupergraphContentUpdate) };

			// save the dirty flag to update the same nodes' previous transforms on the next frame
			current->m_Dirty =
				(currentTransformUpdated || context.SupergraphTransformUpdated)
				? SceneGraphNode<APITag>::DirtyFlags::PrevTransform
				: SceneGraphNode<APITag>::DirtyFlags::None;

			if (deltaDepth > 0) {
				// going down the tree
				stack.push_back(context);
				context.SupergraphTransformUpdated |= currentTransformUpdated;
				context.SupergraphContentUpdate |= currentContentUpdated;
			}
			else {
				// sibling or going up. done with our bbox, update the parent.
				if (nullptr != parent) {
					parent->m_GlobalBoundingBox |= current->m_GlobalBoundingBox;
					if ((current->m_Dirty & SceneGraphNode<APITag>::DirtyFlags::PrevTransform) != SceneGraphNode<APITag>::DirtyFlags::None)
						parent->m_Dirty |= SceneGraphNode<APITag>::DirtyFlags::SubgraphPrevTransforms;
					parent->m_Dirty |= current->m_Dirty & SceneGraphNode<APITag>::DirtyFlags::SubgraphMask;
					parent->m_SubgraphContent |= current->m_SubgraphContent;
				}

				// going up the tree, potentially multiple levels
				while (deltaDepth++ < 0) {
					if (stack.empty())
						context = StackItem{}; // should only happen once when we reach the top
					else {
						// we're moving up to node 'parent' whose parent is 'newParent'
						// update 'newParent's bbox with the finished bbox of 'parent'
						ASSERT(nullptr != parent);
						current = parent;
						parent = current->m_Parent;

						if (nullptr != parent) {
							parent->m_GlobalBoundingBox |= current->m_GlobalBoundingBox;
							parent->m_Dirty |= current->m_Dirty & SceneGraphNode<APITag>::DirtyFlags::SubgraphMask;
							parent->m_SubgraphContent |= current->m_SubgraphContent;
						}

						context = stack.back();
						stack.pop_back();
					}
				}
			}
		}

		if (structureDirty) {
			Int32 instanceIndex{ 0 };
			Int32 geometryInstanceIndex{ 0 };
			for (const auto& instance : this->m_MeshInstances) {
				instance->m_InstanceIndex = instanceIndex;
				++instanceIndex;

				const auto& mesh{ instance->Get_Mesh() };
				instance->m_GeometryInstanceIndex = geometryInstanceIndex;
				geometryInstanceIndex += static_cast<Int32>(mesh->Geometries.size());
			}
			this->m_GeometryInstancesCount = geometryInstanceIndex;

			Int32 meshIndex{ 0 };
			Int32 geometryIndex{ 0 };
			for (const auto& mesh : this->m_Meshes) {
				for (const auto& geometry : mesh->Geometries) {
					geometry->GlobalGeometryIndex = geometryIndex;
					++geometryIndex;
				}

				mesh->GlobalMeshIndex = meshIndex;
				++meshIndex;
			}

			ASSERT(this->m_GeometryCount == geometryIndex);

			Int32 materialIndex{ 0 };
			for (const auto& material : this->m_Materials) {
				material->MaterialID = materialIndex;
				++materialIndex;
			}
		}
	}


	template<RHI::APITagConcept APITag>
	inline void SceneGraphNode<APITag>::Set_Leaf(const SharedPtr<SceneGraphLeaf<APITag>>& leaf) {
		auto graph{ this->m_Graph.lock() };

		if (nullptr != this->m_Leaf) {
			this->m_Leaf->m_Node.reset();
			if (nullptr != graph)
				graph->UnregisterLeaf(this->m_Leaf);
		}

		this->m_Leaf = leaf;
		leaf->m_Node = SceneGraphNode<APITag>::weak_from_this();
		if (nullptr != graph)
			graph->RegisterLeaf(leaf);

		this->m_Dirty |= DirtyFlags::Leaf;
		this->PropagateDirtyFlags(DirtyFlags::SubgraphStructure);
	}

	template<RHI::APITagConcept APITag>
	inline void SceneGraphNode<APITag>::PropagateDirtyFlags(SceneGraphNode::DirtyFlags flags) {
		SceneGraphIterator<APITag>walker{ this, nullptr };
		while (walker) {
			walker->m_Dirty |= flags;
			walker.Up();
		}
	}

	template<RHI::APITagConcept APITag>
	inline Math::VecD3 Light<APITag>::Get_Position(void) const {
		if (auto node{ this->Get_Node() }; nullptr == node)
			return node->Get_LocalToWorldTransform().m_translation;
		else
			return Math::VecD3::Zero();

	}

	template<RHI::APITagConcept APITag>
	inline Math::VecD3 Light<APITag>::Get_Direction(void) const {
		if (auto node{ this->Get_Node() }; nullptr != node)
			return -Math::Normalize(Math::VecD3{ node->Get_LocalToWorldTransform().m_Linear.Row2 });
		else
			return Math::VecD3::Zero();
	}

	template<RHI::APITagConcept APITag>
	inline void Light<APITag>::Set_Position(const Math::VecD3& position) const {
		auto node{ this->Get_Node() };

		if (nullptr == node) {
			LOG_ERROR("Light::Set_Position: node is null");

			return;
		}

		SceneGraphNode<APITag>* parent{ node->Get_Parent() };
		Math::VecD3 parentToWorld{ Math::VecD3::Identity() };
		if (nullptr != parent)
			parentToWorld = parent->Get_LocalToWorldTransform();

		Math::VecD3 translation{ Math::Inverse(parentToWorld).TransformPoint(position) };
		node->Set_Translation(translation);
	}

	template<RHI::APITagConcept APITag>
	inline void Light<APITag>::Set_Direction(const Math::VecD3& direction) const {
		auto node{ this->Get_Node() };

		if (nullptr == node) {
			LOG_ERROR("Light::Set_Position: node is null");

			return;
		}

		SceneGraphNode<APITag>* parent{ node->Get_Parent() };
		Math::AffineD3 parentToWorld{ Math::AffineD3::Identity() };
		if (nullptr != parent)
			parentToWorld = Math::AffineD3{ parent->Get_LocalToWorldTransform() };

		Math::AffineD3 worldToLocal{ Math::LookatZ(direction) };
		Math::AffineD3 localToParent{ Math::Inverse(worldToLocal * parentToWorld) };

		Math::QuatD rotation;
		Math::VecD3 scaling;
		Math::DecomposeAffine<double>(localToParent, nullptr, &rotation, &scaling);

		node->Set_Transform(nullptr, &rotation, &scaling);
	}

	template<RHI::APITagConcept APITag>
	inline void DirectionalLight<APITag>::FillLightConstants(LightConstants& lightConstants) const {
		this->Light<APITag>::FillLightConstants(lightConstants);

		lightConstants.LightType = ::LightType_Directional;//TODO :
		lightConstants.Direction = Math::VecF3{ Math::Normalize(this->Get_Direction()) };
		float clampedAngularSize{ Math::Clamp(this->AngularSize, 0.f, 90.f) };
		lightConstants.AngularSizeOrInvRange = Math::Radians(clampedAngularSize);
		lightConstants.Intensity = this->Irradiance;
	}

}