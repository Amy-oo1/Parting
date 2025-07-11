#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"



#else
#pragma once

#include "Core/ModuleBuild.h"

#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"
//Global
#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#include "Core/Algorithm/Module/Algorithm.h"
#include "Core/Concurrent/Module/Concurrent.h"
#include "Core/Container/Module/Container.h"
#include "Core/String/Module/String.h"
//#include "Core/VectorMath/Module/VectorMath.h"
#include "Core/Logger/Module/Logger.h"
#include "Core/VFS/Module/VFS.h"

#include "D3D12RHI/Module/D3D12RHI.h"

#include "Engine/Render/Module/Camera.h"

#include "Engine/Render/Module/DrawStrategy.h"
#include "Engine/Render/Module/ShadowMap.h"
#include "Engine/Render/Module/RenderPass.h"

#include "Engine/Engine/Module/BindingCache.h"
#include "Engine/Engine/Module/ShaderFactory.h"
#include "Engine/Engine/Module/FrameBufferFactory.h"
#include "Engine/Render/Module/GBuffer.h"

#include "Engine/Render/Module/Sence.h"

#include "Engine/Application/Module/Application.h"
#include "Engine/Render/Module/UIRender.h"

#include "Sample/Include/CubeGeometry.h"

#endif // PARTING_MODULE_BUILD

using CurrentAPI = RHI::D3D12Tag;

using IRenderPass = Parting::IRenderPass<CurrentAPI>;
using ApplicationBase = Parting::ApplicationBase<CurrentAPI>;

using DeviceManager = Parting::ManageTypeTraits<CurrentAPI>::DeviceManager;

constexpr const char* g_WindowTitle{ "Parting Engine" };

class RenderTargets : public Parting::GBufferRenderTargets<CurrentAPI> {
	using Imp_Device = typename RHI::RHITypeTraits<CurrentAPI>::Imp_Device;
	using Imp_Texture = typename RHI::RHITypeTraits<CurrentAPI>::Imp_Texture;
public:
	RHI::RefCountPtr<Imp_Texture> ShadedColor;

	void Init(Imp_Device* device, Math::VecU2 size, Uint32 sampleCount, bool enableMotionVectors, bool useReverseProjection) override {
		GBufferRenderTargets<CurrentAPI>::Init(device, size, sampleCount, enableMotionVectors, useReverseProjection);

		this->ShadedColor = device->CreateTexture(RHI::RHITextureDescBuilder{}
			.Set_Width(size.X).Set_Height(size.Y)
			.Set_SampleCount(sampleCount)
			.Set_Format(RHI::RHIFormat::RGBA16_FLOAT)
			.Set_DebugName(String{ "ShadedColor" })
			.Set_IsUAV(true)
			.Set_InitialState(RHI::RHIResourceState::UnorderedAccess)
			.Set_KeepInitialState(true)
			.Build()
		);
	}
};

class SimpleScene {
	using Imp_Device = typename RHI::RHITypeTraits<CurrentAPI>::Imp_Device;
	using Imp_CommandList = typename RHI::RHITypeTraits<CurrentAPI>::Imp_CommandList;
	using Imp_Buffer = typename RHI::RHITypeTraits<CurrentAPI>::Imp_Buffer;
	using Imp_Texture = typename RHI::RHITypeTraits<CurrentAPI>::Imp_Texture;
	using Imp_Sampler = typename RHI::RHITypeTraits<CurrentAPI>::Imp_Sampler;
public:

