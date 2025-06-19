#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"

PARTING_SUBMODULE(Container, SparseBitset)

PARTING_IMPORT std;

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;
PARTING_IMPORT Concurrent;
PARTING_IMPORT Algorithm;

PARTING_SUBMODE_IMPORT(Optional)
PARTING_SUBMODE_IMPORT(Vector)

#else
#pragma once

#include "Core/ModuleBuild.h"

#include "Core/Utility/Include/UtilityMacros.h"
//Global
#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#include "Core/Concurrent/Module/Concurrent.h"
#include "Core/Algorithm/Module/Algorithm.h"

#include "Core/Container/Module/Container-Optional.h"
#include "Core/Container/Module/Container-Vector.h"


#endif // PARTING_MODULE_BUILD#pragma once

PARTING_EXPORT using BitVector = std::vector<bool>;

PARTING_EXPORT class BitSetAllocator final {
public:
	BitSetAllocator(Uint32 capacity, bool multithreaded) :
		m_Allocated(capacity),//NOTE : middle kuohao will use initlist
		m_MultiThreaded{ multithreaded } {
	}

	~BitSetAllocator(void) = default;

public:

	Uint32 Allocate(void) {
		auto Re{ Max_Uint32 };

		if (this->m_MultiThreaded)
			this->m_Mutex.lock();

		auto Capacity{ static_cast<Uint32>(this->m_Allocated.size()) };

		for (Uint32 Index = 0; Index < Capacity; ++Index) {
			auto Next{ (this->m_NextAvailable + Index) % Capacity };

			if (!this->m_Allocated[Next]) {
				this->m_Allocated[Next] = true;
				this->m_NextAvailable = (Next + 1) % Capacity;

				break;
			}
		}

		if (this->m_MultiThreaded)
			this->m_Mutex.unlock();

		return Re;
	}

	void Release(Uint32 Index) {
		if (this->m_MultiThreaded)
			this->m_Mutex.lock();

		this->m_Allocated[Index] = false;
		this->m_NextAvailable = Math::Min(this->m_NextAvailable, Index);

		if (this->m_MultiThreaded)
			this->m_Mutex.unlock();
	}

	Uint32 Get_Capacity(void) { return static_cast<Uint32>(this->m_Allocated.size()); }

private:
	Uint32 m_NextAvailable{ 0 };
	BitVector m_Allocated;
	bool m_MultiThreaded;
	Mutex m_Mutex;

};

