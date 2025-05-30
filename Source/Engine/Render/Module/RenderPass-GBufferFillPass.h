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

#include "Shader/gbuffer_cb.h"



#endif // PARTING_MODULE_BUILD


namespace Parting {

	namespace _NameSpace_GBufferFillPass {
		union PipelineKey {
			struct {
				RHI::RHIRasterCullMode CullMode : 2;
				bool AlphaTested : 1;
				bool FrontCounterClockwise : 1;
				bool ReverseDepth : 1;
			} Bits;
			Uint32 Value;

			static constexpr Uint32 COUNT{ 1 << 5 };
		};

	}

	template<RHI::APITagConcept APITag>
	class GBufferFillPass : public IGeometryPass<APITag> {
	protected:
		using Imp_Device = typename RHI::RHITypeTraits<APITag>::Imp_Device;
		using Imp_CommandList = typename RHI::RHITypeTraits<APITag>::Imp_CommandList;
		using Imp_Texture = typename RHI::RHITypeTraits<APITag>::Imp_Texture;
		using Imp_Heap = typename RHI::RHITypeTraits<APITag>::Imp_Heap;
		using Imp_Sampler = typename RHI::RHITypeTraits<APITag>::Imp_Sampler;
		using Imp_BindingSet = typename RHI::RHITypeTraits<APITag>::Imp_BindingSet;
		using Imp_BindingLayout = typename RHI::RHITypeTraits<APITag>::Imp_BindingLayout;
		using Imp_InputLayout = typename RHI::RHITypeTraits<APITag>::Imp_InputLayout;
		using Imp_Shader = typename RHI::RHITypeTraits<APITag>::Imp_Shader;
		using Imp_FrameBuffer = typename RHI::RHITypeTraits<APITag>::Imp_FrameBuffer;
		using Imp_Buffer = typename RHI::RHITypeTraits<APITag>::Imp_Buffer;
		using Imp_GraphicsPipeline = typename RHI::RHITypeTraits<APITag>::Imp_GraphicsPipeline;
	public:
		using PipelineKey = _NameSpace_GBufferFillPass::PipelineKey;

		class Context final : public GeometryPassContext {
		public:
			RHI::RefCountPtr<Imp_BindingSet> InputBindingSet;
			PipelineKey KeyTemplate;

			Uint32 PositionOffset{ 0 };
			Uint32 PrevPositionOffset{ 0 };
			Uint32 TexCoordOffset{ 0 };
			Uint32 NormalOffset{ 0 };
			Uint32 TangentOffset{ 0 };

			Context(void) { this->KeyTemplate.Value = 0; }
			~Context(void) = default;
		};

		struct CreateParameters final {
			SharedPtr<MaterialBindingCache<APITag>> MaterialBindings;
			bool EnableSinglePassCubemap{ false };
			bool EnableDepthWrite{ true };
			bool EnableMotionVectors{ false };
			bool TrackLiveness{ true };

			// Switches between loading vertex data through the Input Assembler (true) or buffer SRVs (false).
			// Using Buffer SRVs is often faster.
			bool UseInputAssembler{ false };

			Uint32 StencilWriteMask{ 0 };
			Uint32 NumConstantBufferVersions{ 16 };
		};


	public:
		GBufferFillPass(Imp_Device* device, SharedPtr<CommonRenderPasses<APITag>> commonPasses);
		virtual ~GBufferFillPass(void) = default;

	public:
		auto GetOrCreateInputBindingSet(const BufferGroup<APITag>* bufferGroup) -> RHI::RefCountPtr<Imp_BindingSet>;

		void ResetBindingCache(void);

	protected:
		RHI::RefCountPtr<Imp_Device> m_Device;
		RHI::RefCountPtr<Imp_InputLayout> m_InputLayout;
		RHI::RefCountPtr<Imp_Shader> m_VertexShader;
		RHI::RefCountPtr<Imp_Shader> m_PixelShader;
		RHI::RefCountPtr<Imp_Shader> m_PixelShaderAlphaTested;
		RHI::RefCountPtr<Imp_Shader> m_GeometryShader;
		RHI::RefCountPtr<Imp_BindingLayout> m_InputBindingLayout;
		RHI::RefCountPtr<Imp_BindingLayout> m_ViewBindingLayout;
		RHI::RefCountPtr<Imp_BindingSet> m_ViewBindingSet;
		RHI::RefCountPtr<Imp_Buffer> m_GBufferCB;
		Array<RHI::RefCountPtr<Imp_GraphicsPipeline>, PipelineKey::COUNT> m_Pipelines;

