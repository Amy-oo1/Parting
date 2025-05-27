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

#include "Shader/forward_cb.h"



#endif // PARTING_MODULE_BUILD


namespace Parting {

	namespace _NameSpace_ForwardShadingPass {

		union PipelineKey {
			struct {
				MaterialDomain Domain : 3;
				RHI::RHIRasterCullMode CullMode : 2;
				bool FrontCounterClockwise : 1;
				bool ReverseDepth : 1;
			} Bits;
			Uint32 Value;

			static constexpr Uint32 COUNT{ 1 << 7 };
		};

	}

	template<RHI::APITagConcept APITag>
	class ForwardShadingPass : public IGeometryPass<APITag> {
		using Imp_Device = typename RHI::RHITypeTraits<APITag>::Imp_Device;
		using Imp_CommandList = typename RHI::RHITypeTraits<APITag>::Imp_CommandList;
		using Imp_Shader = typename RHI::RHITypeTraits<APITag>::Imp_Shader;
		using Imp_InputLayout = typename RHI::RHITypeTraits<APITag>::Imp_InputLayout;
		using Imp_Texture = typename RHI::RHITypeTraits<APITag>::Imp_Texture;
		using Imp_Sampler = typename RHI::RHITypeTraits<APITag>::Imp_Sampler;
		using Imp_Buffer = typename RHI::RHITypeTraits<APITag>::Imp_Buffer;
		using Imp_BindingLayout = typename RHI::RHITypeTraits<APITag>::Imp_BindingLayout;
		using Imp_BindingSet = typename RHI::RHITypeTraits<APITag>::Imp_BindingSet;

		using Imp_GraphicsPipeline = typename RHI::RHITypeTraits<APITag>::Imp_GraphicsPipeline;
		using Imp_FrameBuffer = typename RHI::RHITypeTraits<APITag>::Imp_FrameBuffer;
	public:
		using PipelineKey = _NameSpace_ForwardShadingPass::PipelineKey;

		class Context final : public GeometryPassContext {
		public:
			Context(void) :GeometryPassContext{} { this->KeyTemplate.Value = 0; }
			~Context(void) = default;

		public:
			RHI::RefCountPtr<Imp_BindingSet> ShadingBindingSet;
			RHI::RefCountPtr<Imp_BindingSet> InputBindingSet;
			PipelineKey KeyTemplate;

			Uint32 PositionOffset{ 0 };
			Uint32 TexCoordOffset{ 0 };
			Uint32 NormalOffset = 0;
			Uint32 TangentOffset = 0;


		};

		struct CreateParameters final {
			SharedPtr<MaterialBindingCache<APITag>> MaterialBindings;
			bool SinglePassCubemap{ false };
			bool TrackLiveness{ true };

			// Switches between loading vertex data through the Input Assembler (true) or buffer SRVs (false).
			// Using Buffer SRVs is often faster.
			bool UseInputAssembler{ false };

			Uint32 NumConstantBufferVersions{ 16 };
		};

	public:
		ForwardShadingPass(Imp_Device* device, SharedPtr<CommonRenderPasses<APITag>> commonPasses);
		~ForwardShadingPass(void) = default;

	public:
		void Init(ShaderFactory<APITag>& shaderFactory, const CreateParameters& params);

		void ResetBindingCache(void);

		void PrepareLights(Context& context, Imp_CommandList* commandList, const Vector<SharedPtr<Light<APITag>>>& lights, Math::VecF3 ambientColorTop, Math::VecF3 ambientColorBottom, const Vector<SharedPtr<LightProbe<APITag>>>& lightProbes);


	public:
		auto CreateVertexShader(ShaderFactory<APITag>& shaderFactory, const CreateParameters& params) -> RHI::RefCountPtr<Imp_Shader>;
		auto CreateGeometryShader(ShaderFactory<APITag>& shaderFactory, const CreateParameters& params) -> RHI::RefCountPtr<Imp_Shader>;
		auto CreatePixelShader(ShaderFactory<APITag>& shaderFactory, const CreateParameters& params, bool transmissiveMaterial) -> RHI::RefCountPtr<Imp_Shader>;
		auto CreateInputLayout(Imp_Shader* vertexShader, const CreateParameters& params) -> RHI::RefCountPtr<Imp_InputLayout>;
		auto CreateViewBindingLayout(void) -> RHI::RefCountPtr<Imp_BindingLayout>;
		auto CreateViewBindingSet(void) -> RHI::RefCountPtr<Imp_BindingSet>;
		auto CreateShadingBindingLayout(void) -> RHI::RefCountPtr<Imp_BindingLayout>;
		auto CreateShadingBindingSet(Imp_Texture* shadowMapTexture, Imp_Texture* diffuse, Imp_Texture* specular, Imp_Texture* environmentBrdf) -> RHI::RefCountPtr<Imp_BindingSet>;
		auto CreateInputBindingLayout(void) -> RHI::RefCountPtr<Imp_BindingLayout>;
		auto CreateInputBindingSet(const BufferGroup<APITag>* bufferGroup) -> RHI::RefCountPtr<Imp_BindingSet>;
		auto CreateMaterialBindingCache(CommonRenderPasses<APITag>& commonPasses) -> SharedPtr<MaterialBindingCache<APITag>>;
		auto CreateGraphicsPipeline(PipelineKey key, Imp_FrameBuffer* framebuffer) -> RHI::RefCountPtr<Imp_GraphicsPipeline>;
		auto GetOrCreateInputBindingSet(const BufferGroup<APITag>* bufferGroup) -> RHI::RefCountPtr<Imp_BindingSet>;