	bool Init(Imp_Device* device, Imp_CommandList* commandList, Parting::TextureCache<CurrentAPI>* textureCache) {
		commandList->Open();

		this->m_Buffers = MakeShared<decltype(this->m_Buffers)::element_type>();
		this->m_Buffers->IndexBuffer = this->CreateGeometryBuffer(device, commandList, _W("IndexBuffer"), g_Indices, sizeof(g_Indices), false, false);

		Uint64 vertexBufferSize{ 0 };
		this->m_Buffers->Set_VertexBufferRange(RHI::RHIVertexAttribute::Position, RHI::RHIBufferRange{ .Offset{ vertexBufferSize }, .ByteSize{ sizeof(g_Positions) } });
		vertexBufferSize += sizeof(g_Positions);
		this->m_Buffers->Set_VertexBufferRange(RHI::RHIVertexAttribute::TexCoord1, RHI::RHIBufferRange{ .Offset{ vertexBufferSize }, .ByteSize{ sizeof(g_TexCoords) } });
		vertexBufferSize += sizeof(g_TexCoords);
		this->m_Buffers->Set_VertexBufferRange(RHI::RHIVertexAttribute::Normal, RHI::RHIBufferRange{ .Offset{ vertexBufferSize }, .ByteSize{ sizeof(g_Normals) } });
		vertexBufferSize += sizeof(g_Normals);
		this->m_Buffers->Set_VertexBufferRange(RHI::RHIVertexAttribute::Tangent, RHI::RHIBufferRange{ .Offset{ vertexBufferSize }, .ByteSize{ sizeof(g_Tangents) } });
		vertexBufferSize += sizeof(g_Tangents);
		this->m_Buffers->VertexBuffer = this->CreateGeometryBuffer(device, commandList, _W("VertexBuffer"), nullptr, vertexBufferSize, true, false);

		commandList->BeginTrackingBufferState(this->m_Buffers->VertexBuffer, RHI::RHIResourceState::CopyDest);
		commandList->WriteBuffer(this->m_Buffers->VertexBuffer, g_Positions, sizeof(g_Positions), this->m_Buffers->Get_VertexBufferRange(RHI::RHIVertexAttribute::Position).Offset);
		commandList->WriteBuffer(this->m_Buffers->VertexBuffer, g_TexCoords, sizeof(g_TexCoords), this->m_Buffers->Get_VertexBufferRange(RHI::RHIVertexAttribute::TexCoord1).Offset);
		commandList->WriteBuffer(this->m_Buffers->VertexBuffer, g_Normals, sizeof(g_Normals), this->m_Buffers->Get_VertexBufferRange(RHI::RHIVertexAttribute::Normal).Offset);
		commandList->WriteBuffer(this->m_Buffers->VertexBuffer, g_Tangents, sizeof(g_Tangents), this->m_Buffers->Get_VertexBufferRange(RHI::RHIVertexAttribute::Tangent).Offset);
		commandList->SetPermanentBufferState(this->m_Buffers->VertexBuffer, RHI::RHIResourceState::ShaderResource);

		Shader::InstanceData instance{};
		instance.Transform = Math::MatF34{ Math::Transpose(Math::AffineToHomogeneous(Math::AffineF3::Identity())) };
		instance.PrevTransform = instance.Transform;
		this->m_Buffers->InstanceBuffer = CreateGeometryBuffer(device, commandList, _W("VertexBufferTransform"), &instance, sizeof(Shader::InstanceData), false, true);

		Path textureFileName{ ::Get_CatallogDirectory().parent_path() / "media/Amy-Logo.jpg" };

		this->m_Material = MakeShared<decltype(this->m_Material)::element_type>();
		this->m_Material->Name = "CubeMaterial";
		this->m_Material->UseSpecularGlossModel = true;
		this->m_Material->EnableBaseOrDiffuseTexture = true;
		this->m_Material->BaseOrDiffuseTexture = textureCache->LoadTextureFromFile(textureFileName, true, nullptr, commandList);
		this->m_Material->MaterialConstants = this->CreateMaterialConstantBuffer(device, commandList, this->m_Material);

		commandList->Close();
		device->ExecuteCommandList(commandList);

		ASSERT(nullptr != this->m_Material->BaseOrDiffuseTexture->Texture);


		auto geometry{ MakeShared<Parting::MeshGeometry<CurrentAPI>>() };
		geometry->Material = this->m_Material;
		geometry->NumIndices = sizeof(g_Indices) / sizeof(g_Indices[0]);//TODO:
		geometry->NumVertices = sizeof(g_Positions) / sizeof(g_Positions[0]);//TODO( :

		this->m_MeshInfo = MakeShared<decltype(this->m_MeshInfo)::element_type>();
		this->m_MeshInfo->Name = "CubeMesh";
		this->m_MeshInfo->Buffers = m_Buffers;
		this->m_MeshInfo->ObjectSpaceBounds = Math::BoxF3{ Math::VecF3{ -0.5f }, Math::VecF3{ 0.5f } };
		this->m_MeshInfo->TotalIndices = geometry->NumIndices;
		this->m_MeshInfo->TotalVertices = geometry->NumVertices;
		this->m_MeshInfo->Geometries.push_back(geometry);

		this->m_SceneGraph = MakeShared<decltype(this->m_SceneGraph)::element_type>();
		auto node{ MakeShared<Parting::SceneGraphNode<CurrentAPI>>() };
		this->m_SceneGraph->Set_RootNode(node);

		this->m_MeshInstance = MakeShared<decltype(this->m_MeshInstance)::element_type>(this->m_MeshInfo);
		node->Set_Leaf(this->m_MeshInstance);
		node->Set_Name("CubeNode");

		auto sunLight{ MakeShared<Parting::DirectionalLight<CurrentAPI>>() };
		this->m_SceneGraph->AttachLeafNode(node, sunLight);

		sunLight->Set_Direction(Math::VecD3{ 0.1, -1.0, 0.2 });
		sunLight->AngularSize = 0.53f;
		sunLight->Irradiance = 1.f;
		sunLight->Set_Name("Sun");

		this->m_SceneGraph->Refresh(0);

		/*	PrintSceneGraph(this->m_SceneGraph->Get_RootNode());*/
			//TODO :

		return true;
	}


