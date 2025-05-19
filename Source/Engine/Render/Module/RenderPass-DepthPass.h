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

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#include "Core/Algorithm/Module/Algorithm.h"
#include "Core/Container/Module/Container.h"
#include "Core/VectorMath/Module/VectorMath.h"
#include "Core/Logger/Module/Logger.h"

#include "RHI/Module/RHI.h"
#include "D3D12RHI/Module/D3D12RHI.h"

#include "Engine/Render/Module/RenderPass-Base.h"

#include "Engine/Render/Module/SceneTypes.h"

#include "Engine/Engine/Module/CommonRenderPasses.h"
#include "Engine/Engine/Module/ShaderFactory.h"
#include "Engine/Render/Module/MaterialBindingCache.h"

#include "Shader/depth_cb.h"

#endif // PARTING_MODULE_BUILD


namespace Parting {

	template<RHI::APITagConcept APITag>
	class DepthPass : public IGeometryPass<APITag> {
		using Imp_Device = typename RHI::RHITypeTraits<APITag>::Imp_Device;
		using Imp_Shader = typename RHI::RHITypeTraits<APITag>::Imp_Shader;
		using Imp_InputLayout = typename RHI::RHITypeTraits<APITag>::Imp_InputLayout;
		using Imp_BindingLayout = typename RHI::RHITypeTraits<APITag>::Imp_BindingLayout;
		using Imp_BindingSet = typename RHI::RHITypeTraits<APITag>::Imp_BindingSet;
		using Imp_Texture = typename RHI::RHITypeTraits<APITag>::Imp_Texture;
		using Imp_FrameBuffer = typename RHI::RHITypeTraits<APITag>::Imp_FrameBuffer;
		using Imp_Buffer = typename RHI::RHITypeTraits<APITag>::Imp_Buffer;

	public:
		struct CreateParameters final {
			Int32 DepthBias{ 0 };
			float DepthBiasClamp{ 0.f };
			float SlopeScaledDepthBias{ 0.f };

			// Switches between loading vertex data through the Input Assembler (true) or buffer SRVs (false).
			// Using Buffer SRVs is often faster.
			bool UseInputAssembler{ false };
			bool TrackLiveness{ true };

			Uint32 numConstantBufferVersions{ 16 };


			SharedPtr<MaterialBindingCache<APITag>> MaterialBindings;
		};

	public:

		DepthPass(Imp_Device* device, SharedPtr<CommonRenderPasses<APITag>> commonRenderPasses) :
			m_Device{ device },
			m_CommonRenderPasses{ MoveTemp(commonRenderPasses) } {
		}

		void DeferInit(ShaderFactory<APITag>& shaderFactory, const CreateParameters& params) {
			this->m_UseInputAssembler = params.UseInputAssembler;
			this->m_TrackLiveness = params.TrackLiveness;

			this->m_DepthBias = params.DepthBias;
			this->m_DepthBiasClamp = params.DepthBiasClamp;
			this->m_SlopeScaledDepthBias = params.SlopeScaledDepthBias;

			this->m_VertexShader = CreateVertexShader(shaderFactory, params);
			this->m_PixelShader = CreatePixelShadewr(shaderFactory, params);
			this->m_InputLayout = CreateInputLayout(this->m_VertexShader.Get(), params);
			this->m_InputBindingLayout = CreateInputBindingLayout();

			if (nullptr != params.MaterialBindings)
				this->m_MaterialBindings = params.MaterialBindings;
			else
				this->m_MaterialBindings = this->CreateMaterialBindingCache(*this->m_CommonRenderPasses);

			this->m_DepthCB = this->m_Device->CreateBuffer(RHI::RHIBufferDescBuilder::CreateVolatileConstantBufferDesc(sizeof(DepthPassConstants), params.numConstantBufferVersions));
			Tie(this->m_ViewBindingLayout, this->m_ViewBindingSet) = CreateViewBindings(params);

		}