	private:
		RHI::RefCountPtr<Imp_Device> m_Device;
		RHI::RefCountPtr<Imp_InputLayout> m_InputLayout;
		RHI::RefCountPtr<Imp_Shader> m_VertexShader;
		RHI::RefCountPtr<Imp_Shader> m_PixelShader;
		RHI::RefCountPtr<Imp_Shader> m_PixelShaderTransmissive;
		RHI::RefCountPtr<Imp_Shader> m_GeometryShader;
		RHI::RefCountPtr<Imp_Sampler> m_ShadowSampler;
		RHI::RefCountPtr<Imp_BindingLayout> m_ViewBindingLayout;
		RHI::RefCountPtr<Imp_BindingSet> m_ViewBindingSet;
		RHI::RefCountPtr<Imp_BindingLayout> m_ShadingBindingLayout;
		RHI::RefCountPtr<Imp_BindingLayout> m_InputBindingLayout;
		RHI::RefCountPtr<Imp_Buffer> m_ForwardViewCB;
		RHI::RefCountPtr<Imp_Buffer> m_ForwardLightCB;
		Array<RHI::RefCountPtr<Imp_GraphicsPipeline>, PipelineKey::COUNT> m_Pipelines;

		ViewType m_SupportedViewTypes{ ViewType::PLANAR };

		bool m_TrackLiveness{ true };
		bool m_UseInputAssembler{ false };

		Mutex m_Mutex;

		UnorderedMap<Pair<Imp_Texture*, Imp_Texture*>, RHI::RefCountPtr<Imp_BindingSet>> m_ShadingBindingSets;
		UnorderedMap<const BufferGroup<APITag>*, RHI::RefCountPtr<Imp_BindingSet>> m_InputBindingSets;

		SharedPtr<CommonRenderPasses<APITag>> m_CommonPasses;
		SharedPtr<MaterialBindingCache<APITag>> m_MaterialBindings;

	public:
		STDNODISCARD ViewType Get_SupportedViewTypes(void) const override { return this->m_SupportedViewTypes; }

		void SetupView(GeometryPassContext& context, Imp_CommandList* commandList, const IView* view, const IView* viewPrev) override;

		bool SetupMaterial(GeometryPassContext& context, const Material<APITag>* material, RHI::RHIRasterCullMode cullMode, RHI::RHIGraphicsState<APITag>& state) override;

		void SetupInputBuffers(GeometryPassContext& context, const BufferGroup<APITag>* buffers, RHI::RHIGraphicsState<APITag>& state) override;

		void SetPushConstants(GeometryPassContext& context, Imp_CommandList* commandList, RHI::RHIGraphicsState<APITag>& state, RHI::RHIDrawArguments& args) override;
	};

	template<RHI::APITagConcept APITag>
	inline ForwardShadingPass<APITag>::ForwardShadingPass(Imp_Device* device, SharedPtr<CommonRenderPasses<APITag>> commonPasses) :
		IGeometryPass<APITag>{},
		m_Device{ device },
		m_CommonPasses{ commonPasses }{
	}

	template<RHI::APITagConcept APITag>
	inline void ForwardShadingPass<APITag>::Init(ShaderFactory<APITag>& shaderFactory, const CreateParameters& params) {
		this->m_UseInputAssembler = params.UseInputAssembler;

		this->m_SupportedViewTypes = ViewType::PLANAR;
		if (params.SinglePassCubemap)
			this->m_SupportedViewTypes = ViewType::CUBEMAP;//TODO

		this->m_VertexShader = this->CreateVertexShader(shaderFactory, params);
		this->m_InputLayout = this->CreateInputLayout(this->m_VertexShader.Get(), params);
		this->m_GeometryShader = this->CreateGeometryShader(shaderFactory, params);
		this->m_PixelShader = this->CreatePixelShader(shaderFactory, params, false);
		this->m_PixelShaderTransmissive = this->CreatePixelShader(shaderFactory, params, true);

		if (nullptr != params.MaterialBindings)
			this->m_MaterialBindings = params.MaterialBindings;
		else
			this->m_MaterialBindings = this->CreateMaterialBindingCache(*this->m_CommonPasses);

		this->m_ShadowSampler = this->m_Device->CreateSampler(RHI::RHISamplerDescBuilder{}
			.Set_AddressModeUVW(RHI::RHISamplerAddressMode::Border)
			.Set_BorderColor(Color{ 1.f })
			.Build()
		);

		RHI::RHIBufferDescBuilder VolatileConstantBufferDescBuilder{}; VolatileConstantBufferDescBuilder
			.Set_IsVolatile(true)
			.Set_IsConstantBuffer(true)
			.Set_MaxVersions(params.NumConstantBufferVersions);

		this->m_ForwardViewCB = this->m_Device->CreateBuffer(VolatileConstantBufferDescBuilder
			.Set_ByteSize(sizeof(ForwardShadingViewConstants))
			.Set_DebugName(_W("ForwardShadingViewConstants"))
			.Build()
		);

		this->m_ForwardLightCB = this->m_Device->CreateBuffer(VolatileConstantBufferDescBuilder
			.Set_ByteSize(sizeof(ForwardShadingLightConstants))
			.Set_DebugName(_W("ForwardShadingLightConstants"))
			.Build()
		);

		this->m_ViewBindingLayout = this->CreateViewBindingLayout();
		this->m_ViewBindingSet = this->CreateViewBindingSet();
		this->m_ShadingBindingLayout = this->CreateShadingBindingLayout();
		this->m_InputBindingLayout = this->CreateInputBindingLayout();
	}

