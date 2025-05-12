#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"


PARTING_SUBMODULE(D3D12RHI, ShaderBinding)

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;
PARTING_IMPORT Algorithm;
PARTING_IMPORT Container;
PARTING_IMPORT String;
PARTING_IMPORT Logger;

PARTING_IMPORT RHI;

PARTING_SUBMODE_IMPORT(Traits)
PARTING_SUBMODE_IMPORT(Common)
PARTING_SUBMODE_IMPORT(Buffer)
PARTING_SUBMODE_IMPORT(Texture)
PARTING_SUBMODE_IMPORT(Sampler)

#else
#pragma once

#include "Core/ModuleBuild.h"

#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"
//Global
#include "D3D12RHI/Module/DirectX12Wrapper.h"

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#include "Core/Algorithm/Module/Algorithm.h"
#include "Core/Container/Module/Container.h"
#include "Core/String/Module/String.h"
#include "Core/Logger/Module/Logger.h"

#include "RHI/Module/RHI.h"

#include "D3D12RHI/Module/D3D12RHI-Traits.h"
#include "D3D12RHI/Module/D3D12RHI-Common.h"
#include "D3D12RHI/Module/D3D12RHI-Buffer.h"
#include "D3D12RHI/Module/D3D12RHI-Texture.h"
#include "D3D12RHI/Module/D3D12RHI-Sampler.h"

#endif // PARTING_MODULE_BUILD

namespace RHI::D3D12 {

	RHIResourceType Get_NormalizedResourceType(RHIResourceType type)noexcept {
		switch (type) {
		case RHIResourceType::StructuredBuffer_UAV:case RHIResourceType::RawBuffer_UAV:
			return RHIResourceType::TypedBuffer_UAV;
		case RHIResourceType::StructuredBuffer_SRV:case RHIResourceType::RawBuffer_SRV:
			return RHIResourceType::TypedBuffer_SRV;
		default:
			return type;
		}
	}

	bool AreResourceTypesCompatible(RHIResourceType a, RHIResourceType b) {
		if (a == b)
			return true;

		a = Get_NormalizedResourceType(a);
		b = Get_NormalizedResourceType(b);

		using enum RHIResourceType;

		if ((a == TypedBuffer_SRV && b == Texture_SRV) ||
			(b == TypedBuffer_SRV && a == Texture_SRV) ||
			(a == TypedBuffer_SRV && b == RayTracingAccelStruct) ||
			(a == Texture_SRV && b == RayTracingAccelStruct) ||
			(b == TypedBuffer_SRV && a == RayTracingAccelStruct) ||
			(b == Texture_SRV && a == RayTracingAccelStruct))
			return true;

		if ((a == TypedBuffer_UAV && b == Texture_UAV) ||
			(b == TypedBuffer_UAV && a == Texture_UAV))
			return true;

		return false;
	}



	class BindingLayout final :public RHIBindingLayout<BindingLayout> {
		friend class RHIResource<BindingLayout>;
		friend class RHIBindingLayout<BindingLayout>;

		friend class BindingSet;
		friend class CommandList;
		friend class Device;
	public:
		using D3D12RootParameterIndex = D3D12RootSignature::D3D12RootParameterIndex;

	public:
		BindingLayout(const RHIBindingLayoutDesc& desc);
		~BindingLayout(void) = default;

	public:

	private:

	private:
		RHIBindingLayoutDesc m_Desc{};

		Uint32 m_PushConstantByteSize{ 0 };
		D3D12RootParameterIndex m_RootParameterPushConstants{ ~0u };
		D3D12RootParameterIndex m_RootParameterSRVetc{ ~0u };
		D3D12RootParameterIndex m_RootParameterSamplers{ ~0u };

		Uint32 m_DescriptorTableSizeSRVetc{ 0 };
		Uint32 m_DescriptorTableSizeSamplers{ 0 };

		Vector<D3D12_DESCRIPTOR_RANGE1> m_DescriptorRangesSRVetc;
		Vector<D3D12_DESCRIPTOR_RANGE1> m_DescriptorRangesSamplers;
		Vector<RHIBindingLayoutItem> m_BindingLayoutsSRVetc;

