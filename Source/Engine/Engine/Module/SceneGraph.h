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

#include "RHI/Module/RHI.h"
#include "D3D12RHI/Module/D3D12RHI.h"

#endif // PARTING_MODULE_BUILD#pragma once

//namespace Parting {
//
//	class SceneGraph;
//	class SceneGraphNode;
//	class SceneTypeFactory;
//
//	enum class SceneContentFlags : Uint32{
//		None = 0,
//		OpaqueMeshes = 0x01,
//		AlphaTestedMeshes = 0x02,
//		BlendedMeshes = 0x04,
//		Lights = 0x08,
//		Cameras = 0x10,
//		Animations = 0x20
//	};
//	EXPORT_ENUM_CLASS_OPERATORS(SceneContentFlags);
//
//	class SceneGraphNode final : public EnableSharedFromThis<SceneGraphNode> {
//	public:
//		SceneGraphNode(void) = default;
//		~SceneGraphNode(void) = default;
//
//	public:
//
//
//	private:
//
//	};
//}


namespace Parting {

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
	class SceneGraphLeaf final :public NonCopyAndMoveAble {
		friend class SceneGraph<APITag>;
	public:
		SceneGraphLeaf(void) = default;
		~SceneGraphLeaf(void) = default;

	public:
		STDNODISCARD SceneGraphNode<APITag>* Get_Node(void) const { return this->m_Node.lock().get(); }
		STDNODISCARD SharedPtr<SceneGraphNode<APITag>> Get_NodeSharedPtr(void) const { return this->m_Node.lock(); }
		/*STDNODISCARD dm::box3 GetLocalBoundingBox() { return dm::box3::empty(); }*/
		STDNODISCARD SharedPtr<SceneGraphLeaf<APITag>> Clone(void) = 0;
		/*STDNODISCARD SceneContentFlags GetContentFlags() const { return SceneContentFlags::None; }*/
		STDNODISCARD const String& Get_Name(void) const;
		void Set_Name(const String& name) const;
		/*	virtual void Load(const Json::Value& node) {}*/
		/*	virtual bool SetProperty(const std::string& name, const dm::float4& value) { return false; }*/




	private:
		WeakPtr<SceneGraphNode<APITag>> m_Node;
	};

	template<RHI::APITagConcept APITag>
	class SceneGraphNode final : public EnableSharedFromThis<SceneGraphNode<APITag>> {
		friend class SceneGraph<APITag>;
	public:
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

		STDNODISCARD SceneGraphNode<APITag>* Get_Parent(void) const { return this->m_Parent; }
		STDNODISCARD SceneGraphNode<APITag>* Get_Child(Uint64 index) const { return (index < this->m_Children.size()) ? this->m_Children[index].get() : nullptr; }
		STDNODISCARD Uint64 Get_NumChildren(void) const { return this->m_Children.size(); }
		STDNODISCARD const SharedPtr<SceneGraphLeaf<APITag>>& Get_Leaf(void) const { return this->m_Leaf; }


	private:


	private:
		WeakPtr<SceneGraph<APITag>> m_Graph;
		SceneGraphNode<APITag>* m_Parent{ nullptr };
		Vector<SharedPtr<SceneGraphNode<APITag>>> m_Children;
		SharedPtr<SceneGraphLeaf<APITag>> m_Leaf;
		DirtyFlags m_Dirty{ DirtyFlags::None };

		bool m_HasLocalTransform{ false };

	};

	template<RHI::APITagConcept APITag>
	struct SceneImportResult final {
		SharedPtr<SceneGraphNode<APITag>> RootNode;
	};



	template<RHI::APITagConcept APITag>
	class SceneGraph : public EnableSharedFromThis<SceneGraph<APITag>> {
	public:
		SceneGraph(void) = default;
		~SceneGraph(void) = default;

	public:
		SharedPtr<SceneGraphNode<APITag>> Set_RootNode(const SharedPtr<SceneGraphNode<APITag>>& root);

