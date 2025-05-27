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

namespace std {
	template<RHI::APITagConcept APITag>
	struct	hash<RHI::RHIBindingSetDesc<APITag>> final {
		size_t operator()(const RHI::RHIBindingSetDesc<APITag>& desc) const noexcept {
			return typename RHI::RHIBindingSetDesc<APITag>::BindingSetHash{}(desc);
		}
	};
}



namespace Parting {

	template<RHI::APITagConcept APITag>
	class BindingCache final {
		using Imp_Device = typename RHI::RHITypeTraits<APITag>::Imp_Device;
		using Imp_BindingLayout = typename RHI::RHITypeTraits<APITag>::Imp_BindingLayout;
		using Imp_BindingSet = typename RHI::RHITypeTraits<APITag>::Imp_BindingSet;
	public:
		BindingCache(Imp_Device* device)
			:m_Device{ device }
		{
		}

		RHI::RefCountPtr<Imp_BindingSet> GetCachedBindingSet(const RHI::RHIBindingSetDesc<APITag>& desc, Imp_BindingLayout* layout) {
			this->m_Mutex.lock_shared();

			RHI::RefCountPtr<Imp_BindingSet> Re{ nullptr };
			if (auto It{ this->m_BindingSets.find(::MakeTuple(desc, layout)) }; It != this->m_BindingSets.end())
				Re = It->second;

			this->m_Mutex.unlock_shared();

			ASSERT(nullptr != Re);
			ASSERT(desc == Re->Get_Desc());

			return Re;

		}

		RHI::RefCountPtr<Imp_BindingSet> GetOrCreateBindingSet(const RHI::RHIBindingSetDesc<APITag>& desc, Imp_BindingLayout* layout) {
			this->m_Mutex.lock_shared();

			RHI::RefCountPtr<Imp_BindingSet> Re{ nullptr };
			if (auto It{ this->m_BindingSets.find(::MakeTuple(desc, layout)) }; It != this->m_BindingSets.end())
				Re = It->second;

			this->m_Mutex.unlock_shared();

			if (nullptr == Re) {
				m_Mutex.lock();

				RHI::RefCountPtr<Imp_BindingSet>& entry{ this->m_BindingSets[::MakeTuple(desc, layout)] };
				if (nullptr == entry) {
					Re = this->m_Device->CreateBindingSet(desc, layout);
					entry = Re;
				}
				else
					Re = entry;

				m_Mutex.unlock();
			}

			ASSERT(nullptr != Re);
			ASSERT(desc == *Re->Get_Desc());

			return Re;
		}

		void Clear(void) {
			this->m_Mutex.lock();
			this->m_BindingSets.clear();
			this->m_Mutex.unlock();
		}

	private:
		RHI::RefCountPtr<Imp_Device> m_Device;
		UnorderedMap<Tuple<RHI::RHIBindingSetDesc<APITag>, Imp_BindingLayout*>, RHI::RefCountPtr<Imp_BindingSet>> m_BindingSets;//TODO :
		SharedMutex m_Mutex;

	};
}