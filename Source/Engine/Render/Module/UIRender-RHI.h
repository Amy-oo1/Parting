#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"

PARTING_MODULE(BindingCache)

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;


#else
#pragma once

#include "Core/ModuleBuild.h"


#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"

#include "ThirdParty/imgui/imgui.h"
//Global

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#include "Core/Concurrent/Module/Concurrent.h"
#include "Core/Container/Module/Container.h"
#include "Core/String/Module/String.h"
#include "Core/VFS/Module/VFS.h"
#include "Core/Logger/Module/Logger.h"

#include "RHI/Module/RHI.h"
#include "D3D12RHI/Module/D3D12RHI.h"

#include "Engine/Engine/Module/ShaderFactory.h"

#endif // PARTING_MODULE_BUILD

namespace Parting {

	template<RHI::APITagConcept APITag>
	struct UIRHI final {
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



		RHI::RefCountPtr<Imp_Device> Device;
		RHI::RefCountPtr<Imp_CommandList> CommandList;

		RHI::RefCountPtr<Imp_Shader> VS;
		RHI::RefCountPtr<Imp_Shader> PS;
		RHI::RefCountPtr<Imp_InputLayout> ShaderAttribLayout;

		RHI::RefCountPtr<Imp_Texture> FontTexture;
		RHI::RefCountPtr<Imp_Sampler> FontSampler;

		RHI::RefCountPtr<Imp_Buffer> VertexBuffer;
		RHI::RefCountPtr<Imp_Buffer> IndexBuffer;

		RHI::RefCountPtr<Imp_BindingLayout> BindingLayout;
		UnorderedMap<Imp_Texture*, RHI::RefCountPtr<Imp_BindingSet>> BindingsCache;

		RHI::RHIGraphicsPipelineDesc<APITag> BasePSODesc;
		RHI::RefCountPtr<Imp_GraphicsPipeline> PSO;

		Vector<ImDrawVert> ImGuiVertexBuffer;
		Vector<ImDrawIdx> ImGuiIndexBuffer;

		bool Initialize(Imp_Device* device, SharedPtr<ShaderFactory<APITag>> shaderFactory);

		bool UpdateFontTexture(void);

		bool UpdateGeometry(Imp_CommandList* commandList);

		bool Render(Imp_FrameBuffer* framebuffer);

		void BackBufferResizing(void) { this->PSO.Reset(); }


		bool ReallocateBuffer(RHI::RefCountPtr<Imp_Buffer>& buffer, Uint64 requiredSize, Uint64 reallocateSize, bool isIndexBuffer);

		Imp_GraphicsPipeline* GetOrCreatePSO(Imp_FrameBuffer* framebuffer) {
			if (nullptr == this->PSO)
				this->PSO = this->Device->CreateGraphicsPipeline(this->BasePSODesc, framebuffer);
			ASSERT(nullptr != this->PSO);

			return this->PSO.Get();

		}

		Imp_BindingSet* GetOrCreateBindingSet(Imp_Texture* texture) {
			if (auto it{ this->BindingsCache.find(texture) }; it != this->BindingsCache.end())
				return it->second.Get();
			// create binding set
			RHI::RefCountPtr<Imp_BindingSet> bindingSet = this->Device->CreateBindingSet(RHI::RHIBindingSetDescBuilder<APITag>{}
			.AddBinding(RHI::RHIBindingSetItem<APITag>::PushConstants(0, sizeof(float) * 2))
				.AddBinding(RHI::RHIBindingSetItem<APITag>::Texture_SRV(0, texture))
				.AddBinding(RHI::RHIBindingSetItem<APITag>::Sampler(0, this->FontSampler.Get()))
				.Build(),
				this->BindingLayout.Get()
				);

			ASSERT(nullptr != bindingSet);

			this->BindingsCache[texture] = bindingSet;
			return bindingSet.Get();
		}

	};