		ViewType m_SupportedViewTypes{ ViewType::PLANAR };

		Mutex m_Mutex;

		UnorderedMap<const BufferGroup<APITag>*, RHI::RefCountPtr<Imp_BindingSet>> m_InputBindingSets;

		SharedPtr<CommonRenderPasses<APITag>> m_CommonPasses;
		SharedPtr<MaterialBindingCache<APITag>> m_MaterialBindings;

		bool m_EnableDepthWrite{ true };
		bool m_EnableMotionVectors{ false };
		bool m_UseInputAssembler{ false };
		Uint32 m_StencilWriteMask{ 0 };

	public:

	public:
		STDNODISCARD ViewType Get_SupportedViewTypes(void) const override { return this->m_SupportedViewTypes; }

		void SetupView(GeometryPassContext& context, Imp_CommandList* commandList, const IView* view, const IView* viewPrev) override;

		bool SetupMaterial(GeometryPassContext& context, const Material<APITag>* material, RHI::RHIRasterCullMode cullMode, RHI::RHIGraphicsState<APITag>& state) override;

		void SetupInputBuffers(GeometryPassContext& context, const BufferGroup<APITag>* buffers, RHI::RHIGraphicsState<APITag>& state) override;

		void SetPushConstants(GeometryPassContext& context, Imp_CommandList* commandList, RHI::RHIGraphicsState<APITag>& state, RHI::RHIDrawArguments& args) override;


	public:


		virtual auto CreateVertexShader(ShaderFactory<APITag>& shaderFactory, const CreateParameters& params) -> RHI::RefCountPtr<Imp_Shader>;

		virtual auto CreateGeometryShader(ShaderFactory<APITag>& shaderFactory, const CreateParameters& params) -> RHI::RefCountPtr<Imp_Shader>;

		virtual auto CreatePixelShader(ShaderFactory<APITag>& shaderFactory, const CreateParameters& params, bool alphaTested) -> RHI::RefCountPtr<Imp_Shader>;

		virtual auto CreateInputLayout(Imp_Shader* vertexShader, const CreateParameters& params) -> RHI::RefCountPtr<Imp_InputLayout>;

		virtual auto CreateInputBindingLayout(void) -> RHI::RefCountPtr<Imp_BindingLayout>;

		virtual auto CreateInputBindingSet(const BufferGroup<APITag>* bufferGroup) -> RHI::RefCountPtr<Imp_BindingSet>;

		virtual auto CreateViewBindings(const CreateParameters& params) -> Tuple<RHI::RefCountPtr<Imp_BindingLayout>, RHI::RefCountPtr<Imp_BindingSet>>;

		virtual auto CreateMaterialBindingCache(CommonRenderPasses<APITag>& commonPasses) -> SharedPtr<MaterialBindingCache<APITag>>;

		virtual auto CreateGraphicsPipeline(PipelineKey key, Imp_FrameBuffer* sampleFramebuffer) -> RHI::RefCountPtr<Imp_GraphicsPipeline>;

		virtual void Init(ShaderFactory<APITag>& shaderFactory, const CreateParameters& params);

	};


	template<RHI::APITagConcept APITag>
	class MaterialIDPass final : public GBufferFillPass<APITag> {
	public:
		MaterialIDPass(GBufferFillPass<APITag>::Imp_Device* device, SharedPtr<CommonRenderPasses<APITag>> commonPasses) :
			GBufferFillPass<APITag>{ device, ::MoveTemp(commonPasses) } {

		}
		~MaterialIDPass(void) = default;

	public:
		void Init(ShaderFactory<APITag>& shaderFactory, const GBufferFillPass<APITag>::CreateParameters& params) override;

	protected:
		auto CreatePixelShader(ShaderFactory<APITag>& shaderFactory, const GBufferFillPass<APITag>::CreateParameters& params, bool alphaTested) -> RHI::RefCountPtr<typename GBufferFillPass<APITag>::Imp_Shader> override;

	};





