#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"

PARTING_MODULE(StateTracking)

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;
PARTING_IMPORT Algorithm;
PARTING_IMPORT Container;
PARTING_IMPORT Logger;

PARTING_IMPORT RHI;

#else
#pragma once

#include "Core/ModuleBuild.h"

#include "Core/Utility/Include/UtilityMacros.h"
//Global
#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#include "Core/Algorithm/Module/Algorithm.h"
#include "Core/Container/Module/Container.h"
#include "Core/Logger/Module/Logger.h"


#include "RHI/Module/RHI.h"

#endif // PARTING_MODULE_BUILD

namespace RHI {

	PARTING_EXPORT template<APITagConcept APITag>
		struct RHIBufferStateExtension final {
		const RHIBufferDesc& DescRef;
		typename RHITypeTraits<APITag>::Imp_Buffer* ParentBuffer{ nullptr };
		RHIResourceState PermanentState{ RHIResourceState::Unknown };
	};

	PARTING_EXPORT template<APITagConcept APITag>
		struct RHITextureStateExtension final {
		using Imp_Texture = typename RHITypeTraits<APITag>::Imp_Texture;

		const RHITextureDesc& DescRef;
		Imp_Texture* ParentTextureRef;
		RHIResourceState PermanentState{ RHIResourceState::Unknown };
	};


	PARTING_EXPORT struct RHIBufferState final {
		RHIResourceState State{ RHIResourceState::Unknown };
		bool EnableUAVBarriers{ true };
		bool FirstUAVBarrierPlaced{ false };
		bool PermanentTransition{ false };
	};

	PARTING_EXPORT struct RHITextureState final {
		Vector<RHIResourceState> SubresourceStates;
		RHIResourceState State{ RHIResourceState::Unknown };
		bool EnableUAVBarriers{ true };
		bool FirstUAVBarrierPlaced{ false };
		bool PermanentTransition{ false };
	};


	PARTING_EXPORT template<APITagConcept APITag>
		struct RHIBufferBarrier {
		RHIBufferStateExtension<APITag>* Buffer{ nullptr };
		RHIResourceState StateBefore{ RHIResourceState::Unknown };
		RHIResourceState StateAfter{ RHIResourceState::Unknown };
	};

	PARTING_EXPORT template<APITagConcept APITag>
		struct RHITextureBarrier final {
		RHITextureStateExtension<APITag>* Texture{ nullptr };
		Uint32 MipLevel{ 0 };
		Uint32 ArraySlice{ 0 };
		bool EntireTexture = false;
		RHIResourceState StateBefore{ RHIResourceState::Unknown };
		RHIResourceState StateAfter{ RHIResourceState::Unknown };
	};

	PARTING_EXPORT template<APITagConcept APITag>
		class RHICommandListResourceStateTracker final {

		public:
			RHICommandListResourceStateTracker(void) = default;

			~RHICommandListResourceStateTracker(void) = default;


		public:
			void SetEnableUavBarriersForTexture(RHITextureStateExtension<APITag>* texture, bool enableBarriers);
			void SetEnableUavBarriersForBuffer(RHIBufferStateExtension<APITag>* buffer, bool enableBarriers);

			void BeginTrackingTextureState(RHITextureStateExtension<APITag>* texture, RHITextureSubresourceSet subresources, RHIResourceState stateBits);
			void BeginTrackingBufferState(RHIBufferStateExtension<APITag>* buffer, RHIResourceState stateBits);

			void SetPermanentTextureState(RHITextureStateExtension<APITag>* texture, RHITextureSubresourceSet subresources, RHIResourceState stateBits);
			void SetPermanentBufferState(RHIBufferStateExtension<APITag>* buffer, RHIResourceState stateBits);

			STDNODISCARD RHIResourceState Get_TextureSubresourceState(RHITextureStateExtension<APITag>* texture, Uint32 arraySlice, Uint32 mipLevel);
			STDNODISCARD RHIResourceState Get_BufferState(RHIBufferStateExtension<APITag>* buffer);