	template<RHI::APITagConcept APITag>
	inline bool UIRHI<APITag>::Initialize(Imp_Device* device, SharedPtr<ShaderFactory<APITag>> shaderFactory) {
		this->Device = device;
		this->CommandList = device->CreateCommandList();

		this->VS = shaderFactory->CreateShader(String{ "Parting/imgui_vertex" }, String{ "main" }, nullptr, RHI::RHIShaderType::Vertex);
		this->PS = shaderFactory->CreateShader(String{ "Parting/imgui_pixel" }, String{ "main" }, nullptr, RHI::RHIShaderType::Pixel);

		if (nullptr == this->VS || nullptr == this->PS) {
			LOG_ERROR("Failed to create ImGui shaders");
			return false;
		}

		// create attribute layout object
		RHI::RHIVertexAttributeDescBuilder vertexAttribDescBuilder; vertexAttribDescBuilder.Set_ElementStride(sizeof(ImDrawVert));
		Array<RHI::RHIVertexAttributeDesc, 3> vertexAttribLayout{
			RHI::RHIVertexAttributeDesc{ vertexAttribDescBuilder.Set_Attribute(RHI::RHIVertexAttribute::Position).Set_Name("POSITION").Set_Format(RHI::RHIFormat::RG32_FLOAT).Set_Offset(offsetof(ImDrawVert,pos)).Build()},
			RHI::RHIVertexAttributeDesc{ vertexAttribDescBuilder.Set_Attribute(RHI::RHIVertexAttribute::TexCoord1).Set_Name("TEXCOORD").Set_Format(RHI::RHIFormat::RG32_FLOAT).Set_Offset(offsetof(ImDrawVert,uv)).Build() },
			RHI::RHIVertexAttributeDesc{ vertexAttribDescBuilder.Set_Attribute(RHI::RHIVertexAttribute::COUNT).Set_Name("COLOR").Set_Format(RHI::RHIFormat::RGBA8_UNORM).Set_Offset(offsetof(ImDrawVert,col)).Build() }
		};

		this->ShaderAttribLayout = this->Device->CreateInputLayout(vertexAttribLayout.data(), static_cast<Uint32>(vertexAttribLayout.size()));

		// create PSO
		{
			this->BindingLayout = this->Device->CreateBindingLayout(RHI::RHIBindingLayoutDescBuilder{}
				.Set_Visibility(RHI::RHIShaderType::All)
				.AddBinding(RHI::RHIBindingLayoutItem::PushConstants(0, sizeof(float) * 2))
				.AddBinding(RHI::RHIBindingLayoutItem::Texture_SRV(0))
				.AddBinding(RHI::RHIBindingLayoutItem::Sampler(0))
				.Build()
			);

			RHI::RHIRenderState renderState;

			RHI::RHIBlendState blendState;
			blendState.RenderTargets[0] = RHI::RHIBlendState::RHIRenderTargetBuilder{}
				.Set_BlendEnable(true)
				.Set_SrcBlend(RHI::RHIBlendFactor::SrcAlpha)
				.Set_DestBlend(RHI::RHIBlendFactor::InvSrcAlpha)
				.Set_SrcBlendAlpha(RHI::RHIBlendFactor::InvSrcAlpha)
				.Set_DestBlendAlpha(RHI::RHIBlendFactor::Zero)
				.Build();
			renderState.BlendState = blendState;

			renderState.DepthStencilState = RHI::RHIDepthStencilStateBuilder{}
				.Set_DepthTestEnable(false)
				.Set_DepthWriteEnable(false)
				.Set_DepthFunc(RHI::RHIComparisonFunc::Always)
				.Build();

			renderState.RasterState = RHI::RHIRasterStateBuilder{}
				.Set_CullMode(RHI::RHIRasterCullMode::None)
				.Set_DepthClipEnable(true)
				.Set_ScissorEnable(true)
				.Build();

			this->BasePSODesc.PrimType = RHI::RHIPrimitiveType::TriangleList;
			this->BasePSODesc.InputLayout = this->ShaderAttribLayout;
			this->BasePSODesc.VS = this->VS.Get();
			this->BasePSODesc.PS = this->PS.Get();
			this->BasePSODesc.RenderState = renderState;
			this->BasePSODesc.BindingLayouts[this->BasePSODesc.BindingLayoutCount++] = this->BindingLayout;
		}

		this->FontSampler = this->Device->CreateSampler(RHI::RHISamplerDescBuilder{}
			.Set_AddressModeUVW(RHI::RHISamplerAddressMode::Wrap)
			.Set_AllFilter(true)
			.Build()
		);

		return nullptr != this->FontSampler;
	}