		Array<Pair<D3D12RootParameterIndex, D3D12_ROOT_DESCRIPTOR1>, g_MaxVolatileConstantBufferCountPerLayout> m_RootParametersVolatileCB;
		RemoveCV<decltype(g_MaxVolatileConstantBufferCountPerLayout)>::type m_VolatileCBCount{ 0 };

		Array<D3D12_ROOT_PARAMETER1, g_D3D12MaxRootParameterWordCount> m_RootParameters;
		RemoveCV<decltype(g_D3D12MaxRootParameterWordCount)>::type m_RootParameterCount{ 0 };

	private:
		RHIObject Imp_GetNativeObject(RHIObjectType)const noexcept { LOG_ERROR("Imp But Empty");  return RHIObject{}; }

		const RHIBindingLayoutDesc* Imp_Get_Desc(void)const { return &this->m_Desc; };
	};

	//Src


	//Imp
	inline BindingLayout::BindingLayout(const RHIBindingLayoutDesc& desc) :
		RHIBindingLayout<BindingLayout>{},
		m_Desc{ desc } {

		//must to do ,becase I will add Range  if Type and Slot is same with last added binding
		auto CurrentType{ RHIResourceType::None };
		Uint32 CurrentSlot{ ~0u };

		D3D12_ROOT_CONSTANTS RootConstants{};

		for (Uint32 Index = 0; Index < this->m_Desc.BindingCount; ++Index) {
			const auto& BindingItem{ this->m_Desc.Bindings[Index] };

			if (RHIResourceType::VolatileConstantBuffer == BindingItem.Type)
				this->m_RootParametersVolatileCB[this->m_VolatileCBCount++] = MakePair(
					Max_Uint32,
					D3D12_ROOT_DESCRIPTOR1{
						.ShaderRegister { BindingItem.Slot },
						.RegisterSpace { this->m_Desc.RegisterSpace },
						.Flags { D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC },
					});
			else if (RHIResourceType::PushConstants == BindingItem.Type) {
				this->m_PushConstantByteSize = BindingItem.ByteSize;

				RootConstants.ShaderRegister = BindingItem.Slot;
				RootConstants.RegisterSpace = this->m_Desc.RegisterSpace;
				RootConstants.Num32BitValues = BindingItem.ByteSize / sizeof(Uint32);
			}
			else if (!AreResourceTypesCompatible(BindingItem.Type, CurrentType) || BindingItem.Slot != CurrentSlot + 1) {
				if (RHIResourceType::Sampler == BindingItem.Type)
					this->m_DescriptorRangesSamplers.emplace_back(D3D12_DESCRIPTOR_RANGE1{
							.RangeType{ D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER },
							.NumDescriptors { 1 },
							.BaseShaderRegister { BindingItem.Slot },
							.RegisterSpace { this->m_Desc.RegisterSpace },
							.Flags{ D3D12_DESCRIPTOR_RANGE_FLAG_NONE },
							.OffsetInDescriptorsFromTableStart{ this->m_DescriptorTableSizeSamplers++ }
						}
					);
				else {
					D3D12_DESCRIPTOR_RANGE_TYPE TempRangeType{};
					switch (BindingItem.Type) {
						using enum RHIResourceType;
					case Texture_SRV:case TypedBuffer_SRV:case StructuredBuffer_SRV:case RawBuffer_SRV:case RayTracingAccelStruct:
						TempRangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
						break;

					case Texture_UAV:case TypedBuffer_UAV:case StructuredBuffer_UAV:case RawBuffer_UAV:case SamplerFeedbackTexture_UAV:
						TempRangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
						break;

					case ConstantBuffer:
						TempRangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
						break;

					case None:case VolatileConstantBuffer:case Sampler:case PushConstants:case Count:default:
						ASSERT(false);
						break;
					}

					this->m_DescriptorRangesSRVetc.emplace_back(D3D12_DESCRIPTOR_RANGE1{
							.RangeType{ TempRangeType },
							.NumDescriptors { 1 },
							.BaseShaderRegister { BindingItem.Slot },
							.RegisterSpace { this->m_Desc.RegisterSpace },
							.Flags{ D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE },//use it to doing
							.OffsetInDescriptorsFromTableStart{ this->m_DescriptorTableSizeSRVetc++ }
						}
					);

					this->m_BindingLayoutsSRVetc.emplace_back(BindingItem);
				}

				CurrentType = BindingItem.Type;
				CurrentSlot = BindingItem.Slot;
			}
			else {
				if (RHIResourceType::Sampler == BindingItem.Type) {
					this->m_DescriptorRangesSamplers.back().NumDescriptors++;
					++this->m_DescriptorTableSizeSamplers;

					//tje same sampler not should add in table
				}
				else {
					this->m_DescriptorRangesSRVetc.back().NumDescriptors++;
					++this->m_DescriptorTableSizeSRVetc;

					this->m_BindingLayoutsSRVetc.emplace_back(BindingItem);
				}

				CurrentSlot = BindingItem.Slot;
			}
		}

		this->m_RootParameterCount = 0;//TODO :Remove

		if (RootConstants.Num32BitValues > 0) {
			this->m_RootParameters[this->m_RootParameterCount++] = D3D12_ROOT_PARAMETER1{
				.ParameterType{ D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS },
				.Constants{ RootConstants },
				.ShaderVisibility{ ConvertShaderStage(this->m_Desc.Visibility)}
			};

			this->m_RootParameterPushConstants = this->m_RootParameterCount - 1;
		}

		for (Uint32 Index = 0; Index < this->m_VolatileCBCount; ++Index) {
			auto& [RootParameterIndex, d3d12RootDescriptor] = this->m_RootParametersVolatileCB[Index];

			this->m_RootParameters[this->m_RootParameterCount++] = D3D12_ROOT_PARAMETER1{
				.ParameterType{ D3D12_ROOT_PARAMETER_TYPE_CBV },
				.Descriptor = { d3d12RootDescriptor },
				.ShaderVisibility{ ConvertShaderStage(this->m_Desc.Visibility) }
			};

			RootParameterIndex = this->m_RootParameterCount - 1;
		}

		if (this->m_DescriptorTableSizeSamplers > 0) {
			this->m_RootParameters[this->m_RootParameterCount++] = D3D12_ROOT_PARAMETER1{
				.ParameterType{ D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE },
				.DescriptorTable{
					.NumDescriptorRanges { static_cast<Uint32>(this->m_DescriptorRangesSamplers.size()) },
					.pDescriptorRanges { this->m_DescriptorRangesSamplers.data() }
				},
				.ShaderVisibility{ ConvertShaderStage(this->m_Desc.Visibility) }
			};

			this->m_RootParameterSamplers = this->m_RootParameterCount - 1;
		}

		if (this->m_DescriptorTableSizeSRVetc > 0) {
			this->m_RootParameters[this->m_RootParameterCount++] = D3D12_ROOT_PARAMETER1{
				.ParameterType{ D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE },
				.DescriptorTable{
					.NumDescriptorRanges { static_cast<Uint32>(this->m_DescriptorRangesSRVetc.size()) },
					.pDescriptorRanges { this->m_DescriptorRangesSRVetc.data() }
				},
				.ShaderVisibility{ ConvertShaderStage(this->m_Desc.Visibility) }
			};

			this->m_RootParameterSRVetc = this->m_RootParameterCount - 1;
		}
	}