			void RequireTextureState(RHITextureStateExtension<APITag>* texture, RHITextureSubresourceSet subresources, RHIResourceState state);
			void RequireBufferState(RHIBufferStateExtension<APITag>* buffer, RHIResourceState state);

			void KeepBufferInitialStates(void);
			void KeepTextureInitialStates(void);
			void CommandListSubmitted(void);

			STDNODISCARD const Vector<RHITextureBarrier<APITag>>& Get_TextureBarriers(void) const { return this->m_TextureBarriers; }
			STDNODISCARD const Vector<RHIBufferBarrier<APITag>>& Get_BufferBarriers(void) const { return this->m_BufferBarriers; }
			void ClearBarriers(void) { this->m_TextureBarriers.clear(); this->m_BufferBarriers.clear(); }

		private:
			RHITextureState* Get_TextureStateTracking(RHITextureStateExtension<APITag>* texture, bool allowCreate);
			RHIBufferState* Get_BufferStateTracking(RHIBufferStateExtension<APITag>* buffer, bool allowCreate);

		private:
			UnorderedMap<RHITextureStateExtension<APITag>*, UniquePtr<RHITextureState>> m_TextureStates;
			UnorderedMap<RHIBufferStateExtension<APITag>*, UniquePtr<RHIBufferState>> m_BufferStates;

			// Deferred transitions of textures and buffers to permanent states.
			// They are executed only when the command list is executed, not when the app calls setPermanentTextureState or setPermanentBufferState.
			Vector<Pair<RHITextureStateExtension<APITag>*, RHIResourceState>> m_PermanentTextureStates;
			Vector<Pair<RHIBufferStateExtension<APITag>*, RHIResourceState>> m_PermanentBufferStates;

			Vector<RHITextureBarrier<APITag>> m_TextureBarriers;
			Vector<RHIBufferBarrier<APITag>> m_BufferBarriers;
	};

	STDNODISCARD constexpr Uint32 CalcSubresource(Uint32 mipLevel, Uint32 arraySlice, const RHITextureDesc& desc) { return mipLevel + arraySlice * desc.MipLevelCount; }

	template<APITagConcept APITag>
	inline void RHICommandListResourceStateTracker<APITag>::SetEnableUavBarriersForTexture(RHITextureStateExtension<APITag>* texture, bool enableBarriers) {
		auto Tracking{ Get_TextureStateTracking(texture, true) };

		Tracking->EnableUAVBarriers = enableBarriers;
		Tracking->FirstUAVBarrierPlaced = false;
	}

	template<APITagConcept APITag>
	inline void RHICommandListResourceStateTracker<APITag>::SetEnableUavBarriersForBuffer(RHIBufferStateExtension<APITag>* buffer, bool enableBarriers) {
		auto Tracking{ Get_BufferStateTracking(buffer, true) };

		Tracking->EnableUAVBarriers = enableBarriers;
		Tracking->FirstUAVBarrierPlaced = false;
	}

	template<APITagConcept APITag>
	inline void RHICommandListResourceStateTracker<APITag>::BeginTrackingTextureState(RHITextureStateExtension<APITag>* texture, RHITextureSubresourceSet subresources, RHIResourceState stateBits) {
		const auto& Desc{ texture->DescRef };

		auto Tracking{ this->Get_TextureStateTracking(texture, true) };

		subresources = subresources.Resolve(Desc, false);

		if (subresources.Is_EntireTexture(Desc)) {
			Tracking->State = stateBits;
			Tracking->SubresourceStates.clear();
		}
		else {
			Tracking->SubresourceStates.resize(static_cast<Uint64>(Desc.MipLevelCount * Desc.ArrayCount), Tracking->State);
			Tracking->State = RHIResourceState::Unknown;

			for (auto MipLevel = subresources.BaseMipLevel; MipLevel < subresources.BaseMipLevel + subresources.MipLevelCount; ++MipLevel)
				for (auto ArraySlice = subresources.BaseArraySlice; ArraySlice < subresources.BaseArraySlice + subresources.ArraySliceCount; ++ArraySlice) {
					Uint32 Subresource{ CalcSubresource(MipLevel, ArraySlice, Desc) };
					Tracking->SubresourceStates[Subresource] = stateBits;
				}
		}

	}