	template<RHI::APITagConcept APITag>
	inline void ForwardShadingPass<APITag>::ResetBindingCache(void) {
		this->m_MaterialBindings->Clear();
		this->m_ShadingBindingSets.clear();
		this->m_InputBindingSets.clear();
	}

	template<RHI::APITagConcept APITag>
	inline void ForwardShadingPass<APITag>::PrepareLights(Context& context, Imp_CommandList* commandList, const Vector<SharedPtr<Light<APITag>>>& lights, Math::VecF3 ambientColorTop, Math::VecF3 ambientColorBottom, const Vector<SharedPtr<LightProbe<APITag>>>& lightProbes) {
		Imp_Texture* shadowMapTexture{ nullptr };
		Math::VecU2 shadowMapTextureSize{ Math::VecU2::Zero() };
		for (const auto& light : lights)
			if (nullptr != light->ShadowMap) {
				shadowMapTexture = light->ShadowMap->Get_Texture();
				shadowMapTextureSize = light->ShadowMap->Get_TextureSize();
				break;
			}

		Imp_Texture* lightProbeDiffuse{ nullptr };
		Imp_Texture* lightProbeSpecular{ nullptr };
		Imp_Texture* lightProbeEnvironmentBrdf{ nullptr };

		for (const auto& probe : lightProbes) {
			if (!probe->Enabled)
				continue;

			if (lightProbeDiffuse == nullptr || lightProbeSpecular == nullptr || lightProbeEnvironmentBrdf == nullptr) {
				lightProbeDiffuse = probe->DiffuseMap;
				lightProbeSpecular = probe->SpecularMap;
				lightProbeEnvironmentBrdf = probe->EnvironmentBRDF;
			}
			else if (lightProbeDiffuse != probe->DiffuseMap || lightProbeSpecular != probe->SpecularMap || lightProbeEnvironmentBrdf != probe->EnvironmentBRDF) {
				LOG_ERROR("All lights probe submitted to ForwardShadingPass::PrepareLights(...) must use the same set of textures");
				return;
			}
		}

		{
			LockGuard lockGuard{ this->m_Mutex };

			auto& shadingBindings = this->m_ShadingBindingSets[::MakePair(shadowMapTexture, lightProbeDiffuse)];

			if (nullptr == shadingBindings)
				shadingBindings = this->CreateShadingBindingSet(shadowMapTexture, lightProbeDiffuse, lightProbeSpecular, lightProbeEnvironmentBrdf);

			context.ShadingBindingSet = shadingBindings;
		}


		ForwardShadingLightConstants constants{};

		constants.ShadowMapTextureSize = Math::VecF2{ shadowMapTextureSize };
		constants.ShadowMapTextureSizeInv = 1.f / constants.ShadowMapTextureSize;

		Uint32 numShadows{ 0 };//TODO :

		for (Uint32 nLight = 0; nLight < Math::Min<Uint32>(static_cast<Uint32>(lights.size()), FORWARD_MAX_LIGHTS); ++nLight) {
			const auto& light{ lights[nLight] };

			LightConstants& lightConstants{ constants.Lights[constants.NumLights] };
			light->FillLightConstants(lightConstants);

			if (nullptr != light->ShadowMap) {
				for (Uint32 cascade = 0; cascade < light->ShadowMap->Get_NumberOfCascades(); ++cascade)
					if (numShadows < FORWARD_MAX_SHADOWS) {
						light->ShadowMap->Get_Cascade(cascade)->FillShadowConstants(constants.Shadows[numShadows]);
						lightConstants.ShadowCascades[cascade] = static_cast<Int32>(numShadows);//TODO :
						++numShadows;
					}

				for (Uint32 perObjectShadow = 0; perObjectShadow < light->ShadowMap->Get_NumberOfPerObjectShadows(); ++perObjectShadow)
					if (numShadows < FORWARD_MAX_SHADOWS){
						light->ShadowMap->Get_PerObjectShadow(perObjectShadow)->FillShadowConstants(constants.Shadows[numShadows]);
						lightConstants.PerObjectShadows[perObjectShadow] = static_cast<Int32>(numShadows);//TODO :
						++numShadows;
					}
			}

			++constants.NumLights;
		}

		constants.AmbientColorTop = Math::VecF4{ ambientColorTop, 0.f };
		constants.AmbientColorBottom = Math::VecF4{ ambientColorBottom, 0.f };

		for (const auto& probe : lightProbes){
			if (!probe->Is_Active())
				continue;

			LightProbeConstants& lightProbeConstants = constants.LightProbes[constants.NumLightProbes];
			probe->FillLightProbeConstants(lightProbeConstants);

			++constants.NumLightProbes;

			if (constants.NumLightProbes >= FORWARD_MAX_LIGHT_PROBES)
				break;
		}

		commandList->WriteBuffer(this->m_ForwardLightCB, &constants, sizeof(constants));
	}