	template<RHI::APITagConcept APITag>
	inline bool UIRHI<APITag>::UpdateFontTexture(void) {
		ImGuiIO& io{ ImGui::GetIO() };

		// If the font texture exists and is bound to ImGui, we're done.
		// Note:  will reset io.Fonts->TexID when new fonts are added.
		if (nullptr != this->FontTexture && nullptr != io.Fonts->TexID)
			return true;

		unsigned char* pixels;
		Int32 width, height;
		if (io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height); nullptr == pixels)
			return false;

		this->FontTexture = this->Device->CreateTexture(RHI::RHITextureDescBuilder{}
			.Set_Width(static_cast<Uint32>(width)).Set_Height(static_cast<Uint32>(height))
			.Set_Format(RHI::RHIFormat::RGBA8_UNORM)
			.Set_DebugName("ImGui font texture")
			.Build()
		);

		if (nullptr == this->FontTexture)
			return false;

		this->CommandList->Open();

		this->CommandList->BeginTrackingTextureState(this->FontTexture.Get(), RHI::g_AllSubResourceSet, RHI::RHIResourceState::Common);

		this->CommandList->WriteTexture(this->FontTexture.Get(), 0, 0, pixels, width * 4);

		this->CommandList->SetPermanentTextureState(this->FontTexture.Get(), RHI::RHIResourceState::ShaderResource);
		this->CommandList->CommitBarriers();

		this->CommandList->Close();
		this->Device->ExecuteCommandList(this->CommandList.Get());

		io.Fonts->TexID = this->FontTexture.Get();

