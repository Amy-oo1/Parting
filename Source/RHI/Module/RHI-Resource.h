#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE

PARTING_SUBMODULE(RHI, Resource)

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;
PARTING_IMPORT Concurrent;
PARTING_IMPORT Logger;

#else 
#pragma once
#include "Core/ModuleBuild.h"


#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"
//Global

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#include "Core/Concurrent/Module/Concurrent.h"
#include "Core/Logger/Module/Logger.h"

#endif // PARTING_MODULE_BUILD

//TODO Use namespace in header ,but not in Module .... Module has a func to do
namespace RHI {
	PARTING_EXPORT enum class RHIObjectType :Uint32 {
		D3D12_Device,
		D3D12_CommandQueue,
		D3D12_GraphicsCommandList,
		D3D12_Resource,
		D3D12_RenderTargetViewDescriptor,
		D3D12_DepthStencilViewDescriptor,
		D3D12_ShaderResourceViewGpuDescripror,
		D3D12_UnorderedAccessViewGpuDescripror,
		D3D12_RootSignature,
		D3D12_PipelineState,
		D3D12_CommandAllocator,

		VK_Device,
		VK_PhysicalDevice,
		VK_Instance,
		VK_Queue,
		VK_CommandBuffer,
		VK_DeviceMemory,
		VK_Buffer,
		VK_Image,
		VK_ImageView,
		VK_AccelerationStructureKHR,
		VK_Sampler,
		VK_ShaderModule,
		VK_RenderPass,
		VK_Framebuffer,
		VK_DescriptorPool,
		VK_DescriptorSetLayout,
		VK_DescriptorSet,
		VK_PipelineLayout,
		VK_Pipeline,
		VK_Micromap,
		VK_ImageCreateInfo,

		Count//TODO
	};

	PARTING_EXPORT struct RHIObject final {
		union {
			Uint64 Integer;
			void* Pointer;
		};
		template<typename NativeType>
		operator NativeType* (void)const noexcept { return reinterpret_cast<NativeType*>(this->Pointer); }
	};


	PARTING_EXPORT template<typename Derived>
		class RHIResource :public NonCopyAndMoveAble {
		protected:
			RHIResource(void) = default;
			PARTING_VIRTUAL ~RHIResource(void) = default;

		public:
			STDNODISCARD RHIObject GetNativeObject(RHIObjectType objectType)const noexcept { return this->Get_Derived()->Imp_GetNativeObject(objectType); }

			void AddRef(void)const noexcept { ++this->m_Count; }
			void Release(void)const noexcept {
				if (--this->m_Count == 0)
					delete this->Get_Derived();
			}

		private:
			mutable Atomic<Uint64> m_Count{ 1 };


		private:
			STDNODISCARD Derived* Get_Derived(void)noexcept { return static_cast<Derived*>(this); }
			STDNODISCARD const Derived* Get_Derived(void)const noexcept { return static_cast<const Derived*>(this); }
		private:
			RHIObject Imp_GetNativeObject(RHIObjectType)const noexcept { LOG_ERROR("No Imp");  return RHIObject{}; }

	};

	PARTING_EXPORT template<typename Type>
		class RefCountPtr final {
		template<typename OtherType>
		friend class RefCountPtr;

		public:
			RefCountPtr<Type>(void) = default;

			RefCountPtr<Type>(Nullptr_T)noexcept {}

			RefCountPtr<Type>(Type* ptr)noexcept//TODO : Not saft
				:m_Ptr{ ptr } {
				if (nullptr != this->m_Ptr)
					this->InternalAdd();
			}

			RefCountPtr<Type>(const RefCountPtr<Type>& other)noexcept
				:m_Ptr{ other.m_Ptr } {
				if (nullptr != this->m_Ptr)
					this->InternalAdd();
			}

			RefCountPtr<Type>(RefCountPtr<Type>&& other)noexcept
				:m_Ptr{ other.m_Ptr } {
				other.m_Ptr = nullptr;
			}

			template <typename OtherType>
				requires (ConvertibleTo<OtherType*, Type*>&& ConvertibleTo<Type*, OtherType*>)
			RefCountPtr<Type>(const RefCountPtr<OtherType>& other)noexcept
				:m_Ptr{ other.m_Ptr } {
				if (nullptr != this->m_Ptr)
					this->InternalAdd();
			}

			template <typename OtherType>
				requires (ConvertibleTo<OtherType*, Type*>&& ConvertibleTo<Type*, OtherType*>)
			RefCountPtr<Type>(const RefCountPtr<OtherType>&& other)noexcept
				:m_Ptr{ other.m_Ptr } {
				other.m_Ptr = nullptr;
			}