	template<RHI::APITagConcept APITag>
	inline GBufferFillPass<APITag>::GBufferFillPass(Imp_Device* device, SharedPtr<CommonRenderPasses<APITag>> commonPasses) :
		IGeometryPass<APITag>{},
		m_Device{ device },
		m_CommonPasses{ ::MoveTemp(commonPasses) } {
	}

	template<RHI::APITagConcept APITag>
	inline auto GBufferFillPass<APITag>::GetOrCreateInputBindingSet(const BufferGroup<APITag>* bufferGroup)->RHI::RefCountPtr<Imp_BindingSet> {
		if (auto it{ this->m_InputBindingSets.find(bufferGroup) }; it != this->m_InputBindingSets.end())
			return it->second;

		auto bindingSet{ this->CreateInputBindingSet(bufferGroup) };
		this->m_InputBindingSets[bufferGroup] = bindingSet;
		return bindingSet;
	}

	template<RHI::APITagConcept APITag>
	inline void GBufferFillPass<APITag>::ResetBindingCache(void) {
		this->m_MaterialBindings->Clear();
		this->m_InputBindingSets.clear();
	}




	template<RHI::APITagConcept APITag>
	inline void GBufferFillPass<APITag>::SetupView(GeometryPassContext& abstractContext, Imp_CommandList* commandList, const IView* view, const IView* viewPrev) {
		auto& context{ static_cast<Context&>(abstractContext) };

		GBufferFillConstants gbufferConstants{};
		view->FillPlanarViewConstants(gbufferConstants.View);
		viewPrev->FillPlanarViewConstants(gbufferConstants.ViewPrev);
		commandList->WriteBuffer(this->m_GBufferCB, &gbufferConstants, sizeof(decltype(gbufferConstants)));

		context.KeyTemplate.Bits.FrontCounterClockwise = view->Is_Mirrored();
		context.KeyTemplate.Bits.ReverseDepth = view->Is_ReverseDepth();
	}

	template<RHI::APITagConcept APITag>
	inline bool GBufferFillPass<APITag>::SetupMaterial(GeometryPassContext& abstractContext, const Material<APITag>* material, RHI::RHIRasterCullMode cullMode, RHI::RHIGraphicsState<APITag>& state) {
		auto& context{ static_cast<Context&>(abstractContext) };

		PipelineKey key{ context.KeyTemplate };
		key.Bits.CullMode = cullMode;

		switch (material->Domain) {
			using enum MaterialDomain;
			/* Case AlphaBlended : Blended and transmissive domains are for the material ID pass, shouldn't be used otherwise*/
		case Opaque:case AlphaBlended:case Transmissive:case TransmissiveAlphaTested:case TransmissiveAlphaBlended:
			key.Bits.AlphaTested = false;
			break;

		case MaterialDomain::AlphaTested:
			key.Bits.AlphaTested = true;
			break;

		default:
			return false;
		}

		auto materialBindingSet{ this->m_MaterialBindings->Get_MaterialBindingSet(material) };

		if (nullptr == materialBindingSet)
			return false;

		state.BindingSetCount = 0;
		state.BindingSets[state.BindingSetCount++] = materialBindingSet;
		state.BindingSets[state.BindingSetCount++] = this->m_ViewBindingSet;

		if (!this->m_UseInputAssembler)
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
	inline void GBufferFillPass<APITag>::SetupInputBuffers(GeometryPassContext& abstractContext, const BufferGroup<APITag>* buffers, RHI::RHIGraphicsState<APITag>& state) {
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
			context.PrevPositionOffset = static_cast<Uint32>(buffers->Get_VertexBufferRange(RHI::RHIVertexAttribute::PrevPosition).Offset);
			context.TexCoordOffset = static_cast<Uint32>(buffers->Get_VertexBufferRange(RHI::RHIVertexAttribute::TexCoord1).Offset);
			context.NormalOffset = static_cast<Uint32>(buffers->Get_VertexBufferRange(RHI::RHIVertexAttribute::Normal).Offset);
			context.TangentOffset = static_cast<Uint32>(buffers->Get_VertexBufferRange(RHI::RHIVertexAttribute::Tangent).Offset);
		}
	}