	template<APITagConcept APITag>
	inline void RHICommandListResourceStateTracker<APITag>::BeginTrackingBufferState(RHIBufferStateExtension<APITag>* buffer, RHIResourceState stateBits) {
		auto Tracking{ this->Get_BufferStateTracking(buffer, true) };

		Tracking->State = stateBits;
	}

	template<APITagConcept APITag>
	inline void RHICommandListResourceStateTracker<APITag>::SetPermanentTextureState(RHITextureStateExtension<APITag>* texture, RHITextureSubresourceSet subresources, RHIResourceState stateBits) {
		const auto& desc{ texture->DescRef };

		subresources = subresources.Resolve(desc, false);

		bool permanent{ true };
		if (!subresources.Is_EntireTexture(desc)) {
			LOG_ERROR("SetPermanentTextureState: Permanent state can only be set for entire texture. Subresources are ignored.");

			permanent = false;
		}

		this->RequireTextureState(texture, subresources, stateBits);

		if (permanent) {
			this->m_PermanentTextureStates.push_back(MakePair(texture, stateBits));
			this->Get_TextureStateTracking(texture, true)->PermanentTransition = true;
		}
	}

	template<APITagConcept APITag>
	inline void RHICommandListResourceStateTracker<APITag>::SetPermanentBufferState(RHIBufferStateExtension<APITag>* buffer, RHIResourceState stateBits) {
		this->RequireBufferState(buffer, stateBits);

		this->m_PermanentBufferStates.push_back(MakePair(buffer, stateBits));
	}

	template<APITagConcept APITag>
	inline RHIResourceState RHICommandListResourceStateTracker<APITag>::Get_TextureSubresourceState(RHITextureStateExtension<APITag>* texture, Uint32 arraySlice, Uint32 mipLevel) {
		auto tracking{ this->Get_TextureStateTracking(texture, false) };
		if (!tracking)
			return texture->DescRef.KeepInitialState ? texture->DescRef.InitialState : RHIResourceState::Unknown;

		// whole resource
		if (tracking->SubresourceStates.empty())
			return tracking->State;

		Uint32 subresource{ CalcSubresource(mipLevel, arraySlice, texture->DescRef) };
		return tracking->SubresourceStates[subresource];
	}

	template<APITagConcept APITag>
	inline RHIResourceState RHICommandListResourceStateTracker<APITag>::Get_BufferState(RHIBufferStateExtension<APITag>* buffer) {
		auto tracking{ this->Get_BufferStateTracking(buffer, false) };

		return tracking ? tracking->State : RHIResourceState::Unknown;
	}