	template<RHI::APITagConcept APITag>
	inline auto ForwardShadingPass<APITag>::CreateVertexShader(ShaderFactory<APITag>& shaderFactory, const CreateParameters& params) -> RHI::RefCountPtr<Imp_Shader> {
		constexpr const char* sourceFileName{ "Parting/Passes/forward_vs.hlsl" };

		if (params.UseInputAssembler)
			return shaderFactory.CreateShader(sourceFileName, "input_assembler", nullptr, RHI::RHIShaderType::Vertex);
		else
			return shaderFactory.CreateShader(sourceFileName, "buffer_loads", nullptr, RHI::RHIShaderType::Vertex);
	}

	template<RHI::APITagConcept APITag>
	inline auto ForwardShadingPass<APITag>::CreateGeometryShader(ShaderFactory<APITag>& shaderFactory, const CreateParameters& params) -> RHI::RefCountPtr<Imp_Shader> {
		if (params.SinglePassCubemap) {

			ASSERT(false);
			return shaderFactory.CreateShader("Parting/Passes/cubemap_gs.hlsl", "main", nullptr, RHI::RHIShaderType::Geometry);
		}

		return nullptr;
	}

	template<RHI::APITagConcept APITag>
	inline auto ForwardShadingPass<APITag>::CreatePixelShader(ShaderFactory<APITag>& shaderFactory, const CreateParameters& params, bool transmissiveMaterial) -> RHI::RefCountPtr<Imp_Shader> {
		Vector<ShaderMacro> Macros{
			ShaderMacro{.Name{ String{ "TRANSMISSIVE_MATERIAL"} }, .Definition{ String{ transmissiveMaterial ? "1" : "0" } }},
		};

		return shaderFactory.CreateShader("Parting/Passes/forward_ps.hlsl", "main", &Macros, RHI::RHIShaderType::Pixel);
	}

	template<RHI::APITagConcept APITag>
	inline auto ForwardShadingPass<APITag>::CreateInputLayout(Imp_Shader* vertexShader, const CreateParameters& params) -> RHI::RefCountPtr<Imp_InputLayout> {
		if (params.UseInputAssembler) {

			RHI::RHIVertexAttributeDescBuilder vertexAttribDescBuilder{};

			Array<RHI::RHIVertexAttributeDesc, 6> inputDescs{//TODO move Rest
				BuildVertexAttributeDesc(vertexAttribDescBuilder.Reset(),RHI::RHIVertexAttribute::Position, "POS", 0),
				BuildVertexAttributeDesc(vertexAttribDescBuilder.Reset(),RHI::RHIVertexAttribute::PrevPosition, "PREV_POS", 1),
				BuildVertexAttributeDesc(vertexAttribDescBuilder.Reset(),RHI::RHIVertexAttribute::TexCoord1, "TEXCOORD", 2),
				BuildVertexAttributeDesc(vertexAttribDescBuilder.Reset(),RHI::RHIVertexAttribute::Normal, "NORMAL", 3),
				BuildVertexAttributeDesc(vertexAttribDescBuilder.Reset(),RHI::RHIVertexAttribute::Tangent, "TANGENT", 4),
				BuildVertexAttributeDesc(vertexAttribDescBuilder.Reset(),RHI::RHIVertexAttribute::Transform, "TRANSFORM", 5),
			};

			return this->m_Device->CreateInputLayout(inputDescs.data(), static_cast<Uint32>(inputDescs.size()), vertexShader);
		}

		return nullptr;
	}

	template<RHI::APITagConcept APITag>
	inline auto ForwardShadingPass<APITag>::CreateViewBindingLayout(void) -> RHI::RefCountPtr<Imp_BindingLayout> {
		return this->m_Device->CreateBindingLayout(RHI::RHIBindingLayoutDescBuilder{}
			.Set_Visibility(RHI::RHIShaderType::Vertex | RHI::RHIShaderType::Pixel)
			.Set_RegisterSpace(FORWARD_SPACE_VIEW)
			.Set_RegisterSpaceIsDescriptorSet(true)
			.AddBinding(RHI::RHIBindingLayoutItem::VolatileConstantBuffer(FORWARD_BINDING_VIEW_CONSTANTS))
			.Build()
		);
	}