	class BindLessLayout final :public RHIBindlessLayout<BindLessLayout> {
		friend class RHIResource<BindLessLayout>;
		friend class RHIBindlessLayout<BindLessLayout>;

		friend class BindingSet;
	public:
		using D3D12RootParameterIndex = D3D12RootSignature::D3D12RootParameterIndex;

	public:
		BindLessLayout(const RHIBindlessLayoutDesc& desc);
		~BindLessLayout(void) = default;

	public:


	private:

	private:
		RHIBindlessLayoutDesc m_Desc{};

		Array<D3D12_DESCRIPTOR_RANGE1, g_D3D12MaxRootParameterWordCount> m_DescriptorRanges;
		RemoveCV<decltype(g_D3D12MaxRootParameterWordCount)>::type m_DescriptorRangeCount{ 0 };

		D3D12_ROOT_PARAMETER1 m_RootParameter{};

	private:
		RHIObject Imp_GetNativeObject(RHIObjectType)const noexcept { LOG_ERROR("Imp But Empty");  return RHIObject{}; }

		const RHIBindlessLayoutDesc* Imp_Get_Desc(void)const { return &this->m_Desc; };
	};


	//Src


	//Imp

	inline BindLessLayout::BindLessLayout(const RHIBindlessLayoutDesc& desc) :
		RHIBindlessLayout<BindLessLayout>{},
		m_Desc{ desc } {

		for (Uint32 Index = 0; Index < this->m_Desc.BindlessLayoutCount; ++Index) {
			const auto& BindItem{ this->m_Desc.BindlessLayouts[Index] };

			D3D12_DESCRIPTOR_RANGE_TYPE rangeType;

			switch (BindItem.Type) {
				using enum RHIResourceType;
			case Texture_SRV:
			case TypedBuffer_SRV:
			case StructuredBuffer_SRV:
			case RawBuffer_SRV:
			case RayTracingAccelStruct:
				rangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
				break;

			case ConstantBuffer:
				rangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
				break;

			case Texture_UAV:
			case TypedBuffer_UAV:
			case StructuredBuffer_UAV:
			case RawBuffer_UAV:
			case SamplerFeedbackTexture_UAV:
				rangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
				break;

			case Sampler:
				rangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
				break;

			case None:
			case VolatileConstantBuffer:
			case PushConstants:
			case Count:
			default:
				ASSERT(false);
			}

			this->m_DescriptorRanges[this->m_DescriptorRangeCount++] = D3D12_DESCRIPTOR_RANGE1{
				.RangeType{ rangeType },
				.NumDescriptors { ~0u },
				.BaseShaderRegister {this->m_Desc.FirstSlot },
				.RegisterSpace { BindItem.Slot },
				.Flags{ D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE },
				.OffsetInDescriptorsFromTableStart{ 0 }
			};

		}

		this->m_RootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		this->m_RootParameter.DescriptorTable.NumDescriptorRanges = this->m_DescriptorRangeCount;
		this->m_RootParameter.DescriptorTable.pDescriptorRanges = this->m_DescriptorRanges.data();
		this->m_RootParameter.ShaderVisibility = ConvertShaderStage(this->m_Desc.Visibility);

	}