	template<RHI::APITagConcept APITag>
	inline void GBufferFillPass<APITag>::SetPushConstants(GeometryPassContext& abstractContext, Imp_CommandList* commandList, RHI::RHIGraphicsState<APITag>& state, RHI::RHIDrawArguments& args) {
		if (this->m_UseInputAssembler)
			return;

		auto& context{ static_cast<Context&>(abstractContext) };

		GBufferPushConstants constants{};
		constants.StartInstanceLocation = args.StartInstanceLocation;
		constants.StartVertexLocation = args.StartVertexLocation;
		constants.PositionOffset = context.PositionOffset;
		constants.PrevPositionOffset = context.PrevPositionOffset;
		constants.TexCoordOffset = context.TexCoordOffset;
		constants.NormalOffset = context.NormalOffset;
		constants.TangentOffset = context.TangentOffset;

		commandList->SetPushConstants(&constants, sizeof(GBufferPushConstants));

		args.StartInstanceLocation = 0;
		args.StartVertexLocation = 0;
	}

	template<RHI::APITagConcept APITag>
	inline auto GBufferFillPass<APITag>::CreateVertexShader(ShaderFactory<APITag>& shaderFactory, const CreateParameters& params) -> RHI::RefCountPtr<Imp_Shader> {
		const String sourceFileName{ "Parting/Passes/gbuffer_vs.hlsl" };

		Vector<ShaderMacro> VertexShaderMacros{
			ShaderMacro{.Name{ String{ "MOTION_VECTORS" } }, .Definition{ String{ params.EnableMotionVectors ? "1" : "0" } } }
		};

		if (params.UseInputAssembler)
			return shaderFactory.CreateShader(sourceFileName, String{ "input_assembler" }, &VertexShaderMacros, RHI::RHIShaderType::Vertex);
		else
			return shaderFactory.CreateShader(sourceFileName, String{ "buffer_loads" }, &VertexShaderMacros, RHI::RHIShaderType::Vertex);
	}

	template<RHI::APITagConcept APITag>
	inline auto GBufferFillPass<APITag>::CreateGeometryShader(ShaderFactory<APITag>& shaderFactory, const CreateParameters& params) -> RHI::RefCountPtr<Imp_Shader> {
		if (params.EnableSinglePassCubemap) {

			ASSERT(false);

			// MVs will not work with cubemap views because:
			// 1. cubemap_gs does not pass through the previous position attribute;
			// 2. Computing correct MVs for a cubemap is complicated and not implemented.
			/*ASSERT(!params.EnableMotionVectors);

			ShaderMacro MotionVectorsMacro("MOTION_VECTORS", params.EnableMotionVectors ? "1" : "0");

			auto desc = nvrhi::ShaderDesc()
				.setShaderType(nvrhi::ShaderType::Geometry)
				.setFastGSFlags(nvrhi::FastGeometryShaderFlags(
					nvrhi::FastGeometryShaderFlags::ForceFastGS |
					nvrhi::FastGeometryShaderFlags::UseViewportMask |
					nvrhi::FastGeometryShaderFlags::OffsetTargetIndexByViewportIndex))
				.setCoordinateSwizzling(CubemapView::GetCubemapCoordinateSwizzle());

			return shaderFactory.CreateAutoShader("donut/passes/cubemap_gs.hlsl", "main", DONUT_MAKE_PLATFORM_SHADER(g_cubemap_gs), nullptr, desc);*/
		}

		return nullptr;
	}

	template<RHI::APITagConcept APITag>
	inline auto GBufferFillPass<APITag>::CreatePixelShader(ShaderFactory<APITag>& shaderFactory, const CreateParameters& params, bool alphaTested) -> RHI::RefCountPtr<Imp_Shader> {
		Vector<ShaderMacro> PixelShaderMacros{
			ShaderMacro{.Name{ String{ "MOTION_VECTORS" } }, .Definition{ String{ params.EnableMotionVectors ? "1" : "0" } } },
			ShaderMacro{.Name{ String{ "ALPHA_TESTED" } }, .Definition{ String{ alphaTested ? "1" : "0" } } }
		};

		return shaderFactory.CreateShader(String{ "Parting/Passes/gbuffer_ps.hlsl" }, String{ "main" }, &PixelShaderMacros, RHI::RHIShaderType::Pixel);
	}