	const SharedPtr<Parting::MeshInstance<CurrentAPI>>& Get_MeshInstance(void) const { return this->m_MeshInstance; }

	const SharedPtr<Parting::SceneGraph<CurrentAPI>>& Get_SceneGraph(void) const { return this->m_SceneGraph; }

	const Vector<SharedPtr<Parting::Light<CurrentAPI>>>& Get_Lights(void) const { return this->m_SceneGraph->Get_Lights(); }

private:
	RHI::RefCountPtr<Imp_Buffer> CreateGeometryBuffer(Imp_Device* device, Imp_CommandList* commandList, const WString& debugName, const void* data, Uint64 dataSize, bool isVertexBuffer, bool isInstanceBuffer) {
		auto bufHandle = device->CreateBuffer(RHI::RHIBufferDescBuilder{}
			.Set_ByteSize(dataSize)
			.Set_StructStride(isInstanceBuffer ? sizeof(Shader::InstanceData) : 0)
			.Set_DebugName(debugName)
			.Set_CanHaveRawViews(isVertexBuffer || isInstanceBuffer)
			.Set_IsIndexBuffer(!isVertexBuffer && !isInstanceBuffer)
			.Set_InitialState(RHI::RHIResourceState::CopyDest)
			.Build()
		);

		if (nullptr != data) {
			commandList->BeginTrackingBufferState(bufHandle, RHI::RHIResourceState::CopyDest);
			commandList->WriteBuffer(bufHandle, data, dataSize);
			commandList->SetPermanentBufferState(bufHandle, (isVertexBuffer || isInstanceBuffer) ? RHI::RHIResourceState::ShaderResource : RHI::RHIResourceState::IndexBuffer);
		}

		return bufHandle;
	}

	RHI::RefCountPtr<Imp_Buffer> CreateMaterialConstantBuffer(Imp_Device* device, Imp_CommandList* commandList, const SharedPtr<Parting::Material<CurrentAPI>> material) {
		auto buffer{ device->CreateBuffer(RHI::RHIBufferDescBuilder{}
			.Set_ByteSize(sizeof(Shader::MaterialConstants))
			.Set_DebugName(_W("Material_Constants"))
			.Set_IsConstantBuffer(true)
			.Set_InitialState(RHI::RHIResourceState::ConstantBuffer)
			.Set_KeepInitialState(true)
			.Build()

		) };

		Shader::MaterialConstants constants;
		material->FillConstantBuffer(constants);
		commandList->WriteBuffer(buffer, &constants, sizeof(Shader::MaterialConstants));

		return buffer;
	}

private:
	SharedPtr<Parting::BufferGroup<CurrentAPI>> m_Buffers;
	SharedPtr<Parting::Material<CurrentAPI>> m_Material;
	SharedPtr<Parting::MeshInfo<CurrentAPI>> m_MeshInfo;
	SharedPtr<Parting::MeshInstance<CurrentAPI>> m_MeshInstance;
	SharedPtr<Parting::SceneGraph<CurrentAPI>> m_SceneGraph;
};