	template<APITagConcept APITag>
	inline void RHICommandListResourceStateTracker<APITag>::RequireTextureState(RHITextureStateExtension<APITag>* texture, RHITextureSubresourceSet subresources, RHIResourceState state) {
		if (RHIResourceState::Unknown != texture->PermanentState) {
			// If the texture has a permanent state, we don't need to track it anymore
			ASSERT((state & texture->PermanentState) == state);
			return;
		}

		subresources = subresources.Resolve(texture->DescRef, false);

		auto tracking{ this->Get_TextureStateTracking(texture, true) };

		if (subresources.Is_EntireTexture(texture->DescRef) && tracking->SubresourceStates.empty()) {
			// We're requiring state for the entire texture, and it's been tracked as entire texture too

			bool transitionNecessary{ tracking->State != state };
			bool uavNecessary{
				(RHIResourceState::Unknown != (state & RHIResourceState::UnorderedAccess)) &&
				(tracking->EnableUAVBarriers || !tracking->FirstUAVBarrierPlaced)
			};

			if (transitionNecessary || uavNecessary)
				this->m_TextureBarriers.emplace_back(RHITextureBarrier<APITag>{
				.Texture{ texture },
					.EntireTexture{ true },
					.StateBefore{ tracking->State },
					.StateAfter{ state }
			});

			tracking->State = state;

			if (uavNecessary && !transitionNecessary)
				tracking->FirstUAVBarrierPlaced = true;
		}
		else {
			// Transition individual subresources

			// Make sure that we're tracking the texture on subresource level
			bool stateExpanded{ false };
			if (tracking->SubresourceStates.empty()) {
				if (RHIResourceState::Unknown == tracking->State)
					LOG_ERROR("RequireTextureState: Texture state is unknown. Call CommandList::beginTrackingTextureState(...) before using the texture or use the keepInitialState and initialState members of TextureDesc.");

				tracking->SubresourceStates.resize(static_cast<Uint64>(texture->DescRef.MipLevelCount) * texture->DescRef.ArrayCount, tracking->State);
				tracking->State = RHIResourceState::Unknown;
				stateExpanded = true;
			}

			bool anyUavBarrier{ false };

			for (auto arraySlice = subresources.BaseArraySlice; arraySlice < subresources.BaseArraySlice + subresources.ArraySliceCount; ++arraySlice)
				for (auto mipLevel = subresources.BaseMipLevel; mipLevel < subresources.BaseMipLevel + subresources.MipLevelCount; ++mipLevel) {
					Uint32 subresourceIndex{ CalcSubresource(mipLevel, arraySlice, texture->DescRef) };

					auto priorState{ tracking->SubresourceStates[subresourceIndex] };

					if (RHIResourceState::Unknown == priorState && !stateExpanded)
						LOG_ERROR("RequireTextureState: Texture state is unknown. Call CommandList::beginTrackingTextureState(...) before using the texture or use the keepInitialState and initialState members of TextureDesc.");

					bool transitionNecessary{ priorState != state };
					bool uavNecessary{
						(RHIResourceState::Unknown != (state & RHIResourceState::UnorderedAccess)) &&
						!anyUavBarrier && (tracking->EnableUAVBarriers || !tracking->FirstUAVBarrierPlaced)
					};

					if (transitionNecessary || uavNecessary)
						this->m_TextureBarriers.emplace_back(RHITextureBarrier<APITag>{
						.Texture{ texture },
							.MipLevel{ mipLevel },
							.ArraySlice{ arraySlice },
							.EntireTexture{ false },
							.StateBefore{ priorState },
							.StateAfter{ state }
					});

					tracking->SubresourceStates[subresourceIndex] = state;

					if (uavNecessary && !transitionNecessary) {
						anyUavBarrier = true;
						tracking->FirstUAVBarrierPlaced = true;
					}
				}
		}
	}

	template<APITagConcept APITag>
	inline void RHICommandListResourceStateTracker<APITag>::RequireBufferState(RHIBufferStateExtension<APITag>* buffer, RHIResourceState state) {
		if (buffer->DescRef.IsVolatile)
			return;

		if (RHIResourceState::Unknown != buffer->PermanentState) {
			// If the texture has a permanent state, we don't need to track it anymore
			ASSERT((state & buffer->PermanentState) == state);
			return;
		}

		// CPU-visible buffers can't change state
		if (RHICPUAccessMode::None != buffer->DescRef.CPUAccess)
			return;

		auto tracking{ this->Get_BufferStateTracking(buffer, true) };

		if (RHIResourceState::Unknown == tracking->State)
			LOG_ERROR("RequireBufferState: Buffer state is unknown. Call CommandList::beginTrackingBufferState(...) before using the buffer or use the keepInitialState and initialState members of BufferDesc.");

		bool transitionNecessary{ tracking->State != state };
		bool uavNecessary{
			(RHIResourceState::Unknown != (state & RHIResourceState::UnorderedAccess)) &&
			(tracking->EnableUAVBarriers || !tracking->FirstUAVBarrierPlaced)
		};

		// See if this buffer is already used for a different purpose in this batch.
		// If it is, combine the state bits.
		// Example: same buffer used as index and vertex buffer, or as SRV and indirect arguments.
		if (transitionNecessary)
			for (auto& barrier : this->m_BufferBarriers)
				if (barrier.Buffer == buffer) {
					barrier.StateAfter = barrier.StateAfter | state;
					tracking->State = barrier.StateAfter;

					return;
				}

		if (transitionNecessary || uavNecessary)
			this->m_BufferBarriers.emplace_back(RHIBufferBarrier<APITag>{
			.Buffer{ buffer },
				.StateBefore{ tracking->State },
				.StateAfter{ state }
		}
			);

		if (uavNecessary && !transitionNecessary)
			tracking->FirstUAVBarrierPlaced = true;

		tracking->State = state;
	}