	private:
		auto CreateVertexShader(ShaderFactory<APITag>& shaderFactory, const CreateParameters& params) -> RHI::RefCountPtr<Imp_Shader>;
		auto CreatePixelShadewr(ShaderFactory<APITag>& shaderFactory, const CreateParameters& params) -> RHI::RefCountPtr<Imp_Shader>;
		auto CreateInputLayout(Imp_Shader* vertexShader, const CreateParameters& params) -> RHI::RefCountPtr<Imp_InputLayout>;
		auto CreateInputBindingLayout(void) -> RHI::RefCountPtr<Imp_BindingLayout>;
		auto CreateMaterialBindingCache(CommonRenderPasses<APITag>& commonPasses) -> SharedPtr<MaterialBindingCache<APITag>>;
		auto CreateViewBindings(const CreateParameters& params) -> Tuple<RHI::RefCountPtr<Imp_BindingLayout>, RHI::RefCountPtr<Imp_BindingSet>>;

	private:
		RHI::RefCountPtr<Imp_Device> m_Device;
		SharedPtr<CommonRenderPasses<APITag>> m_CommonRenderPasses;

		RHI::RefCountPtr<Imp_Shader> m_VertexShader;
		RHI::RefCountPtr<Imp_Shader> m_PixelShader;

		RHI::RefCountPtr<Imp_InputLayout> m_InputLayout;//TODO : add Optional maybe better?
		RHI::RefCountPtr<Imp_BindingLayout> m_InputBindingLayout;

		SharedPtr<MaterialBindingCache<APITag>> m_MaterialBindings;

		RHI::RefCountPtr<Imp_Buffer> m_DepthCB;

		RHI::RefCountPtr<Imp_BindingLayout> m_ViewBindingLayout;
		RHI::RefCountPtr<Imp_BindingSet> m_ViewBindingSet;

		
		Int32 m_DepthBias{ 0 };
		float m_DepthBiasClamp{ 0.f };
		float m_SlopeScaledDepthBias{ 0.f };
		bool m_UseInputAssembler{ false };
		bool m_TrackLiveness{ true };

	};


	template<RHI::APITagConcept APITag>
	inline auto DepthPass<APITag>::CreateVertexShader(ShaderFactory<APITag>& shaderFactory, const CreateParameters& params) -> RHI::RefCountPtr<Imp_Shader> {
		return shaderFactory.CreateShader(
			"Parting/Passes/depth_vs.hlsl",
			params.UseInputAssembler ? "input_assembler" : "buffer_loads",
			nullptr,
			RHI::RHIShaderType::Vertex
		);
	}

	template<RHI::APITagConcept APITag>
	inline auto DepthPass<APITag>::CreatePixelShadewr(ShaderFactory<APITag>& shaderFactory, const CreateParameters& params) -> RHI::RefCountPtr<Imp_Shader> {
		return shaderFactory.CreateShader("Parting/Passes/depth_ps.hlsl", "main", nullptr, RHI::RHIShaderType::Pixel);
	}

	template<RHI::APITagConcept APITag>
	inline auto DepthPass<APITag>::CreateInputLayout(Imp_Shader* vertexShader, const CreateParameters& params) -> RHI::RefCountPtr<Imp_InputLayout> {
		if (!params.UseInputAssembler)
			return nullptr;

		RHI::RHIVertexAttributeDescBuilder builder;

		Array<RHI::RHIVertexAttributeDesc, 3> aInputDescs{
			BuildVertexAttributeDesc(builder.Reset(), RHI::RHIVertexAttribute::Position, "POSITION", 0),
			BuildVertexAttributeDesc(builder.Reset(), RHI::RHIVertexAttribute::TexCoord1, "TEXCOORD", 0),
			BuildVertexAttributeDesc(builder.Reset(), RHI::RHIVertexAttribute::Transform, "TRANSFORM", 2)
		};

		return this->m_Device->CreateInputLayout(aInputDescs.data(), static_cast<Uint32>(aInputDescs.size()), vertexShader);
	}