	template<RHI::APITagConcept APITag>
	inline auto ForwardShadingPass<APITag>::CreateViewBindingSet(void) -> RHI::RefCountPtr<Imp_BindingSet> {
		return this->m_Device->CreateBindingSet(RHI::RHIBindingSetDescBuilder<APITag>{}
		.Set_TrackLiveness(this->m_TrackLiveness)
			.AddBinding(RHI::RHIBindingSetItem<APITag>::ConstantBuffer(FORWARD_BINDING_VIEW_CONSTANTS, this->m_ForwardViewCB))
			.Build(),
			this->m_ViewBindingLayout
			);
	}

	template<RHI::APITagConcept APITag>
	inline auto ForwardShadingPass<APITag>::CreateShadingBindingLayout(void) -> RHI::RefCountPtr<Imp_BindingLayout> {
		return this->m_Device->CreateBindingLayout(RHI::RHIBindingLayoutDescBuilder{}
			.Set_Visibility(RHI::RHIShaderType::Pixel)
			.Set_RegisterSpace(FORWARD_SPACE_SHADING)
			.Set_RegisterSpaceIsDescriptorSet(true)
			.AddBinding(RHI::RHIBindingLayoutItem::VolatileConstantBuffer(FORWARD_BINDING_LIGHT_CONSTANTS))
			.AddBinding(RHI::RHIBindingLayoutItem::Texture_SRV(FORWARD_BINDING_SHADOW_MAP_TEXTURE))
			.AddBinding(RHI::RHIBindingLayoutItem::Texture_SRV(FORWARD_BINDING_DIFFUSE_LIGHT_PROBE_TEXTURE))
			.AddBinding(RHI::RHIBindingLayoutItem::Texture_SRV(FORWARD_BINDING_SPECULAR_LIGHT_PROBE_TEXTURE))
			.AddBinding(RHI::RHIBindingLayoutItem::Texture_SRV(FORWARD_BINDING_ENVIRONMENT_BRDF_TEXTURE))
			.AddBinding(RHI::RHIBindingLayoutItem::Sampler(FORWARD_BINDING_MATERIAL_SAMPLER))
			.AddBinding(RHI::RHIBindingLayoutItem::Sampler(FORWARD_BINDING_SHADOW_MAP_SAMPLER))
			.AddBinding(RHI::RHIBindingLayoutItem::Sampler(FORWARD_BINDING_LIGHT_PROBE_SAMPLER))
			.AddBinding(RHI::RHIBindingLayoutItem::Sampler(FORWARD_BINDING_ENVIRONMENT_BRDF_SAMPLER))
			.Build()
		);
	}

	template<RHI::APITagConcept APITag>
	inline auto ForwardShadingPass<APITag>::CreateShadingBindingSet(Imp_Texture* shadowMapTexture, Imp_Texture* diffuse, Imp_Texture* specular, Imp_Texture* environmentBrdf) -> RHI::RefCountPtr<Imp_BindingSet> {
		return this->m_Device->CreateBindingSet(RHI::RHIBindingSetDescBuilder<APITag>{}
		.Set_TrackLiveness(this->m_TrackLiveness)
			.AddBinding(RHI::RHIBindingSetItem<APITag>::ConstantBuffer(FORWARD_BINDING_LIGHT_CONSTANTS, this->m_ForwardLightCB))
			.AddBinding(RHI::RHIBindingSetItem<APITag>::Texture_SRV(FORWARD_BINDING_SHADOW_MAP_TEXTURE, nullptr != shadowMapTexture ? shadowMapTexture : this->m_CommonPasses->m_BlackTexture2DArray.Get()))
			.AddBinding(RHI::RHIBindingSetItem<APITag>::Texture_SRV(FORWARD_BINDING_DIFFUSE_LIGHT_PROBE_TEXTURE, nullptr != diffuse ? diffuse : this->m_CommonPasses->m_BlackCubeMapArray.Get()))
			.AddBinding(RHI::RHIBindingSetItem<APITag>::Texture_SRV(FORWARD_BINDING_SPECULAR_LIGHT_PROBE_TEXTURE, nullptr != specular ? specular : this->m_CommonPasses->m_BlackCubeMapArray.Get()))
			.AddBinding(RHI::RHIBindingSetItem<APITag>::Texture_SRV(FORWARD_BINDING_ENVIRONMENT_BRDF_TEXTURE, nullptr != environmentBrdf ? environmentBrdf : this->m_CommonPasses->m_BlackTexture.Get()))
			.AddBinding(RHI::RHIBindingSetItem<APITag>::Sampler(FORWARD_BINDING_MATERIAL_SAMPLER, this->m_CommonPasses->m_AnisotropicWrapSampler.Get()))
			.AddBinding(RHI::RHIBindingSetItem<APITag>::Sampler(FORWARD_BINDING_SHADOW_MAP_SAMPLER, this->m_ShadowSampler.Get()))
			.AddBinding(RHI::RHIBindingSetItem<APITag>::Sampler(FORWARD_BINDING_LIGHT_PROBE_SAMPLER, this->m_CommonPasses->m_LinearWrapSampler.Get()))
			.AddBinding(RHI::RHIBindingSetItem<APITag>::Sampler(FORWARD_BINDING_ENVIRONMENT_BRDF_SAMPLER, this->m_CommonPasses->m_LinearClampSampler.Get()))
			.Build(),
			this->m_ShadingBindingLayout
			);
	}