class DeferredShading : public Parting::IRenderPass<CurrentAPI> {
	using Imp_Device = typename RHI::RHITypeTraits<CurrentAPI>::Imp_Device;
	using Imp_CommandList = typename RHI::RHITypeTraits<CurrentAPI>::Imp_CommandList;
	using Imp_FrameBuffer = typename RHI::RHITypeTraits<CurrentAPI>::Imp_FrameBuffer;
public:
	using IRenderPass::IRenderPass;

private:
	SharedPtr<Parting::ShaderFactory<CurrentAPI>> m_ShaderFactory;
	SharedPtr<Parting::TextureCache<CurrentAPI>> m_TextureCache;
	SharedPtr<Parting::CommonRenderPasses<CurrentAPI>> m_CommonPasses;
	SharedPtr<Parting::BindingCache<CurrentAPI>> m_BindingCache;

	SharedPtr<RenderTargets> m_RenderTargets;
	UniquePtr<Parting::GBufferFillPass<CurrentAPI>> m_GBufferPass;
	UniquePtr<Parting::DeferredLightingPass<CurrentAPI>> m_DeferredLightingPass;

	Parting::PlanarView m_View;

	SimpleScene m_Scene;

	RHI::RefCountPtr<Imp_CommandList> m_CommandList;
	float m_Rotation{ 0.f };

public:
	bool Init(void) {
		auto nativeFS{ MakeShared<NativeFileSystem>() };

		Path frameworkShaderPath{ Get_CatallogDirectory() / "Shaders/Framework" / RHI::RHITypeTraits<CurrentAPI>::ShaderType };

		auto rootFS{ MakeShared<RootFileSystem>() };
		rootFS->Mount("/Shaders/Parting", frameworkShaderPath);
		this->m_ShaderFactory = MakeShared<decltype(this->m_ShaderFactory)::element_type>(this->m_DeviceManager->Get_Device(), rootFS, "/Shaders");
		this->m_CommonPasses = MakeShared<decltype(this->m_CommonPasses)::element_type>(this->m_DeviceManager->Get_Device(), this->m_ShaderFactory);
		this->m_BindingCache = MakeShared<decltype(this->m_BindingCache)::element_type>(this->m_DeviceManager->Get_Device());

		this->m_DeferredLightingPass = MakeUnique<decltype(this->m_DeferredLightingPass)::element_type>(this->m_DeviceManager->Get_Device(), this->m_CommonPasses);
		this->m_DeferredLightingPass->Init(this->m_ShaderFactory);

		this->m_TextureCache = MakeUnique<decltype(this->m_TextureCache)::element_type>(this->m_DeviceManager->Get_Device(), nativeFS);

		this->m_CommandList = this->m_DeviceManager->Get_Device()->CreateCommandList();

		return this->m_Scene.Init(this->m_DeviceManager->Get_Device(), this->m_CommandList, this->m_TextureCache.get());
	}


private:
	void SetupView(void) {
		Math::VecF2 renderTargetSize{ Math::VecF2{ this->m_RenderTargets->Get_Size() } };

		Math::AffineF3 viewMatrix{ Math::YawPitchRoll(this->m_Rotation, 0.f, 0.f) * Math::YawPitchRoll(0.f, Math::Radians(-30.f), 0.f) * Math::Translation(Math::VecF3{0, 0, 2}) };

		Math::MatF44 projection{ Math::PerspProjD3DStyle(Math::Radians(60.f), renderTargetSize.X / renderTargetSize.Y, 0.1f, 10.f) };

		m_View.Set_Viewport(RHI::RHIViewport::Build(renderTargetSize.X, renderTargetSize.Y));
		m_View.Set_Matrices(viewMatrix, projection);
		m_View.UpdateCache();
	}

public:
	void Animate(float seconds) override {
		m_Rotation += seconds * 1.1f;
		this->m_DeviceManager->Set_InformativeWindowTitle(g_WindowTitle);
	}