	template<RHI::APITagConcept APITag>
	inline auto GBufferFillPass<APITag>::CreateInputLayout(Imp_Shader* vertexShader, const CreateParameters& params) -> RHI::RefCountPtr<Imp_InputLayout> {
		if (params.UseInputAssembler) {
			RHI::RHIVertexAttributeDescBuilder vertexAttribDescBuilder{};

			Vector<RHI::RHIVertexAttributeDesc> inputDescs{
				BuildVertexAttributeDesc(vertexAttribDescBuilder.Reset(), RHI::RHIVertexAttribute::Position, String{ "POS" }, 0),
				BuildVertexAttributeDesc(vertexAttribDescBuilder.Reset(), RHI::RHIVertexAttribute::PrevPosition, String{ "PREV_POS" }, 1),
				BuildVertexAttributeDesc(vertexAttribDescBuilder.Reset(), RHI::RHIVertexAttribute::TexCoord1, String{ "TEXCOORD" }, 2),
				BuildVertexAttributeDesc(vertexAttribDescBuilder.Reset(), RHI::RHIVertexAttribute::Normal, String{ "NORMAL" }, 3),
				BuildVertexAttributeDesc(vertexAttribDescBuilder.Reset(), RHI::RHIVertexAttribute::Tangent, String{ "TANGENT" }, 4),
				BuildVertexAttributeDesc(vertexAttribDescBuilder.Reset(), RHI::RHIVertexAttribute::Transform, String{ "TRANSFORM" }, 5)
			};
			if (params.EnableMotionVectors)
				inputDescs.emplace_back(BuildVertexAttributeDesc(vertexAttribDescBuilder.Reset(), RHI::RHIVertexAttribute::PrevTransform, "PREV_TRANSFORM", 6));

			return this->m_Device->CreateInputLayout(inputDescs.data(), static_cast<Uint32>(inputDescs.size()));
		}

		return nullptr;
	}

	template<RHI::APITagConcept APITag>
	inline auto GBufferFillPass<APITag>::CreateInputBindingLayout(void) -> RHI::RefCountPtr<Imp_BindingLayout> {
		if (this->m_UseInputAssembler)
			return nullptr;

		return this->m_Device->CreateBindingLayout(RHI::RHIBindingLayoutDescBuilder{}
			.Set_Visibility(RHI::RHIShaderType::Vertex | RHI::RHIShaderType::Pixel)
			.Set_RegisterSpace(GBUFFER_SPACE_INPUT)
			.Set_RegisterSpaceIsDescriptorSet(true)
			.AddBinding(RHI::RHIBindingLayoutItem::StructuredBuffer_SRV(GBUFFER_BINDING_INSTANCE_BUFFER))
			.AddBinding(RHI::RHIBindingLayoutItem::RawBuffer_SRV(GBUFFER_BINDING_VERTEX_BUFFER))
			.AddBinding(RHI::RHIBindingLayoutItem::PushConstants(GBUFFER_BINDING_PUSH_CONSTANTS, sizeof(GBufferPushConstants)))
			.Build()
		);
	}

	template<RHI::APITagConcept APITag>
	inline auto GBufferFillPass<APITag>::CreateInputBindingSet(const BufferGroup<APITag>* bufferGroup) -> RHI::RefCountPtr<Imp_BindingSet> {
		return this->m_Device->CreateBindingSet(RHI::RHIBindingSetDescBuilder<APITag>{}
		.AddBinding(RHI::RHIBindingSetItem<APITag>::StructuredBuffer_SRV(GBUFFER_BINDING_INSTANCE_BUFFER, bufferGroup->InstanceBuffer))
			.AddBinding(RHI::RHIBindingSetItem<APITag>::RawBuffer_SRV(GBUFFER_BINDING_VERTEX_BUFFER, bufferGroup->VertexBuffer))
			.AddBinding(RHI::RHIBindingSetItem<APITag>::PushConstants(GBUFFER_BINDING_PUSH_CONSTANTS, sizeof(GBufferPushConstants)))
			.Build(),
			this->m_InputBindingLayout
			);
	}