	template<RHI::APITagConcept APITag>
	inline auto ForwardShadingPass<APITag>::CreateInputBindingLayout(void) -> RHI::RefCountPtr<Imp_BindingLayout> {
		if (this->m_UseInputAssembler)
			return nullptr;

		return this->m_Device->CreateBindingLayout(RHI::RHIBindingLayoutDescBuilder{}
			.Set_Visibility(RHI::RHIShaderType::Vertex)
			.Set_RegisterSpace(FORWARD_SPACE_INPUT)
			.Set_RegisterSpaceIsDescriptorSet(true)
			.AddBinding(RHI::RHIBindingLayoutItem::StructuredBuffer_SRV(FORWARD_BINDING_INSTANCE_BUFFER))
			.AddBinding(RHI::RHIBindingLayoutItem::RawBuffer_SRV(FORWARD_BINDING_VERTEX_BUFFER))
			.AddBinding(RHI::RHIBindingLayoutItem::PushConstants(FORWARD_BINDING_PUSH_CONSTANTS, sizeof(ForwardPushConstants)))
			.Build()
		);
	}

	template<RHI::APITagConcept APITag>
	inline auto ForwardShadingPass<APITag>::CreateInputBindingSet(const BufferGroup<APITag>* bufferGroup) -> RHI::RefCountPtr<Imp_BindingSet> {
		return this->m_Device->CreateBindingSet(RHI::RHIBindingSetDescBuilder<APITag>{}
		.AddBinding(RHI::RHIBindingSetItem<APITag>::StructuredBuffer_SRV(FORWARD_BINDING_INSTANCE_BUFFER, bufferGroup->InstanceBuffer))
			.AddBinding(RHI::RHIBindingSetItem<APITag>::RawBuffer_SRV(FORWARD_BINDING_VERTEX_BUFFER, bufferGroup->VertexBuffer))
			.AddBinding(RHI::RHIBindingSetItem<APITag>::PushConstants(FORWARD_BINDING_PUSH_CONSTANTS, sizeof(ForwardPushConstants)))
			.Build(),
			this->m_InputBindingLayout
			);
	}

	template<RHI::APITagConcept APITag>
	inline auto ForwardShadingPass<APITag>::CreateMaterialBindingCache(CommonRenderPasses<APITag>& commonPasses) -> SharedPtr<MaterialBindingCache<APITag>> {
		using enum MaterialResource;
		Vector<MaterialResourceBinding> materialBindings{
			MaterialResourceBinding{.Resource{ ConstantBuffer },			.Slot{ FORWARD_BINDING_MATERIAL_CONSTANTS } },
			MaterialResourceBinding{.Resource{ DiffuseTexture },			.Slot{ FORWARD_BINDING_MATERIAL_DIFFUSE_TEXTURE } },
			MaterialResourceBinding{.Resource{ SpecularTexture },			.Slot{ FORWARD_BINDING_MATERIAL_SPECULAR_TEXTURE } },
			MaterialResourceBinding{.Resource{ NormalTexture },				.Slot{ FORWARD_BINDING_MATERIAL_NORMAL_TEXTURE } },
			MaterialResourceBinding{.Resource{ EmissiveTexture },			.Slot{ FORWARD_BINDING_MATERIAL_EMISSIVE_TEXTURE } },
			MaterialResourceBinding{.Resource{ OcclusionTexture },			.Slot{ FORWARD_BINDING_MATERIAL_OCCLUSION_TEXTURE } },
			MaterialResourceBinding{.Resource{ TransmissionTexture },		.Slot{ FORWARD_BINDING_MATERIAL_TRANSMISSION_TEXTURE } },
			MaterialResourceBinding{.Resource{ OpacityTexture },			.Slot{ FORWARD_BINDING_MATERIAL_OPACITY_TEXTURE } }
		};

		return MakeShared<MaterialBindingCache<APITag>>(
			this->m_Device,
			RHI::RHIShaderType::Pixel,
			/* registerSpace = */ FORWARD_SPACE_MATERIAL,
			/* registerSpaceIsDescriptorSet = */ true,//TODO :Rempve
			materialBindings,
			commonPasses.m_AnisotropicWrapSampler.Get(),
			commonPasses.m_GrayTexture.Get(),
			commonPasses.m_BlackTexture.Get()
		);//TODO :add
	}

