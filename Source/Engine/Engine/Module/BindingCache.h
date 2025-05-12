#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"

PARTING_MODULE(BindingCache)

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;


#else
#pragma once

#include "Core/ModuleBuild.h"


#include "Core/Utility/Include/UtilityMacros.h"
//Global

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#include "Core/Concurrent/Module/Concurrent.h"
#include "Core/Container/Module/Container.h"

#include "RHI/Module/RHI.h"
#include "D3D12RHI/Module/D3D12RHI.h"

#endif // PARTING_MODULE_BUILD

namespace Parting {


	template<RHI::APITagConcept APITag>
	struct HashBindingSetItem final {
		Uint64 operator()(const RHI::RHIBindingSetItem<APITag>& item)const noexcept {
			Uint64 hash{ 0 };
			hash = HashCombine(hash, HashVoidPtr{}(item.ResourcePtr));
			hash = HashCombine(hash, item.Slot);
			hash = HashCombine(hash, Tounderlying(item.Type));
			hash = HashCombine(hash, Tounderlying(item.Dimension));
			hash = HashCombine(hash, Tounderlying(item.Format));
			hash = HashCombine(hash, item.RawData[0]);
			hash = HashCombine(hash, item.RawData[1]);
			return hash;
		}
	};

	template<RHI::APITagConcept APITag>
	struct HashBindingSet final {
		Uint64 operator()(const RHI::RHIBindingSetDesc<APITag>& desc)const noexcept {
			Uint64 hash{ 0 };
			for (Uint32 Index = 0; Index < desc.BindingCount; ++Index)
				hash = HashCombine(hash, HashBindingSetItem<APITag>{}(desc.Bindings[Index]));
			return hash;
		}
	};




	template<RHI::APITagConcept APITag>
	class BindingCache final {
		using Imp_Device = typename RHI::RHITypeTraits<APITag>::Imp_Device;
		using Imp_BindingSet = typename RHI::RHITypeTraits<APITag>::Imp_BindingSet;
	public:
		BindingCache(Imp_Device* device)
			:m_Device{ device }
		{
		}

		RHI::RefCountPtr<Imp_BindingSet> GetCachedBindingSet(const RHI::RHIBindingSetDesc<APITag>& desc, Imp_BindingSet* layout) {
			Uint64 Hash = 0;
			HashCombine(Hash, HashBindingSet<APITag>{}(desc));
			HashCombine(Hash, HashVoidPtr{}(layout));

			this->m_Mutex.lock_shared();

			RHI::RefCountPtr<Imp_BindingSet> Re{ nullptr };
			if (auto It = this->m_BindingSets.find(Hash); It != this->m_BindingSets.end())
				Re = It->second;

			this->m_Mutex.unlock_shared();

			ASSERT(nullptr != Re);
			ASSERT(desc == Re->Get_Desc());

			return Re;

		}

		RHI::RefCountPtr<Imp_BindingSet> GetOrCreateBindingSet(const RHI::RHIBindingSetDesc<APITag>& desc, Imp_BindingSet* layout) {
			Uint64 Hash = 0;
			HashCombine(Hash, HashBindingSet<APITag>{}(desc));
			HashCombine(Hash, HashVoidPtr{}(layout));

			this->m_Mutex.lock_shared();

			RHI::RefCountPtr<Imp_BindingSet> Re{ nullptr };
			if (auto It = this->m_BindingSets.find(Hash); It != this->m_BindingSets.end())
				Re = It->second;

			this->m_Mutex.unlock_shared();

			if (nullptr == Re) {
				m_Mutex.lock();

				RHI::RefCountPtr<Imp_BindingSet>& entry{ this->m_BindingSets[Hash] };
				if (nullptr == entry) {
					Re = this->m_Device->CreateBindingSet(desc, layout);
					entry = Re;
				}
				else
					Re = entry;

				m_Mutex.unlock();
			}

			ASSERT(nullptr != Re);
			ASSERT(desc == Re->Get_Desc());

			return Re;
		}
		
		void Clear(void) {
			this->m_Mutex.lock();
			this->m_BindingSets.clear();
			this->m_Mutex.unlock();
		}

	private:
		RHI::RefCountPtr<Imp_Device> m_Device;
		UnorderedMap<Uint64, RHI::RefCountPtr<Imp_BindingSet>> m_BindingSets;
		SharedMutex m_Mutex;

	};
}