	void Render(Imp_FrameBuffer* framebuffer) override {
		const auto& fbinfo{ framebuffer->Get_Info() };

		Math::VecU2 size{ fbinfo.Width, fbinfo.Height };

		if (nullptr == this->m_RenderTargets || Math::Any(this->m_RenderTargets->Get_Size() != size)) {
			this->m_RenderTargets = nullptr;
			this->m_BindingCache->Clear();
			this->m_DeferredLightingPass->ResetBindingCache();

			this->m_GBufferPass.reset();

			this->m_RenderTargets = MakeShared<decltype(this->m_RenderTargets)::element_type>();
			this->m_RenderTargets->Init(this->m_DeviceManager->Get_Device(), size, 1, false, false);
		}

		this->SetupView();

		if (nullptr == this->m_GBufferPass) {
			decltype(this->m_GBufferPass)::element_type::CreateParameters GBufferParams;
			this->m_GBufferPass = MakeUnique<decltype(this->m_GBufferPass)::element_type>(this->m_DeviceManager->Get_Device(), this->m_CommonPasses);
			this->m_GBufferPass->Init(*this->m_ShaderFactory, GBufferParams);
		}

		this->m_CommandList->Open();

		this->m_RenderTargets->Clear(this->m_CommandList);

		Parting::DrawItem<CurrentAPI> drawItem{};
		drawItem.Instance = m_Scene.Get_MeshInstance().get();
		drawItem.Mesh = drawItem.Instance->Get_Mesh().get();
		drawItem.Geometry = drawItem.Mesh->Geometries[0].get();
		drawItem.Material = drawItem.Geometry->Material.get();
		drawItem.Buffers = drawItem.Mesh->Buffers.get();
		drawItem.DistanceToCamera = 0;
		drawItem.CullMode = RHI::RHIRasterCullMode::Back;

		Parting::PassthroughDrawStrategy<CurrentAPI> drawStrategy;
		drawStrategy.Set_Data(&drawItem, 1);

		decltype(this->m_GBufferPass)::element_type::Context context;

		Parting::IGeometryPass<CurrentAPI>::RenderView(
			this->m_CommandList,
			&this->m_View,
			&this->m_View,
			this->m_RenderTargets->GBufferFrameBuffer->Get_FrameBuffer(this->m_View),
			drawStrategy,
			*this->m_GBufferPass,
			context,
			false
		);

		decltype(this->m_DeferredLightingPass)::element_type::Inputs deferredInputs;
		deferredInputs.Set_GBuffer(*this->m_RenderTargets);
		deferredInputs.AmbientColorTop = 0.2f;
		deferredInputs.AmbientColorBottom = deferredInputs.AmbientColorTop * Math::VecF3{ 0.3f, 0.4f, 0.3f };
		deferredInputs.Lights = &this->m_Scene.Get_Lights();
		deferredInputs.Output = this->m_RenderTargets->ShadedColor;

		this->m_DeferredLightingPass->Render(this->m_CommandList, this->m_View, deferredInputs);

		this->m_CommonPasses->BLITTexture(this->m_CommandList, framebuffer, this->m_RenderTargets->ShadedColor, this->m_BindingCache.get());

		this->m_CommandList->Close();
		this->m_DeviceManager->Get_Device()->ExecuteCommandList(this->m_CommandList);
	}

};

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {

	Parting::DeviceCreationParameters deviceParams;

	/*deviceParams.BackBufferWidth = 1920;
	deviceParams.BackBufferHeight = 1080;*/

	deviceParams.StartFullscreen = false;
	deviceParams.VsyncEnabled = true;
	deviceParams.EnablePerMonitorDPI = true;
	deviceParams.SupportExplicitDisplayScaling = true;

	/*deviceParams.EnableDebugRuntime = true;
	deviceParams.EnableGPUValidation = true;*/

	auto deviceManager{ MakeUnique<DeviceManager>() };
	if (false == deviceManager->CreateWindowDeviceAndSwapChain(deviceParams, g_WindowTitle))
		LOG_ERROR("Failed to create device manager");

	{
		DeferredShading example{ deviceManager.get() };
		if (example.Init()) {
			deviceManager->AddRenderPassToBack(&example);
			deviceManager->RunMessageLoop();
			deviceManager->RemoveRenderPass(&example);
		}
	}

	deviceManager->Shutdown();

	return 0;//Windows win app must retunr by myself,other will return 120
}