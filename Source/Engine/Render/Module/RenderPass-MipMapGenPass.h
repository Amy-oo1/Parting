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


PARTING_SUBMODULE(Parting, SSAOPass)


#else
#pragma once

#include "Core/ModuleBuild.h"


#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"
//Global

#include<random>

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#include "Core/Algorithm/Module/Algorithm.h"
#include "Core/Container/Module/Container.h"
#include "Core/String/Module/String.h"
#include "Core/VectorMath/Module/VectorMath.h"
#include "Core/Logger/Module/Logger.h"

#include "RHI/Module/RHI.h"
#include "D3D12RHI/Module/D3D12RHI.h"

#include "Engine/Render/Module/RenderPass-Base.h"

#include "Engine/Render/Module/SceneTypes.h"
#include "Engine/Engine/Module/SceneGraph.h"

#include "Engine/Engine/Module/CommonRenderPasses.h"
#include "Engine/Engine/Module/FrameBufferFactory.h"
#include "Engine/Engine/Module/ShaderFactory.h"
#include "Engine/Render/Module/MaterialBindingCache.h"
#include "Engine/Render/Module/View.h"

#include "Shader/mipmapgen_cb.h"


#endif // PARTING_MODULE_BUILD


namespace Parting {


	namespace _NameSpace_MipMapGenPass {
		enum class Mode : Uint8 {
			Color,		// bilinear reduction of RGB channels
			Min,		// min() reduction of R channel
			Max,		// max() reduction of R channel
			MinMax		// min() and max() reductions of R channel into RG channels
		};
	}


	template<RHI::APITagConcept APITag>
	class MipMapGenPass final {
		using Imp_Device = typename RHI::RHITypeTraits<APITag>::Imp_Device;
		using Imp_CommandList = typename RHI::RHITypeTraits<APITag>::Imp_CommandList;
		using Imp_Texture = typename RHI::RHITypeTraits<APITag>::Imp_Texture;
		using Imp_Buffer = typename RHI::RHITypeTraits<APITag>::Imp_Buffer;
		using Imp_Shader = typename RHI::RHITypeTraits<APITag>::Imp_Shader;
		using Imp_Heap = typename RHI::RHITypeTraits<APITag>::Imp_Heap;
		using Imp_Sampler = typename RHI::RHITypeTraits<APITag>::Imp_Sampler;
		using Imp_ComputePipeline = typename RHI::RHITypeTraits<APITag>::Imp_ComputePipeline;
		using Imp_BindingLayout = typename RHI::RHITypeTraits<APITag>::Imp_BindingLayout;
		using Imp_BindingSet = typename RHI::RHITypeTraits<APITag>::Imp_BindingSet;

	public:
		using Mode = _NameSpace_MipMapGenPass::Mode;

		struct NullTextures final {
			Array<RHI::RefCountPtr<Imp_Texture>, Shader::NumLODs> Lods;

			static SharedPtr<NullTextures> Get(Imp_Device* device) {
				static Mutex _mutex;
				static WeakPtr<NullTextures> _nullTextures;

				SharedPtr<NullTextures> result{ _nullTextures.lock() };
				{
					LockGuard lock{ _mutex };

					if (nullptr == result) {
						result = ::MakeShared<NullTextures>();
						for (Uint32 Index = 0; Index < Shader::NumLODs; ++Index)
							result->Lods[Index] = NullTextures::CreateNullTexture(device);
						_nullTextures = result;
					}
				}

				return result;
			}

			static RHI::RefCountPtr<Imp_Texture> CreateNullTexture(Imp_Device* device) {
				RHI::RHITextureDesc desc{
					.Format{ RHI::RHIFormat::RGBA8_UNORM },
					.IsUAV{ true },
					.InitialState{ RHI::RHIResourceState::UnorderedAccess },
					.KeepInitialState{ true },
				};
				return device->CreateTexture(desc);
			}
		};

	public:
		MipMapGenPass(
			Imp_Device* device,
			SharedPtr<ShaderFactory<APITag>> shaderFactory,
			RHI::RefCountPtr<Imp_Texture> texture,
			Mode mode = Mode::Max
		);

		~MipMapGenPass(void) = default;

	public:


	private:
		RHI::RefCountPtr<Imp_Device> m_Device;
		RHI::RefCountPtr<Imp_Shader> m_Shader;
		RHI::RefCountPtr<Imp_Texture> m_Texture;
		RHI::RefCountPtr<Imp_Buffer> m_ConstantBuffer;
		RHI::RefCountPtr<Imp_BindingLayout> m_BindingLayout;
		Array<RHI::RefCountPtr<Imp_BindingSet>, Shader::MaxPasses> m_BindingSets;
		RHI::RefCountPtr<Imp_ComputePipeline> m_PSO;