	template<RHI::APITagConcept APITag>
	inline auto DepthPass<APITag>::CreateInputBindingLayout(void) -> RHI::RefCountPtr<Imp_BindingLayout> {
		if (this->m_UseInputAssembler)
			return nullptr;

		return this->m_Device->CreateBindingLayout(RHI::RHIBindingLayoutDescBuilder{}
			.Set_Visibility(RHI::RHIShaderType::Vertex)
			.Set_RegisterSpace(DepthSapceInput)
			.Set_RegisterSpaceIsDescriptorSet(true)//not support d3d11.... i do not lean d3d11 forever
			.AddBinding(RHI::RHIBindingLayoutItem::StructuredBuffer_SRV(DepthBindingInstanceBuffer))
			.AddBinding(RHI::RHIBindingLayoutItem::RawBuffer_SRV(DepthBindingVertexBuffer))
			.AddBinding(RHI::RHIBindingLayoutItem::PushConstants(DepthBindingPushConstants, sizeof(DepthPushConstants)))
			.Build()
		);
	}

	template<RHI::APITagConcept APITag>
	inline auto DepthPass<APITag>::CreateMaterialBindingCache(CommonRenderPasses<APITag>& commonPasses) -> SharedPtr<MaterialBindingCache<APITag>> {
		Vector<MaterialResourceBinding> materialBindings{
			MaterialResourceBinding{.Resource{ MaterialResource::DiffuseTexture }, .Slot{ DepthBindingMaterialDiffuseTexture } },
			MaterialResourceBinding{.Resource{ MaterialResource::OpacityTexture }, .Slot{ DepthBindingMaterialOpacityTexture } },
			MaterialResourceBinding{.Resource{ MaterialResource::ConstantBuffer }, .Slot{ DepthBindingMaterialConstants } }
		};

		return MakeShared<MaterialBindingCache<APITag>>(
			this->m_Device,
			RHI::RHIShaderType::Pixel,
			DepthSapceInput,
			true,//not use d3d11 opengl...
			materialBindings,
			commonPasses.m_AnisotropicWrapSampler.Get(),
			commonPasses.m_GrayTexture.Get()
		);
	}

	template<RHI::APITagConcept APITag>
	inline auto DepthPass<APITag>::CreateViewBindings(const CreateParameters& params) -> Tuple<RHI::RefCountPtr<Imp_BindingLayout>, RHI::RefCountPtr<Imp_BindingSet>> {
		auto layout{ this->m_Device->CreateBindingLayout(RHI::RHIBindingLayoutDescBuilder{}
			.Set_Visibility(RHI::RHIShaderType::Vertex | RHI::RHIShaderType::Pixel)
			.Set_RegisterSpace(DepthSpaceView)
			.Set_RegisterSpaceIsDescriptorSet(true)//not support d3d11.... i do not lean d3d11 forever
			.AddBinding(RHI::RHIBindingLayoutItem::VolatileConstantBuffer(DepthBindingViewConstants))
			.AddBinding(RHI::RHIBindingLayoutItem::Sampler(DepthBindingMaterialSampler))
			.Build()
		) };

		auto bindingset{ this->m_Device->CreateBindingSet(RHI::RHIBindingSetDescBuilder<APITag>{}
			.Set_TrackLiveness(params.TrackLiveness)
			.AddBinding(RHI::RHIBindingSetItem<APITag>::ConstantBuffer(DepthBindingViewConstants,this->m_DepthCB))
			.AddBinding(RHI::RHIBindingSetItem<APITag>::Sampler(DepthBindingMaterialSampler, this->m_CommonRenderPasses->m_AnisotropicWrapSampler))
			.Build(),
			layout.Get()
		) };

		return MakeTuple<RHI::RefCountPtr<Imp_BindingLayout>, RHI::RefCountPtr<Imp_BindingSet>>(MoveTemp(layout), MoveTemp(bindingset));
	}

}