			template <typename OtherType>
				requires (ConvertibleTo<OtherType*, Type*>&& ConvertibleTo<Type*, OtherType*>)
			RefCountPtr<Type>(const OtherType* other) noexcept//TODO : Not saft
				:m_Ptr{ const_cast<OtherType*>(other) } {
				if (nullptr != this->m_Ptr)
					this->InternalAdd();
			}

			~RefCountPtr(void) noexcept { this->InternalRelease(); }

		public:
			RefCountPtr<Type>& operator=(Nullptr_T) noexcept {
				this->InternalRelease();

				return *this;
			}


			RefCountPtr<Type>& operator=(const RefCountPtr<Type>& other) noexcept {
				if (this->m_Ptr != other.m_Ptr)
					RefCountPtr<Type>{ other }.Swap(*this);

				return *this;
			}

			RefCountPtr<Type>& operator=(RefCountPtr<Type>&& other) noexcept {
				RefCountPtr<Type>{ MoveTemp(other) }.Swap(*this);

				return *this;
			}

			RefCountPtr<Type>& operator=(const Type* other) noexcept {
				if (this->m_Ptr != other)
					RefCountPtr<Type>{ const_cast<Type*>(other) }.Swap(*this);
				return *this;
			}

			template <typename OtherType>
				requires (ConvertibleTo<OtherType*, Type*>&& ConvertibleTo<Type*, OtherType*>)
			RefCountPtr<Type>& operator=(const RefCountPtr<OtherType>& other) noexcept {
				if (this->m_Ptr != other.m_Ptr)
					RefCountPtr<Type>{ other }.Swap(*this);
				return *this;
			}

			template <typename OtherType>
				requires (ConvertibleTo<OtherType*, Type*>&& ConvertibleTo<Type*, OtherType*>)
			RefCountPtr<Type>& operator=(const RefCountPtr<OtherType>&& other) noexcept {
				if (this->m_Ptr != other.m_Ptr)
					RefCountPtr<Type>{ MoveTemp(other) }.Swap(*this);
				return *this;
			}

			template <typename OtherType>
				requires (ConvertibleTo<OtherType*, Type*>&& ConvertibleTo<Type*, OtherType*>)
			RefCountPtr<Type>& operator=(const OtherType* other) noexcept {
				if (this->m_Ptr != other.m_Ptr)
					RefCountPtr<Type>{ const_cast<OtherType*>(other) }.Swap(*this);
				return *this;
			}

			Type* operator->(void)const noexcept { return this->m_Ptr; }

			Type& operator*(void)const noexcept { return *this->m_Ptr; }

			Type** operator& (void)noexcept { return &this->m_Ptr; }

			Type* const* operator& (void)const noexcept { return &this->m_Ptr; }

			operator Type* (void)const noexcept { return this->m_Ptr; }

		public:
			void InternalAdd(void)const noexcept { if (nullptr != this->m_Ptr)this->m_Ptr->AddRef(); }
			void InternalRelease(void) noexcept {
				if (nullptr != this->m_Ptr) {
					this->m_Ptr->Release();
					this->m_Ptr = nullptr;
				}
			}

			//TODO :use std swap
			void Swap(RefCountPtr<Type>& other) noexcept {//Keeep .i Do not use except 
				Type* temp = this->m_Ptr;
				this->m_Ptr = other.m_Ptr;
				other.m_Ptr = temp;
			}

			void Swap(RefCountPtr<Type>&& other) noexcept {
				Type* temp = this->m_Ptr;
				this->m_Ptr = other.m_Ptr;
				other.m_Ptr = temp;
			}

			Type* Get(void) const noexcept { return this->m_Ptr; }

			Type** GetAddress(void) noexcept { return &this->m_Ptr; }//TODO :Remove

			Type* const* GetAddress(void)const noexcept { return &this->m_Ptr; }

			Type* Detach(void) noexcept {
				Type* temp = this->m_Ptr;
				this->m_Ptr = nullptr;
				return temp;
			}

			void Attach(Type* ptr) noexcept {
				if (nullptr != this->m_Ptr)
					this->InternalRelease();
				this->m_Ptr = ptr;
			}

			void Reset(void) noexcept { if (nullptr != this->m_Ptr)this->InternalRelease(); }

		public:
			static RefCountPtr<Type> Create(Type* other)noexcept {
				RefCountPtr<Type> ptr;
				ptr.Attach(other);
				return ptr;
			}


		private:
			Type* m_Ptr{ nullptr };

	};
}