//PARTING_EXPORT class SparseBitset final {
//private:
//	struct Element final {
//		Uint32 WordIndex;
//		Uint32 Bits;
//	};
//
//public:
//	struct const_iterator {
//		const SparseBitset* m_Bitset;
//		Uint32 m_ElementIndex;
//		Int32 m_Bit{};
//
//		const Uint32 operator*() const { return (this->m_Bitset->m_Storage[this->m_ElementIndex].WordIndex << 5) + this->m_Bit; }
//		const_iterator& operator++(void) {
//			while (this->m_ElementIndex < this->m_Bitset->m_Storage.size()) {
//				const auto& Element{ this->m_Bitset->m_Storage[this->m_ElementIndex] };
//
//				[[maybe_unused]] const Uint32 NextBits{ Element.Bits & ~((1 << (this->m_Bit + 1)) - 1) };
//
//				this->m_Bit = -1;
//				++this->m_ElementIndex;
//
//				for (Uint32 Index = this->m_Bit + 1; Index < 32; ++Index)
//					if (NextBits & (1 << Index)) {
//						this->m_Bit = Index;
//
//						return *this;
//					}
//
//
//			}
//
//			this->m_Bit = 0;
//
//			return *this;
//
//		}
//		bool operator!=(const const_iterator& other) const {
//			return
//				this->m_Bitset != other.m_Bitset ||
//				this->m_ElementIndex != other.m_ElementIndex ||
//				this->m_Bit != other.m_Bit;
//		}
//	};
//
//public:
//	SparseBitset operator&(const SparseBitset& other) const { return SparseBitset::Intersect(*this, other); }
//
//	SparseBitset operator-(const SparseBitset& other) const { return SparseBitset::Difference(*this, other); }
//
//	SparseBitset& operator|=(const SparseBitset& other) {
//		this->Include(other);
//
//		return *this;
//	}
//
//
//public:
//	Uint32& FindOrInsertWord(Uint32 wordIndex) {
//		auto It{ LowerBound(this->m_Storage.begin(), this->m_Storage.end(), wordIndex, [](const Element& element, Uint32 wordIndex) {return element.WordIndex < wordIndex; }) };
//
//		if (It != this->m_Storage.end() && It->WordIndex == wordIndex)
//			return It->Bits;
//
//		else
//			It = this->m_Storage.insert(It, Element{ .WordIndex{ wordIndex }, .Bits{ 0 } });
//
//		return It->Bits;
//	}
//
//	STDNODISCARD Uint32 GetWord(Uint32 WordIndex) const {
//		auto It{ LowerBound(this->m_Storage.begin(), this->m_Storage.end(), WordIndex, [](const Element& element, Uint32 wordIndex) {return element.WordIndex < wordIndex; }) };
//		if (It != this->m_Storage.end() && It->WordIndex == WordIndex)
//			return It->Bits;
//
//		return 0;//TODO maybe can use Optional
//	}
//
//	STDNODISCARD bool Is_Ordered(void) { return  Is_Sorted(this->m_Storage.begin(), this->m_Storage.end(), [](const Element& element, const Element& other) {return element.WordIndex <= other.WordIndex; }); }
//
//	void Set(Uint32 BitIndex, bool Value) {
//		//Uint32 wordIndex{ BitIndex / 32 };
//		auto WordIndex{ BitIndex >> 5 };
//		//Uint32 bitIndex{ BitIndex % 32 };
//		BitIndex = BitIndex & 0x1F;
//
//		Uint32& Bites{ this->FindOrInsertWord(WordIndex) };//TODO Use Word maybe name flaging
//		if (Value)
//			Bites |= (1 << BitIndex);
//		else
//			Bites &= ~(1 << BitIndex);
//	}
//
//	STDNODISCARD bool Get(Uint32 BitIndex) const {
//		auto WordIndex{ BitIndex >> 5 };
//		BitIndex = BitIndex & 0x1F;
//
//		auto Bites{ this->GetWord(WordIndex) };
//
//		return (Bites & BitIndex) != 0;
//	}
//
//	void Include(const SparseBitset& other) {
//		auto ItRe{ this->m_Storage.begin() };
//		auto ItOther{ other.m_Storage.cbegin() };
//
//		while (ItOther != other.m_Storage.cend()) {
//			if (ItRe == this->m_Storage.end() || ItRe->WordIndex > ItOther->WordIndex) {
//				ItRe = this->m_Storage.insert(ItRe, *ItOther);
//
//				++ItRe;
//			}
//			else if (ItRe->WordIndex < ItOther->WordIndex) {
//				++ItRe;
//			}
//			else {
//				ItRe->Bits |= ItOther->Bits;
//
//				++ItRe;
//				++ItOther;
//			}
//		}
//	}
//
//	STDNODISCARD bool Any(void)const {
//		for (const auto& element : this->m_Storage)
//			if (element.Bits != 0)
//				return true;
//
//		return false;
//	}
//
//	STDNODISCARD const_iterator begin(void) const {
//		return const_iterator{ .m_Bitset{ this }, .m_ElementIndex{ 0 }, .m_Bit{ -1 } };
//	}
//
//	STDNODISCARD const_iterator end(void) const {
//		return const_iterator{ .m_Bitset{ this }, .m_ElementIndex{ static_cast<Uint32>(this->m_Storage.size()) }, .m_Bit{ 0 } };
//	}
//
//public:
//	STDNODISCARD static SparseBitset Intersect(const SparseBitset& lhs, const SparseBitset& rhs) {
//		auto  ItLhs{ lhs.m_Storage.cbegin() };
//		auto  ItRhs{ rhs.m_Storage.cbegin() };
//
//		SparseBitset Re;
//		while (ItLhs != lhs.m_Storage.cend() && ItRhs != rhs.m_Storage.cend()) {
//			if (ItLhs->WordIndex < ItRhs->WordIndex)
//				++ItLhs;
//			else if (ItLhs->WordIndex > ItRhs->WordIndex)
//				++ItRhs;
//			else {
//				if (ItLhs->Bits & ItRhs->Bits)
//					Re.m_Storage.push_back(Element{ .WordIndex{ ItLhs->WordIndex }, .Bits{ ItLhs->Bits & ItRhs->Bits } });
//
//				++ItLhs;
//				++ItRhs;
//			}
//		}
//	}
//
//	STDNODISCARD static SparseBitset Difference(const SparseBitset& lhs, const SparseBitset& rhs) {
//		auto  ItLhs{ lhs.m_Storage.cbegin() };
//		auto  ItRhs{ rhs.m_Storage.cbegin() };
//
//		SparseBitset Re;
//		while (ItLhs != lhs.m_Storage.cend()) {
//			if (ItRhs == rhs.m_Storage.cend() || ItLhs->WordIndex < ItRhs->WordIndex) {
//				Re.m_Storage.push_back(*ItLhs);
//
//				++ItLhs;
//			}
//			else if (ItLhs->WordIndex > ItRhs->WordIndex)
//				++ItRhs;
//			else {
//				if (ItLhs->Bits & ~ItRhs->Bits)
//					Re.m_Storage.push_back(Element{ .WordIndex{ ItLhs->WordIndex }, .Bits{ ItLhs->Bits & ~ItRhs->Bits } });
//
//				++ItLhs;
//				++ItRhs;
//			}
//		}
//	}
//
//private:
//	Vector<Element> m_Storage;
//
//};