		// Attaches a node and its subgraph to the parent.
	   // If the node is already attached to this or other graph, a deep copy of the subgraph is made first.
		SharedPtr<SceneGraphNode<APITag>> Attach(const SharedPtr<SceneGraphNode<APITag>>& parent, const SharedPtr<SceneGraphNode<APITag>>& child);


		// Removes the node and its subgraph from the graph.
		// When preserveOrder is 'false', the order of node's siblings may be changed during this operation to improve performance.
		SharedPtr<SceneGraphNode<APITag>> Detach(const SharedPtr<SceneGraphNode<APITag>>& node, bool preserveOrder = false);

	private:


	private:
		SharedPtr<SceneGraphNode<APITag>> m_Root;

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
		SceneGraphNode<APITag>* m_Current;
		SceneGraphNode<APITag>* m_Scope;
		Vector<Uint64> m_ChildIndices;

	};




	template<RHI::APITagConcept APITag>
	inline Int32 SceneGraphIterator<APITag>::Next(bool allowChildren) {
		if (nullptr == m_Current)
			return 0;

		// Try to move down to the children of the current node
		if (allowChildren && m_Current->Get_NumChildren() > 0) {
			this->m_ChildIndices.push(0);
			m_Current = m_Current->GetChild(0);
			return 1;
		}

		Int32 depth{ 0 };

		// No chlidren or not allowed to use them - try the next sibling or go up and try parent's sibling, etc.
		while (this->m_Current) {
			if (this->m_Current == this->m_Scope) {
				this->m_Current = nullptr;
				return depth;
			}

			if (!this->m_ChildIndices.empty()) {
				Uint64& siblingIndex{ this->m_ChildIndices.back() };
				++siblingIndex;

				SceneGraphNode* parent{ this->m_Current->Get_Parent() };
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
		if (nullptr != this->m_Current)
			return 0;

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


	}

	template<RHI::APITagConcept APITag>
	inline SharedPtr<SceneGraphNode<APITag>> SceneGraph<APITag>::Attach(const SharedPtr<SceneGraphNode<APITag>>& parent, const SharedPtr<SceneGraphNode<APITag>>& child) {
		//auto parentGraph{ parent ? parent->m_Graph.lock() : EnableSharedFromThis<SceneGraph<APITag>>::shared_from_this() };
		//auto childGraph{ child->m_Graph.lock() };

		//if (nullptr != parentGraph && nullptr != childGraph) {
		//	// operating on an orphaned subgraph - do not copy or register anything
		//	ASSERT(nullptr != parent);

		//	parent->m_Children.push_back(child);
		//	child->m_Parent = parent.get();
		//	return child;
		//}

		//ASSERT(parentGraph.get() == this);
		//SharedPtr<SceneGraphNode<APITag>> attachedChild;

		//if (nullptr != childGraph) {
		//	// attaching a subgraph that already belongs to a graph - this one or another
		//	// copy the subgraph first

		//	// keep a mapping of old nodes to new nodes to patch the copied animations
		//	UnorderedMap<SceneGraphNode<APITag>*, SharedPtr<SceneGraphNode<APITag>>> nodeMap;

		//	SceneGraphNode<APITag>* currentParent{ parent.get() };
		//	SceneGraphIterator<APITag> walker(child.get());
		//	while (walker) {
		//		// make a copy of the current node
		//		SharedPtr<SceneGraphNode<APITag>>copy{ MakeShared<SceneGraphNode>() };
		//		nodeMap[walker.Get()] = copy;

		//		copy->m_Name = walker->m_Name;
		//		copy->m_Parent = currentParent;
		//		copy->m_Graph = EnableSharedFromThis<SceneGraph<APITag>>::weak_from_this();
		//		copy->m_Dirty = walker->m_Dirty;

		//		if (walker->m_HasLocalTransform)
		//			copy->SetTransform(&walker->m_Translation, &walker->m_Rotation, &walker->m_Scaling);

		//		if (walker->m_Leaf)
		//		{
		//			auto leafCopy = walker->m_Leaf->Clone();
		//			copy->SetLeaf(leafCopy);
		//		}

		//		// attach the copy to the new parent
		//		if (currentParent)
		//		{
		//			currentParent->m_Children.push_back(copy);
		//		}
		//		else
		//		{
		//			m_Root = copy;
		//		}

		//		// if it's the first node we copied, store it as the new root
		//		if (!attachedChild)
		//			attachedChild = copy;

		//		// go to the next node
		//		int deltaDepth = walker.Next(true);

		//		if (deltaDepth > 0)
		//		{
		//			currentParent = copy.get();
		//		}
		//		else
		//		{
		//			while (deltaDepth++ < 0)
		//			{
		//				// go up the new tree
		//				currentParent = currentParent->m_Parent;
		//			}
		//		}
		//	}

		//	// go over the new nodes and patch the cloned animations and skinned groups to use the *new* nodes
		//	walker = SceneGraphWalker(attachedChild.get());
		//	while (walker)
		//	{
		//		if (auto animation = dynamic_cast<SceneGraphAnimation*>(walker->m_Leaf.get()))
		//		{
		//			for (const auto& channel : animation->GetChannels())
		//			{
		//				auto newNode = nodeMap[channel->GetTargetNode().get()];
		//				if (newNode)
		//				{
		//					channel->SetTargetNode(newNode);
		//				}
		//			}
		//		}
		//		else if (auto skinnedInstance = dynamic_cast<SkinnedMeshInstance*>(walker->m_Leaf.get()))
		//		{
		//			for (auto& joint : skinnedInstance->joints)
		//			{
		//				auto jointNode = joint.node.lock();
		//				auto newNode = nodeMap[jointNode.get()];
		//				if (newNode)
		//				{
		//					joint.node = newNode;
		//				}
		//			}
		//		}
		//		else if (auto meshReference = dynamic_cast<SkinnedMeshReference*>(walker->m_Leaf.get()))
		//		{
		//			auto instance = meshReference->m_Instance.lock();
		//			if (instance)
		//			{
		//				auto oldNode = instance->GetNode();

		//				auto newNode = nodeMap[oldNode];
		//				if (newNode)
		//					meshReference->m_Instance = std::dynamic_pointer_cast<SkinnedMeshInstance>(newNode->m_Leaf);
		//				else
		//					meshReference->m_Instance.reset();
		//			}
		//		}

		//		walker.Next(true);
		//	}
		//}
		//else
		//{
		//	// attaching a subgraph that has been detached from another graph (or never attached)

		//	SceneGraphWalker walker(child.get());
		//	while (walker)
		//	{
		//		walker->m_Graph = weak_from_this();
		//		auto leaf = walker->GetLeaf();
		//		if (leaf)
		//			RegisterLeaf(leaf);
		//		walker.Next(true);
		//	}

		//	child->m_Parent = parent.get();

		//	if (parent)
		//	{
		//		parent->m_Children.push_back(child);
		//	}
		//	else
		//	{
		//		m_Root = child;
		//	}

		//	attachedChild = child;
		//}

		//attachedChild->PropagateDirtyFlags(SceneGraphNode::DirtyFlags::SubgraphStructure
		//	| (child->m_Dirty & SceneGraphNode::DirtyFlags::SubgraphMask));

		//return attachedChild;
		//return nullptr;	
		
		return parent;
	}

	template<RHI::APITagConcept APITag>
	inline SharedPtr<SceneGraphNode<APITag>> SceneGraph<APITag>::Detach(const SharedPtr<SceneGraphNode<APITag>>& node, bool preserveOrder) {
		SharedPtr<SceneGraph<APITag>> NodeGraph{ node->m_Graph.Lock() };

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



}