	template<RHI::APITagConcept APITag>
	inline auto GBufferFillPass<APITag>::CreateViewBindings(const CreateParameters& params) -> Tuple<RHI::RefCountPtr<Imp_BindingLayout>, RHI::RefCountPtr<Imp_BindingSet>> {
		return this->m_Device->CreateBindingLayoutAndSet(
			RHI::RHIShaderType::Vertex | RHI::RHIShaderType::Pixel,
			GBUFFER_SPACE_VIEW,
			RHI::RHIBindingSetDescBuilder<APITag>{}
		.AddBinding(RHI::RHIBindingSetItem<APITag>::ConstantBuffer(GBUFFER_BINDING_VIEW_CONSTANTS, this->m_GBufferCB))
			.AddBinding(RHI::RHIBindingSetItem<APITag>::Sampler(GBUFFER_BINDING_MATERIAL_SAMPLER, this->m_CommonPasses->m_AnisotropicWrapSampler))
			.Build()
			);
	}

	template<RHI::APITagConcept APITag>
	inline auto GBufferFillPass<APITag>::CreateMaterialBindingCache(CommonRenderPasses<APITag>& commonPasses) -> SharedPtr<MaterialBindingCache<APITag>> {
		using enum MaterialResource;
		Vector<MaterialResourceBinding> materialBindings{
			MaterialResourceBinding{.Resource{ ConstantBuffer },		.Slot{ GBUFFER_BINDING_MATERIAL_CONSTANTS } },
			MaterialResourceBinding{.Resource{ DiffuseTexture },		.Slot{ GBUFFER_BINDING_MATERIAL_DIFFUSE_TEXTURE } },
			MaterialResourceBinding{.Resource{ SpecularTexture },		.Slot{ GBUFFER_BINDING_MATERIAL_SPECULAR_TEXTURE } },
			MaterialResourceBinding{.Resource{ NormalTexture },			.Slot{ GBUFFER_BINDING_MATERIAL_NORMAL_TEXTURE } },
			MaterialResourceBinding{.Resource{ EmissiveTexture },		.Slot{ GBUFFER_BINDING_MATERIAL_EMISSIVE_TEXTURE } },
			MaterialResourceBinding{.Resource{ OcclusionTexture },		.Slot{ GBUFFER_BINDING_MATERIAL_OCCLUSION_TEXTURE } },
			MaterialResourceBinding{.Resource{ TransmissionTexture },	.Slot{ GBUFFER_BINDING_MATERIAL_TRANSMISSION_TEXTURE } },
			MaterialResourceBinding{.Resource{ OpacityTexture },		.Slot{ GBUFFER_BINDING_MATERIAL_OPACITY_TEXTURE } }
		};

		return MakeShared<MaterialBindingCache<APITag>>(
			this->m_Device,
			RHI::RHIShaderType::Pixel,
			/* registerSpace = */ GBUFFER_SPACE_MATERIAL,
			/* registerSpaceIsDescriptorSet = */ true,
			materialBindings,
			commonPasses.m_AnisotropicWrapSampler,
			commonPasses.m_GrayTexture,
			commonPasses.m_BlackTexture
		);
	}

	template<RHI::APITagConcept APITag>
	inline auto GBufferFillPass<APITag>::CreateGraphicsPipeline(PipelineKey key, Imp_FrameBuffer* sampleFramebuffer) -> RHI::RefCountPtr<Imp_GraphicsPipeline> {
		RHI::RHIGraphicsPipelineDescBuilder<APITag> pipelineDescBuilder; pipelineDescBuilder
			.Set_InputLayout(this->m_InputLayout)
			.Set_VS(this->m_VertexShader)
			.Set_GS(this->m_GeometryShader)
			.Set_FrontCounterClockwise(key.Bits.FrontCounterClockwise)
			.Set_CullMode(key.Bits.CullMode)
			.Set_AlphaToCoverageEnable(false)
			.Set_DepthWriteEnable(this->m_EnableDepthWrite)
			.Set_DepthFunc(key.Bits.ReverseDepth ? RHI::RHIComparisonFunc::GreaterOrEqual : RHI::RHIComparisonFunc::LessOrEqual)
			.AddBindingLayout(this->m_MaterialBindings->Get_Layout())
			.AddBindingLayout(this->m_ViewBindingLayout);


		if (!this->m_UseInputAssembler)
			pipelineDescBuilder.AddBindingLayout(this->m_InputBindingLayout);

		if (0u != this->m_StencilWriteMask)
			pipelineDescBuilder
			.Set_StencilEnable(true)
			.Set_StencilReadMask(0)
			.Set_StencilWriteMask(this->m_StencilWriteMask)
			.Set_StencilRefValue(this->m_StencilWriteMask)
			.Set_StencilFrontFacePassOp(RHI::RHIStencilOp::Replace)
			.Set_StencilBackFaceFailOp(RHI::RHIStencilOp::Replace);



		if (key.Bits.AlphaTested) {
			pipelineDescBuilder.Set_CullMode(RHI::RHIRasterCullMode::None);

			if (nullptr != this->m_PixelShaderAlphaTested)
				pipelineDescBuilder.Set_PS(this->m_PixelShaderAlphaTested);
			else
				pipelineDescBuilder.Set_PS(this->m_PixelShader).Set_AlphaToCoverageEnable(true);
		}
		else
			pipelineDescBuilder.Set_PS(this->m_PixelShader);

		return this->m_Device->CreateGraphicsPipeline(pipelineDescBuilder.Build(), sampleFramebuffer);
	}