		return true;
	}

	template<RHI::APITagConcept APITag>
	inline bool UIRHI<APITag>::UpdateGeometry(Imp_CommandList* commandList) {
		ImDrawData* drawData{ ImGui::GetDrawData() };

		// create/resize vertex and index buffers if needed
		if (!this->ReallocateBuffer(this->VertexBuffer, drawData->TotalVtxCount * sizeof(ImDrawVert), (static_cast<Uint64>(drawData->TotalVtxCount) + 5000) * sizeof(ImDrawVert), false))
			return false;

		if (!this->ReallocateBuffer(this->IndexBuffer, drawData->TotalIdxCount * sizeof(ImDrawIdx), (static_cast<Uint64>(drawData->TotalIdxCount) + 5000) * sizeof(ImDrawIdx), true))
			return false;

		this->ImGuiVertexBuffer.resize(this->VertexBuffer->Get_Desc().ByteSize / sizeof(ImDrawVert));
		this->ImGuiIndexBuffer.resize(this->IndexBuffer->Get_Desc().ByteSize / sizeof(ImDrawIdx));

		// copy and convert all vertices into a single contiguous buffer
		//NOTE : Imgui has a little ctype but ImVector set cpp iterator
		for (Uint64 VertexOffset = 0, IndexOffset = 0; const auto & DrawList : drawData->CmdLists) {
			memcpy(this->ImGuiVertexBuffer.data() + VertexOffset, DrawList->VtxBuffer.Data, DrawList->VtxBuffer.Size * sizeof(ImDrawVert));
			memcpy(this->ImGuiIndexBuffer.data() + IndexOffset, DrawList->IdxBuffer.Data, DrawList->IdxBuffer.Size * sizeof(ImDrawIdx));

			VertexOffset += DrawList->VtxBuffer.Size;
			IndexOffset += DrawList->IdxBuffer.Size;
		}

		commandList->WriteBuffer(this->VertexBuffer, this->ImGuiVertexBuffer.data(), this->VertexBuffer->Get_Desc().ByteSize);
		commandList->WriteBuffer(this->IndexBuffer, this->ImGuiIndexBuffer.data(), this->IndexBuffer->Get_Desc().ByteSize);

		return true;
	}

	template<RHI::APITagConcept APITag>
	inline bool UIRHI<APITag>::ReallocateBuffer(RHI::RefCountPtr<Imp_Buffer>& buffer, Uint64 requiredSize, Uint64 reallocateSize, bool isIndexBuffer) {
		if (nullptr == buffer || buffer->Get_Desc().ByteSize < requiredSize) {
			RHI::RHIBufferDesc desc{
				.ByteSize{ reallocateSize },
				.DebugName{ isIndexBuffer ? _W("ImGui index buffer") : _W("ImGui vertex buffer") },
				.IsVertexBuffer{ !isIndexBuffer },
				.IsIndexBuffer{ isIndexBuffer },
				.InitialState{ isIndexBuffer ? RHI::RHIResourceState::IndexBuffer : RHI::RHIResourceState::VertexBuffer },
				.KeepInitialState{ true },
			};

			buffer = this->Device->CreateBuffer(desc);

			if (nullptr == buffer)
				return false;
		}

		return true;
	}



	template<RHI::APITagConcept APITag>
	inline bool UIRHI<APITag>::Render(Imp_FrameBuffer* framebuffer) {
		ImDrawData* drawData{ ImGui::GetDrawData() };
		const auto& io{ ImGui::GetIO() };

		this->CommandList->Open();
		this->CommandList->BeginMarker("ImGUI");

		if (!this->UpdateGeometry(this->CommandList)) {
			this->CommandList->Close();
			return false;
		}

		// handle DPI scaling
		drawData->ScaleClipRects(io.DisplayFramebufferScale);

		Array<float, 2> invDisplaySize{
			1.f / io.DisplaySize.x,
			1.f / io.DisplaySize.y
		};

		// set up graphics state
		RHI::RHIGraphicsStateBuilder<APITag> drawStateBuilder; drawStateBuilder
			.Set_Pipeline(this->GetOrCreatePSO(framebuffer))
			.Set_FrameBuffer(framebuffer)
			.AddViewport(RHI::RHIViewport::Build(io.DisplaySize.x * io.DisplayFramebufferScale.x, io.DisplaySize.y * io.DisplayFramebufferScale.y))
			.AddVertexBuffer(RHI::RHIVertexBufferBinding<APITag>{.Buffer{ this->VertexBuffer }/*, .Slot{ 0 }, .Offset{ 0 }*/ })
			.Set_IndexBuffer(RHI::RHIIndexBufferBinding<APITag>{.Buffer{ this->IndexBuffer }, .Format{ sizeof(ImDrawIdx) == 2 ? RHI::RHIFormat::R16_UINT : RHI::RHIFormat::R32_UINT } /*.Offset{ 0 }*/ });


		// render command lists
		for (Uint32 VertexOffset = 0, IndexOffset = 0; const auto & drawList : drawData->CmdLists) {
			for (const auto& cmd : drawList->CmdBuffer) {
				if (nullptr != cmd.UserCallback)
					cmd.UserCallback(drawList, &cmd);
				else {
					drawStateBuilder
						.AddBindingSet(this->GetOrCreateBindingSet(static_cast<Imp_Texture*>(cmd.TextureId)))
						.AddScissorRect(RHI::RHIRect2D{
							.Offset{.X{ static_cast<Uint32>(cmd.ClipRect.x) }, .Y{ static_cast<Uint32>(cmd.ClipRect.y) } },
							.Extent{.Width {static_cast<Uint32>(cmd.ClipRect.z - cmd.ClipRect.x) }, .Height{ static_cast<Uint32>(cmd.ClipRect.w - cmd.ClipRect.y) } }
							});

					this->CommandList->SetGraphicsState(drawStateBuilder.Build());
					this->CommandList->SetPushConstants(invDisplaySize.data(), static_cast<Uint32>(sizeof(typename decltype(invDisplaySize)::value_type) * invDisplaySize.size()));
					this->CommandList->DrawIndexed(RHI::RHIDrawArguments{ .VertexCount{ cmd.ElemCount }, .StartIndexLocation{ IndexOffset }, .StartVertexLocation{ VertexOffset } });

					drawStateBuilder.SubBindingSet().SubScissorRect();
				}
				IndexOffset += cmd.ElemCount;
			}
			VertexOffset += drawList->VtxBuffer.Size;
		}

		this->CommandList->EndMarker();
		this->CommandList->Close();
		this->Device->ExecuteCommandList(this->CommandList.Get());

		return true;
	}



}