	template<APITagConcept APITag>
	inline void RHICommandListResourceStateTracker<APITag>::KeepBufferInitialStates(void) {
		for (auto& [buffer, tracking] : this->m_BufferStates)
			if (buffer->DescRef.KeepInitialState &&
				RHIResourceState::Unknown == buffer->PermanentState &&
				!buffer->DescRef.IsVolatile &&
				!tracking->PermanentTransition)
				this->RequireBufferState(buffer, buffer->DescRef.InitialState);
	}

	template<APITagConcept APITag>
	inline void RHICommandListResourceStateTracker<APITag>::KeepTextureInitialStates(void) {
		for (auto& [texture, tracking] : this->m_TextureStates)
			if (texture->DescRef.KeepInitialState &&
				RHIResourceState::Unknown == texture->PermanentState &&
				!tracking->PermanentTransition)
				this->RequireTextureState(texture, g_AllSubResourceSet, texture->DescRef.InitialState);
	}

	template<APITagConcept APITag>
	inline void RHICommandListResourceStateTracker<APITag>::CommandListSubmitted(void) {
		for (auto& [texture, state] : this->m_PermanentTextureStates) {
			if (RHIResourceState::Unknown != texture->PermanentState && texture->PermanentState != state) {
				LOG_ERROR("Attempted to switch permanent state of texture");

				continue;
			}

			texture->PermanentState = state;
		}
		this->m_PermanentTextureStates.clear();

		for (auto& [buffer, state] : this->m_PermanentBufferStates) {
			if (RHIResourceState::Unknown != buffer->PermanentState && buffer->PermanentState != state) {
				LOG_ERROR("Attempted to switch permanent state of buffer");

				continue;
			}

			buffer->PermanentState = state;
		}
		this->m_PermanentBufferStates.clear();

		this->m_TextureStates.clear();
		this->m_BufferStates.clear();
	}

	template<APITagConcept APITag>
	inline RHITextureState* RHICommandListResourceStateTracker<APITag>::Get_TextureStateTracking(RHITextureStateExtension<APITag>* texture, bool allowCreate) {
		if (auto It = this->m_TextureStates.find(texture); It != this->m_TextureStates.end())
			return It->second.get();

		if (!allowCreate)
			return nullptr;

		auto TrackingRef{ MakeUnique<RHITextureState>() };

		auto Tracking{ TrackingRef.get() };
		this->m_TextureStates.insert(MakePair(texture, MoveTemp(TrackingRef)));

		if (texture->DescRef.KeepInitialState)
			Tracking->State = texture->DescRef.InitialState;

		return Tracking;
	}

	template<APITagConcept APITag>
	inline RHIBufferState* RHICommandListResourceStateTracker<APITag>::Get_BufferStateTracking(RHIBufferStateExtension<APITag>* buffer, bool allowCreate) {
		if (auto It = this->m_BufferStates.find(buffer); It != this->m_BufferStates.end())
			return It->second.get();

		if (!allowCreate)
			return nullptr;

		auto TrackingRef{ MakeUnique<RHIBufferState>() };

		auto Tracking{ TrackingRef.get() };
		this->m_BufferStates.insert(MakePair(buffer, MoveTemp(TrackingRef)));

		if (buffer->DescRef.KeepInitialState)
			Tracking->State = buffer->DescRef.InitialState;

		return Tracking;
	}

}