	template<RHI::APITagConcept APITag>
	inline auto ForwardShadingPass<APITag>::CreateGraphicsPipeline(PipelineKey key, Imp_FrameBuffer* framebuffer) -> RHI::RefCountPtr<Imp_GraphicsPipeline> {
		RHI::RHIGraphicsPipelineDescBuilder<APITag> pipelineDescBuilder{}; pipelineDescBuilder
			.Set_InputLayout(this->m_InputLayout)
			.Set_VS(this->m_VertexShader)
			.Set_GS(this->m_GeometryShader)
			.Set_FrontCounterClockwise(key.Bits.FrontCounterClockwise)
			.Set_CullMode(key.Bits.CullMode)
			.Set_DepthFunc(key.Bits.ReverseDepth ? RHI::RHIComparisonFunc::GreaterOrEqual : RHI::RHIComparisonFunc::LessOrEqual)
			.AddBindingLayout(this->m_MaterialBindings->Get_Layout())
			.AddBindingLayout(this->m_ViewBindingLayout)
			.AddBindingLayout(this->m_ShadingBindingLayout);

		if (!m_UseInputAssembler)
			pipelineDescBuilder.AddBindingLayout(this->m_InputBindingLayout);

		bool const framebufferUsesMSAA{ framebuffer->Get_Info().SampleCount > 1 };

		switch (key.Bits.Domain) {
		case MaterialDomain::Opaque:
			pipelineDescBuilder.Set_PS(this->m_PixelShader);
			break;

		case MaterialDomain::AlphaTested:
			pipelineDescBuilder
				.Set_PS(this->m_PixelShader)
				.Set_AlphaToCoverageEnable(framebufferUsesMSAA);
			break;

		case MaterialDomain::AlphaBlended: {
			pipelineDescBuilder
				.Set_PS(this->m_PixelShader)
				.Set_DepthWriteEnable(false)
				.Set_BlendState(0,
					RHI::RHIBlendState::RHIRenderTargetBuilder{}
					.Set_BlendEnable(true)
					.Set_SrcBlend(RHI::RHIBlendFactor::SrcAlpha)
					.Set_DestBlend(RHI::RHIBlendFactor::InvSrcAlpha)
					.Set_SrcBlendAlpha(RHI::RHIBlendFactor::Zero)
					.Set_DestBlendAlpha(RHI::RHIBlendFactor::One)
					.Build()
				);

			break;
		}

		case MaterialDomain::Transmissive:case MaterialDomain::TransmissiveAlphaTested:case MaterialDomain::TransmissiveAlphaBlended: {
			pipelineDescBuilder
				.Set_PS(this->m_PixelShaderTransmissive)
				.Set_DepthWriteEnable(false)
				.Set_BlendState(0,
					RHI::RHIBlendState::RHIRenderTargetBuilder{}
					.Set_BlendEnable(true)
					.Set_SrcBlend(RHI::RHIBlendFactor::One)
					.Set_DestBlend(RHI::RHIBlendFactor::Src1Color)
					.Set_SrcBlendAlpha(RHI::RHIBlendFactor::Zero)
					.Set_DestBlendAlpha(RHI::RHIBlendFactor::One)
					.Build()
				);
			break;

		}
		default: return nullptr;
		}

		return this->m_Device->CreateGraphicsPipeline(pipelineDescBuilder.Build(), framebuffer);
	}

	template<RHI::APITagConcept APITag>
	inline auto ForwardShadingPass<APITag>::GetOrCreateInputBindingSet(const BufferGroup<APITag>* bufferGroup) -> RHI::RefCountPtr<Imp_BindingSet> {

		if (auto it{ this->m_InputBindingSets.find(bufferGroup) }; it != this->m_InputBindingSets.end())
			return it->second;

		auto bindingSet{ this->CreateInputBindingSet(bufferGroup) };
		this->m_InputBindingSets[bufferGroup] = bindingSet;
		return bindingSet;
	}

	template<RHI::APITagConcept APITag>
	inline void ForwardShadingPass<APITag>::SetupView(GeometryPassContext& abstractContext, Imp_CommandList* commandList, const IView* view, const IView* viewPrev) {
		auto& context{ static_cast<Context&>(abstractContext) };

		ForwardShadingViewConstants viewConstants{};
		view->FillPlanarViewConstants(viewConstants.View);
		commandList->WriteBuffer(this->m_ForwardViewCB, &viewConstants, sizeof(viewConstants));

		context.KeyTemplate.Bits.FrontCounterClockwise = view->Is_Mirrored();
		context.KeyTemplate.Bits.ReverseDepth = view->Is_ReverseDepth();
	}