	class BindingSet final :public RHIBindingSet<BindingSet, D3D12Tag> {
		friend class RHIResource<BindingSet>;
		friend class RHIBindingSet<BindingSet, D3D12Tag>;

		friend class CommandList;
	public:
		BindingSet(const Context& context, D3D12DeviceResources& resources) :
			RHIBindingSet<BindingSet, D3D12Tag>{},
			m_Context{ context },
			m_DeviceResourcesRef{ resources }{
		}

		~BindingSet(void) {
			this->m_DeviceResourcesRef.ShaderResourceViewHeap.ReleaseDescriptor(this->m_DescriptorTableSRVetc, this->m_Layout->m_DescriptorTableSizeSRVetc);
			this->m_DeviceResourcesRef.SamplerHeap.ReleaseDescriptor(this->m_DescriptorTableSamplers, this->m_Layout->m_DescriptorTableSizeSamplers);
		}


	public:
		using D3D12RootParameterIndex = D3D12RootSignature::D3D12RootParameterIndex;

	public:
		void CreateDescriptors(void);

	private:

	private:
		const Context& m_Context{};
		D3D12DeviceResources& m_DeviceResourcesRef;

		RHIBindingSetDesc<D3D12Tag> m_Desc{};
		RefCountPtr<BindingLayout> m_Layout{ nullptr };

		D3D12DescriptorIndex m_DescriptorTableSRVetc{ 0 };
		D3D12DescriptorIndex m_DescriptorTableSamplers{ 0 };
		D3D12DescriptorIndex m_RootParameterIndexSRVetc{ 0 };
		D3D12DescriptorIndex m_RootParameterIndexSamplers{ 0 };
		bool m_DescriptorTableValidSRVetc{ false };
		bool m_DescriptorTableValidSamplers{ false };
		bool m_HasUavBindings{ false };