	template<RHI::APITagConcept APITag>
	inline void GBufferFillPass<APITag>::Init(ShaderFactory<APITag>& shaderFactory, const CreateParameters& params) {
		this->m_EnableMotionVectors = params.EnableMotionVectors;
		this->m_UseInputAssembler = params.UseInputAssembler;

		this->m_EnableDepthWrite = params.EnableDepthWrite;
		this->m_StencilWriteMask = params.StencilWriteMask;

		this->m_SupportedViewTypes = ViewType::PLANAR;
		if (params.EnableSinglePassCubemap)
			this->m_SupportedViewTypes |= ViewType::CUBEMAP;

		this->m_VertexShader = this->CreateVertexShader(shaderFactory, params);
		this->m_InputLayout = this->CreateInputLayout(this->m_VertexShader.Get(), params);
		this->m_GeometryShader = this->CreateGeometryShader(shaderFactory, params);
		this->m_PixelShader = this->CreatePixelShader(shaderFactory, params, false);
		this->m_PixelShaderAlphaTested = this->CreatePixelShader(shaderFactory, params, true);

		if (nullptr != params.MaterialBindings)
			this->m_MaterialBindings = params.MaterialBindings;
		else
			this->m_MaterialBindings = this->CreateMaterialBindingCache(*this->m_CommonPasses);

		this->m_GBufferCB = this->m_Device->CreateBuffer(RHI::RHIBufferDescBuilder{}
			.Set_ByteSize(sizeof(GBufferFillConstants))
			.Set_MaxVersions(params.NumConstantBufferVersions)
			.Set_DebugName(_W("GBufferFillConstants"))
			.Set_IsVolatile(true)
			.Set_IsConstantBuffer(true)
			.Build()
		);


		Tie(this->m_ViewBindingLayout, this->m_ViewBindingSet) = this->CreateViewBindings(params);

		this->m_InputBindingLayout = this->CreateInputBindingLayout();
	}

	template<RHI::APITagConcept APITag>
	inline void MaterialIDPass<APITag>::Init(ShaderFactory<APITag>& shaderFactory, const GBufferFillPass<APITag>::CreateParameters& params) {
		auto paramsCopy{ params };
		// The material ID pass relies on the push constants filled by the buffer load path (firstInstance)
		paramsCopy.UseInputAssembler = false;
		// The material ID pass doesn't support generating motion vectors
		paramsCopy.EnableMotionVectors = false;

		GBufferFillPass<APITag>::Init(shaderFactory, paramsCopy);
	}

	template<RHI::APITagConcept APITag>
	inline auto MaterialIDPass<APITag>::CreatePixelShader(ShaderFactory<APITag>& shaderFactory, const GBufferFillPass<APITag>::CreateParameters& params, bool alphaTested) -> RHI::RefCountPtr<typename GBufferFillPass<APITag>::Imp_Shader> {
		Vector<ShaderMacro> PixelShaderMacros{
		ShaderMacro{.Name{ String{ "ALPHA_TESTED" } }, .Definition{ String{ params.EnableMotionVectors ? "1" : "0" } } }
		};

		return shaderFactory.CreateShader("Parting/Passes/material_id_ps.hlsl", "main", &PixelShaderMacros, RHI::RHIShaderType::Pixel);
	}

}