	template<RHI::APITagConcept APITag>
	inline bool ForwardShadingPass<APITag>::SetupMaterial(GeometryPassContext& abstractContext, const Material<APITag>* material, RHI::RHIRasterCullMode cullMode, RHI::RHIGraphicsState<APITag>& state) {
		auto& context{ static_cast<Context&>(abstractContext) };

		auto materialBindingSet{ this->m_MaterialBindings->Get_MaterialBindingSet(material) };

		if (nullptr == materialBindingSet)
			return false;

		ASSERT(cullMode == RHI::RHIRasterCullMode::None);

		PipelineKey key{ context.KeyTemplate };
		key.Bits.CullMode = cullMode;
		key.Bits.Domain = material->Domain;

		state.BindingSetCount = 0;
		state.BindingSets[state.BindingSetCount++] = materialBindingSet;
		state.BindingSets[state.BindingSetCount++] = this->m_ViewBindingSet;
		state.BindingSets[state.BindingSetCount++] = context.ShadingBindingSet;

		if (!m_UseInputAssembler)
			state.BindingSets[state.BindingSetCount++] = context.InputBindingSet;

		auto& pipeline{ this->m_Pipelines[key.Value] };

		if (nullptr == pipeline) {
			LockGuard lockGuard{ this->m_Mutex };

			if (nullptr == pipeline)
				pipeline = this->CreateGraphicsPipeline(key, state.FrameBuffer);

			if (nullptr == pipeline)
				return false;
		}

		ASSERT(pipeline->Get_FrameBufferInfo() == state.FrameBuffer->Get_Info());

		state.Pipeline = pipeline;
		return true;
	}

	template<RHI::APITagConcept APITag>
	inline void ForwardShadingPass<APITag>::SetupInputBuffers(GeometryPassContext& abstractContext, const BufferGroup<APITag>* buffers, RHI::RHIGraphicsState<APITag>& state) {
		auto& context{ static_cast<Context&>(abstractContext) };

		state.IndexBuffer = RHI::RHIIndexBufferBinding<APITag>{ .Buffer{ buffers->IndexBuffer }, .Format{ RHI::RHIFormat::R32_UINT } };

		state.VertexBufferCount = 0;
		if (this->m_UseInputAssembler) {
			state.VertexBuffers[state.VertexBufferCount++] = RHI::RHIVertexBufferBinding<APITag>{ .Buffer{ buffers->VertexBuffer }, .Slot{ 0 }, .Offset{ buffers->Get_VertexBufferRange(RHI::RHIVertexAttribute::Position).Offset } };
			state.VertexBuffers[state.VertexBufferCount++] = RHI::RHIVertexBufferBinding<APITag>{ .Buffer{ buffers->VertexBuffer }, .Slot{ 1 }, .Offset{ buffers->Get_VertexBufferRange(RHI::RHIVertexAttribute::PrevPosition).Offset } };
			state.VertexBuffers[state.VertexBufferCount++] = RHI::RHIVertexBufferBinding<APITag>{ .Buffer{ buffers->VertexBuffer }, .Slot{ 2 }, .Offset{ buffers->Get_VertexBufferRange(RHI::RHIVertexAttribute::TexCoord1).Offset } };
			state.VertexBuffers[state.VertexBufferCount++] = RHI::RHIVertexBufferBinding<APITag>{ .Buffer{ buffers->VertexBuffer }, .Slot{ 3 }, .Offset{ buffers->Get_VertexBufferRange(RHI::RHIVertexAttribute::Normal).Offset } };
			state.VertexBuffers[state.VertexBufferCount++] = RHI::RHIVertexBufferBinding<APITag>{ .Buffer{ buffers->VertexBuffer }, .Slot{ 4 }, .Offset{ buffers->Get_VertexBufferRange(RHI::RHIVertexAttribute::Tangent).Offset } };
			state.VertexBuffers[state.VertexBufferCount++] = RHI::RHIVertexBufferBinding<APITag>{ .Buffer{ buffers->InstanceBuffer }, .Slot{ 5 }, .Offset{ 0 } };
		}
		else {
			context.InputBindingSet = this->GetOrCreateInputBindingSet(buffers);
			context.PositionOffset = static_cast<Uint32>(buffers->Get_VertexBufferRange(RHI::RHIVertexAttribute::Position).Offset);
			context.TexCoordOffset = static_cast<Uint32>(buffers->Get_VertexBufferRange(RHI::RHIVertexAttribute::TexCoord1).Offset);
			context.NormalOffset = static_cast<Uint32>(buffers->Get_VertexBufferRange(RHI::RHIVertexAttribute::Normal).Offset);
			context.TangentOffset = static_cast<Uint32>(buffers->Get_VertexBufferRange(RHI::RHIVertexAttribute::Tangent).Offset);
		}
	}

	template<RHI::APITagConcept APITag>
	inline void ForwardShadingPass<APITag>::SetPushConstants(GeometryPassContext& abstractContext, Imp_CommandList* commandList, RHI::RHIGraphicsState<APITag>& state, RHI::RHIDrawArguments& args) {
		if (this->m_UseInputAssembler)
			return;

		auto& context{ static_cast<Context&>(abstractContext) };

		ForwardPushConstants constants{};
		constants.StartInstanceLocation = args.StartInstanceLocation;
		constants.StartVertexLocation = args.StartVertexLocation;
		constants.PositionOffset = context.PositionOffset;
		constants.TexCoordOffset = context.TexCoordOffset;
		constants.NormalOffset = context.NormalOffset;
		constants.TangentOffset = context.TangentOffset;

		commandList->SetPushConstants(&constants, sizeof(constants));

		args.StartInstanceLocation = 0;
		args.StartVertexLocation = 0;
	}

}