		Array<Pair<D3D12RootParameterIndex, Buffer*>, g_MaxVolatileConstantBufferCountPerLayout> m_RootParametersVolatileCB;
		RemoveCV<decltype(g_MaxVolatileConstantBufferCountPerLayout)>::type m_VolatileCBCount{ 0 };

		Vector<RHIShaderBindingResources<D3D12Tag>> m_Resources;

		Vector<Uint16> m_BindingsThatNeedTransitions;


	private:
		RHIObject Imp_GetNativeObject(RHIObjectType)const noexcept { LOG_ERROR("Imp But Empty");  return RHIObject{}; }

		const RHIBindingSetDesc<D3D12Tag>* Imp_Get_Desc(void)const { return &this->m_Desc; };
		const BindingLayout* Imp_Get_Layout(void)const { return this->m_Layout.Get(); };
	};

	//Src
	void BindingSet::CreateDescriptors(void) {
		for (Uint32 Index = 0; Index < this->m_Layout->m_VolatileCBCount; ++Index) {
			const auto& [RootParameterIndex, d3d12RootDescriptor] = this->m_Layout->m_RootParametersVolatileCB[Index];

			Buffer* FoundBuffer{ nullptr };
			for (Uint32 BingdIndx = 0; BingdIndx < this->m_Desc.BindingCount; ++BingdIndx) {
				const auto& BindingItem{ this->m_Desc.Bindings[BingdIndx] };
				if (RHIResourceType::VolatileConstantBuffer == BindingItem.Type && BindingItem.Slot == d3d12RootDescriptor.ShaderRegister) {
					this->m_Resources.push_back(BindingItem.ResourcePtr);

					FoundBuffer = Get<RefCountPtr<Buffer>>(BindingItem.ResourcePtr).Get();
					break;
				}
			}

			this->m_RootParametersVolatileCB[this->m_VolatileCBCount++] = MakePair(RootParameterIndex, FoundBuffer);
		}

		if (this->m_Layout->m_DescriptorTableSizeSamplers > 0) {
			auto descriptorTableBaseIndex{ this->m_DeviceResourcesRef.SamplerHeap.AllocateDescriptor(this->m_Layout->m_DescriptorTableSizeSamplers) };

			this->m_DescriptorTableSamplers = descriptorTableBaseIndex;
			this->m_RootParameterIndexSamplers = this->m_Layout->m_RootParameterSamplers;
			this->m_DescriptorTableValidSamplers = true;

			for (const auto& Range : this->m_Layout->m_DescriptorRangesSamplers)
				for (Uint32 ItemRange = 0; ItemRange < Range.NumDescriptors; ++ItemRange) {
					Uint32 Slot{ Range.BaseShaderRegister + ItemRange };
					bool Found{ false };

					auto descriptorHandle{ this->m_DeviceResourcesRef.SamplerHeap.Get_CPUHandle(descriptorTableBaseIndex + Range.OffsetInDescriptorsFromTableStart + ItemRange) };

					for (Uint32 BindIndx = 0; BindIndx < this->m_Desc.BindingCount; ++BindIndx) {
						auto& BindingItem{ this->m_Desc.Bindings[BindIndx] };
						if (RHIResourceType::Sampler == BindingItem.Type && BindingItem.Slot == Slot) {
							this->m_Resources.push_back(BindingItem.ResourcePtr);
							Get<RefCountPtr<Sampler>>(BindingItem.ResourcePtr)->CreateDescriptor(descriptorHandle);
							Found = true;
							break;
						}

						if (!Found) {
							D3D12_SAMPLER_DESC DefaultSamplerDesc{};
							this->m_Context.Device->CreateSampler(&DefaultSamplerDesc, descriptorHandle);
						}
					}
				}

			this->m_DeviceResourcesRef.SamplerHeap.CopyToShaderVisibleHeap(descriptorTableBaseIndex, this->m_Layout->m_DescriptorTableSizeSamplers);
		}

		if (this->m_Layout->m_DescriptorTableSizeSRVetc > 0) {
			auto descriptorTableBaseIndex{ this->m_DeviceResourcesRef.ShaderResourceViewHeap.AllocateDescriptor(this->m_Layout->m_DescriptorTableSizeSRVetc) };

			this->m_DescriptorTableSRVetc = descriptorTableBaseIndex;
			this->m_RootParameterIndexSRVetc = this->m_Layout->m_RootParameterSRVetc;
			this->m_DescriptorTableValidSRVetc = true;

			for (const auto& Range : this->m_Layout->m_DescriptorRangesSRVetc)
				for (Uint32 ItemRange = 0; ItemRange < Range.NumDescriptors; ++ItemRange) {
					Uint32 Slot{ Range.BaseShaderRegister + ItemRange };
					bool Found{ false };
					auto descriptorHandle{ this->m_DeviceResourcesRef.ShaderResourceViewHeap.Get_CPUHandle(descriptorTableBaseIndex + Range.OffsetInDescriptorsFromTableStart + ItemRange) };

					RHIShaderBindingResources<D3D12Tag> pResource{ nullptr };

					for (Uint32 BindIndex = 0; BindIndex < this->m_Desc.BindingCount; ++BindIndex) {
						const auto& Binding{ this->m_Desc.Bindings[BindIndex] };

						if (Binding.Slot != Slot)
							continue;

						const auto BindingType{ Get_NormalizedResourceType(Binding.Type) };
						using enum RHIResourceType;
						if (D3D12_DESCRIPTOR_RANGE_TYPE_SRV == Range.RangeType && TypedBuffer_SRV == BindingType) {
							if (!HoldsAlternative<Nullptr_T>(Binding.ResourcePtr)) {
								pResource = Binding.ResourcePtr;

								Buffer* TempBuffer{ Get<RefCountPtr<Buffer>>(Binding.ResourcePtr).Get() };
								TempBuffer->CreateSRV(descriptorHandle, Binding.Range, Binding.Format, Binding.Type);

								if (RHIResourceState::Unknown == TempBuffer->m_StateExtension.PermanentState)
									this->m_BindingsThatNeedTransitions.push_back(static_cast<Uint16>(BindIndex));
								else
									ASSERT(false);//TODO :
							}
							else
								Buffer::CreateNullSRV(descriptorHandle, Binding.Format, this->m_Context);

							Found = true;
							break;
						}
						else if (D3D12_DESCRIPTOR_RANGE_TYPE_UAV == Range.RangeType && TypedBuffer_UAV == BindingType) {
							if (!HoldsAlternative<Nullptr_T>(Binding.ResourcePtr)) {
								pResource = Binding.ResourcePtr;

								Buffer* TempBuffer{ Get<RefCountPtr<Buffer>>(Binding.ResourcePtr).Get() };
								TempBuffer->CreateUAV(descriptorHandle, Binding.Range, Binding.Format, Binding.Type);

								if (RHIResourceState::Unknown == TempBuffer->m_StateExtension.PermanentState)
									this->m_BindingsThatNeedTransitions.push_back(static_cast<Uint16>(BindIndex));
								else
									ASSERT(false);//TODO :
							}
							else
								Buffer::CreateNullUAV(descriptorHandle, Binding.Format, this->m_Context);

							this->m_HasUavBindings = true;
							Found = true;
							break;
						}
						else if (D3D12_DESCRIPTOR_RANGE_TYPE_SRV == Range.RangeType && Texture_SRV == BindingType) {
							pResource = Binding.ResourcePtr;

							Texture* TempTexture{ Get<RefCountPtr<Texture>>(Binding.ResourcePtr).Get() };
							TempTexture->CreateSRV(descriptorHandle, Binding.Format, Binding.Dimension, Binding.Subresources);

							if (RHIResourceState::Unknown == TempTexture->m_StateExtension.PermanentState)
								this->m_BindingsThatNeedTransitions.push_back(static_cast<Uint16>(BindIndex));
							else
								ASSERT(false);//TODO :

							Found = true;
							break;
						}
						else if (D3D12_DESCRIPTOR_RANGE_TYPE_UAV == Range.RangeType && Texture_UAV == BindingType) {
							pResource = Binding.ResourcePtr;

							Texture* TempTexture{ Get<RefCountPtr<Texture>>(Binding.ResourcePtr).Get() };
							TempTexture->CreateUAV(descriptorHandle, Binding.Format, Binding.Dimension, Binding.Subresources);

							if (RHIResourceState::Unknown == TempTexture->m_StateExtension.PermanentState)
								this->m_BindingsThatNeedTransitions.push_back(static_cast<Uint16>(BindIndex));
							else
								ASSERT(false);//TODO :

							this->m_HasUavBindings = true;
							Found = true;
							break;
						}
						else if (D3D12_DESCRIPTOR_RANGE_TYPE_CBV == Range.RangeType && ConstantBuffer == BindingType) {
							pResource = Binding.ResourcePtr;

							Buffer* TempBuffer{ Get<RefCountPtr<Buffer>>(Binding.ResourcePtr).Get() };
							TempBuffer->CreateCBV(descriptorHandle, Binding.Range);
							ASSERT(false == TempBuffer->m_Desc.IsVolatile);

							if (RHIResourceState::Unknown == TempBuffer->m_StateExtension.PermanentState)
								this->m_BindingsThatNeedTransitions.push_back(static_cast<Uint16>(BindIndex));
							else
								ASSERT(false);//TODO :

							Found = true;
							break;
						}
						else if (D3D12_DESCRIPTOR_RANGE_TYPE_UAV == Range.RangeType && SamplerFeedbackTexture_UAV == BindingType) {
							if (!HoldsAlternative<Nullptr_T>(Binding.ResourcePtr))
								break;

							pResource = Binding.ResourcePtr;
							SamplerFeedbackTexture* TempSamplerFeedbackTexture{ Get<RefCountPtr<SamplerFeedbackTexture>>(Binding.ResourcePtr).Get() };
							TempSamplerFeedbackTexture->CreateUAV(descriptorHandle);

							this->m_HasUavBindings = true;
							Found = true;
							break;
						}
						//TODO add RayTracingAccelStruct
					}

					if (!HoldsAlternative<Nullptr_T>(pResource))
						this->m_Resources.push_back(pResource);

					if (!Found)
						switch (Range.RangeType) {
						case D3D12_DESCRIPTOR_RANGE_TYPE_SRV: Buffer::CreateNullSRV(descriptorHandle, RHIFormat::UNKNOWN, this->m_Context); break;
						case D3D12_DESCRIPTOR_RANGE_TYPE_UAV: Buffer::CreateNullUAV(descriptorHandle, RHIFormat::UNKNOWN, this->m_Context); break;
						case D3D12_DESCRIPTOR_RANGE_TYPE_CBV: this->m_Context.Device->CreateConstantBufferView(nullptr, descriptorHandle); break;
						case D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER:
						default: ASSERT(false); break;
						}
				}

			this->m_DeviceResourcesRef.ShaderResourceViewHeap.CopyToShaderVisibleHeap(descriptorTableBaseIndex, this->m_Layout->m_DescriptorTableSizeSRVetc);
		}
	}

	class DescriptorTable final :public RHIDescriptorTable<DescriptorTable> {
		friend class RHIResource<DescriptorTable>;
		friend class RHIDescriptorTable<DescriptorTable>;
	public:

		DescriptorTable(D3D12DeviceResources& resources) :
			RHIDescriptorTable<DescriptorTable>{},
			m_DeviceResourcesRef{ resources } 
		{}

		~DescriptorTable(void) { this->m_DeviceResourcesRef.SamplerHeap.ReleaseDescriptor(this->m_FirstDescriptor, this->m_Capacity); }

	public:
		using D3D12RootParameterIndex = D3D12RootSignature::D3D12RootParameterIndex;

	private:

	private:
		D3D12DeviceResources& m_DeviceResourcesRef;
		Uint32 m_Capacity{ 0 };
		D3D12DescriptorIndex m_FirstDescriptor{ 0 };

	private:
		RHIObject Imp_GetNativeObject(RHIObjectType)const noexcept { LOG_ERROR("Imp But Empty");  return RHIObject{}; }

		Uint32 Imp_Get_Capacity(void)const { return this->m_Capacity; }
		Uint32 Imp_Get_FirstDescriptorIndexInHeap(void)const { return this->m_FirstDescriptor; }
	};



}