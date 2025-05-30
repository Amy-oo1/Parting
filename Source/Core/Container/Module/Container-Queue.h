#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"

PARTING_SUBMODULE(Container, Hash)

PARTING_IMPORT std;

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;
PARTING_IMPORT Concurrent;
PARTING_IMPORT Memory;

PARTING_SUBMODE_IMPORT(Optional)

#else
#pragma once

#include "Core/ModuleBuild.h"

#include "Core/Utility/Include/UtilityMacros.h"
//Global
#include<deque>
#include<queue>

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#include "Core/Concurrent/Module/Concurrent.h"
#include "Core/Memory/Module/Memory.h"

#include "Core/Container/Module/Container-Optional.h"

#endif // PARTING_MODULE_BUILD

PARTING_EXPORT template<typename Type>
class Queue_SPMC final :public MoveAbleOnly {
private:
	struct Node {
		explicit Node(Type&& data) :Data{ MoveTemp(data) }{}
		explicit Node(const Type& data) :Data{ data }{}

		Type Data;
		Atomic<Node*> Next{ nullptr };
	};

public:
	//NOTE : Alloctor has a cont use defualtresource
	//TODO : Uset Must inpout a ThreadSaft alloctor
	Queue_SPMC() :
		MoveAbleOnly{},
		m_PoolResource{ NewDeleteResource() },
		m_Alloctor{ &this->m_PoolResource },
		m_Head{ this->m_Alloctor.new_object<Node>(Type{}) },
		m_Tail{ this->m_Head.load(MemoryOrderRelaxd) }
	{
	}

	~Queue_SPMC(void) {
		auto Current{ this->m_Head.load(MemoryOrderRelaxd) };
		while (nullptr != Current) {
			auto Next{ Current->Next.load(MemoryOrderRelaxd) };

			this->m_Alloctor.delete_object(Current);
			Current = Next;
		}

	}

	void EnQueue(Type&& data) {
		auto NewNode{ this->m_Alloctor.new_object<Node>(MoveTemp(data)) };

		auto PervTail{ this->m_Tail.exchange(NewNode,MemoryOrderAcqRel) };
		PervTail->Next.store(NewNode, MemoryOrderRelease);
	}

	void EnQueue(const Type& data) {
		auto NewNode{ this->m_Alloctor.new_object<Node>(data) };

		auto PervTail{ this->m_Tail.exchange(NewNode, MemoryOrderAcqRel) };
		PervTail->Next.store(NewNode, MemoryOrderRelease);
	}

	STDNODISCARD Optional<Type> DeQueue(void) {
		Node* Current{ nullptr };
		Node* Next{ nullptr };

		for (;;) {
			Current = m_Head.load(MemoryOrderAcquire);
			Next = Current->Next.load(MemoryOrderAcquire);

			if (nullptr == Current)
				return NullOpt;

			if (this->m_Head.compare_exchange_weak(
				Current, Next,
				MemoryOrderAcqRel, MemoryOrderRelaxd))
				break;
		}

		auto Resource{ Next->Data };
		this->m_Alloctor.delete_object(Current);

		return Resource;
	}

	STDNODISCARD const Optional<Type> Peek(void) {
		auto Current{ m_Head.load(MemoryOrderRelaxd) };
		auto Next{ Current->Next.load(MemoryOrderRelaxd) };

		if (nullptr == Next)
			return NullOpt;

		return Next->Data;
	}

	STDNODISCARD bool Is_Empty(void) {
		return
			this->m_Head.load(MemoryOrderRelaxd) ==
			this->m_Tail.load(MemoryOrderRelaxd);
	}

private:
	SynchronizedPoolResource m_PoolResource{ nullptr };
	PolymorphicAllocator<Node> m_Alloctor;

	alignas(64) volatile Atomic<Node*> m_Head;
	alignas(64) Atomic<Node*> m_Tail;
};

PARTING_EXPORT template<typename _Ty>
using Queue = std::queue<_Ty, std::deque<_Ty>>;//Dont Fix

PARTING_EXPORT template<typename _Ty>
using Deque = std::deque<_Ty, std::allocator<_Ty>>;//Dont Fix