		// Set of unique dummy textures - see details in class implementation
		SharedPtr<NullTextures> m_NullTextures;

		BindingCache<APITag> m_BindingCache{ this->m_Device };

	};



	template<RHI::APITagConcept APITag>
	inline MipMapGenPass<APITag>::MipMapGenPass(Imp_Device* device, SharedPtr<ShaderFactory<APITag>> shaderFactory, RHI::RefCountPtr<Imp_Texture> texture, Mode mode) :
		m_Device{ device },
		m_Texture{ ::MoveTemp(texture) } {

		ASSERT(nullptr != this->m_Texture);

		this->m_NullTextures = NullTextures::Get(this->m_Device);

		Uint32 nmipLevels{ this->m_Texture->Get_Desc().MipLevelCount };

		Vector<ShaderMacro> macros{
			ShaderMacro{.Name{ String{ "MODE" } }, .Definition{ IntegralToString(Tounderlying(mode)) } }
		};

		this->m_Shader = shaderFactory->CreateShader("Parting/Passes/mipmapgen_cs.hlsl", "main", &macros, RHI::RHIShaderType::Compute);

		// Constants
		this->m_ConstantBuffer = this->m_Device->CreateBuffer(RHI::RHIBufferDescBuilder{}
			.Set_ByteSize(sizeof(Shader::MipmmapGenConstants))
			.Set_MaxVersions(c_MaxRenderPassConstantBufferVersions)
			.Set_DebugName(_W("MipMapGenPass/Constants"))
			.Set_IsConstantBuffer(true)
			.Set_IsVolatile(true)
			.Build()
		);

		RHI::RHIBindingLayoutDescBuilder bindingLayoutDescBuilder{}; bindingLayoutDescBuilder
			.Set_Visibility(RHI::RHIShaderType::Compute)
			.AddBinding(RHI::RHIBindingLayoutItem::VolatileConstantBuffer(0))
			.AddBinding(RHI::RHIBindingLayoutItem::Texture_SRV(0));
		for (Uint32 MipLevel = 0; MipLevel < Shader::NumLODs; ++MipLevel)
			bindingLayoutDescBuilder.AddBinding(RHI::RHIBindingLayoutItem::Texture_UAV(MipLevel));
		this->m_BindingLayout = this->m_Device->CreateBindingLayout(bindingLayoutDescBuilder.Build());


		this->m_BindingSets.fill(nullptr);
		RHI::RHIBindingSetDescBuilder<APITag> bindingSetDescBuilder;

		for (Uint32 Index = 0; auto & bindingSet : this->m_BindingSets) {
			if (Index * Shader::NumLODs >= nmipLevels)// Create a unique binding set for each compute pass
				break;

			bindingSetDescBuilder.Reset()
				.AddBinding(RHI::RHIBindingSetItem<APITag>::ConstantBuffer(0, this->m_ConstantBuffer))
				.AddBinding(RHI::RHIBindingSetItem<APITag>::Texture_SRV(0, this->m_Texture, RHI::RHIFormat::UNKNOWN, RHI::RHITextureSubresourceSet{.BaseMipLevel{ Index * Shader::NumLODs } }));

			for (Uint32 MipLevel = 1; MipLevel <= Shader::NumLODs; ++MipLevel) {
				if (Index * Shader::NumLODs + MipLevel < nmipLevels)
					bindingSetDescBuilder.AddBinding(RHI::RHIBindingSetItem<APITag>::Texture_UAV(MipLevel - 1, this->m_Texture, RHI::RHIFormat::UNKNOWN, RHI::RHITextureSubresourceSet{.BaseMipLevel{ Index * Shader::NumLODs + MipLevel } }));
				else
					bindingSetDescBuilder.AddBinding(RHI::RHIBindingSetItem<APITag>::Texture_UAV(MipLevel - 1, this->m_NullTextures->Lods[MipLevel - 1]));
			}

			bindingSet = this->m_Device->CreateBindingSet(bindingSetDescBuilder.Build(), this->m_BindingLayout);

			++Index;
		}

		this->m_PSO = device->CreateComputePipeline(RHI::RHIComputePipelineDescBuilder<APITag>{}
		.Set_CS(this->m_Shader)
			.AddBindingLayout(this->m_BindingLayout)
			.Build()
			);
	}

}