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
#include "Engine/Engine/Module/DescriptorTableManager.h"
#include "Engine/Engine/Module/ShaderFactory.h"
#include "Engine/Engine/Module/FrameBufferFactory.h"
#include "Engine/Render/Module/GBuffer.h"

#include "Engine/Render/Module/Sence.h"

#include "Engine/Application/Module/Application.h"
#include "Engine/Render/Module/UIRender.h"

#endif // PARTING_MODULE_BUILD

using CurrentAPI = RHI::D3D12Tag;

using IRenderPass = Parting::IRenderPass<RHI::D3D12Tag>;
using ApplicationBase = Parting::ApplicationBase<RHI::D3D12Tag>;

using DeviceManager = Parting::ManageTypeTraits<CurrentAPI>::DeviceManager;

constexpr const char* g_WindowTitle{ "Parting Engine" };

class BasicTriangle : public Parting::IRenderPass<CurrentAPI> {
	using Imp_Shader = typename RHI::RHITypeTraits<CurrentAPI>::Imp_Shader;
	using Imp_CommandList = typename RHI::RHITypeTraits<CurrentAPI>::Imp_CommandList;
	using Imp_GraphicsPipeline = typename RHI::RHITypeTraits<CurrentAPI>::Imp_GraphicsPipeline;
	using Imp_FrameBuffer = typename RHI::RHITypeTraits<CurrentAPI>::Imp_FrameBuffer;
private:
	RHI::RefCountPtr<Imp_Shader> m_VertexShader;
	RHI::RefCountPtr<Imp_Shader> m_PixelShader;
	RHI::RefCountPtr<Imp_GraphicsPipeline> m_Pipeline;
	RHI::RefCountPtr<Imp_CommandList> m_CommandList;

public:
	using IRenderPass::IRenderPass;

	bool Init(void) {
		Path appShaderPath{ ::Get_CatallogDirectory() / "Shaders/Triangle" };

		auto nativeFS{ MakeShared<NativeFileSystem>() };
		Parting::ShaderFactory<CurrentAPI> shaderFactory(this->m_DeviceManager->Get_Device(), nativeFS, appShaderPath);

		this->m_VertexShader = shaderFactory.CreateShader("shaders.hlsl", "main_vs", nullptr, RHI::RHIShaderType::Vertex);
		this->m_PixelShader = shaderFactory.CreateShader("shaders.hlsl", "main_ps", nullptr, RHI::RHIShaderType::Pixel);

		if (nullptr == this->m_VertexShader || nullptr == this->m_PixelShader)
			return false;

		this->m_CommandList = this->m_DeviceManager->Get_Device()->CreateCommandList();

		return true;
	}

	void BackBufferResizing(void) override { m_Pipeline = nullptr; }

	void Animate(float fElapsedTimeSeconds) override {
		this->m_DeviceManager->Set_InformativeWindowTitle(g_WindowTitle);
	}

	void Render(Imp_FrameBuffer* framebuffer) override {
		if (nullptr == this->m_Pipeline)
			this->m_Pipeline = this->m_DeviceManager->Get_Device()->CreateGraphicsPipeline(RHI::RHIGraphicsPipelineDescBuilder<CurrentAPI>{}
		.Set_VS(this->m_VertexShader)
			.Set_PS(this->m_PixelShader)
			.Set_PrimType(RHI::RHIPrimitiveType::TriangleList)
			.Set_DepthTestEnable(false)
			.Build(),
			framebuffer
			);

		this->m_CommandList->Open();

		const auto& Attachment{ framebuffer->Get_Desc().ColorAttachments[0] };

		if (nullptr != Attachment.Texture)
			this->m_CommandList->ClearTextureFloat(Attachment.Texture, Attachment.Subresources, Color{ 0.f });

		this->m_CommandList->SetGraphicsState(RHI::RHIGraphicsStateBuilder<CurrentAPI>{}
		.Set_Pipeline(this->m_Pipeline)
			.Set_FrameBuffer(framebuffer)
			.AddViewportAndScissorRect(RHI::RHIViewport::Build(static_cast<float>(framebuffer->Get_Info().Width), static_cast<float>(framebuffer->Get_Info().Height)))
			.Build()
			);
		this->m_CommandList->Draw(RHI::RHIDrawArguments{ .VertexCount{ 3 } });

		this->m_CommandList->Close();
		this->m_DeviceManager->Get_Device()->ExecuteCommandList(this->m_CommandList);
	}

};




int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {

	Parting::DeviceCreationParameters deviceParams;

	deviceParams.BackBufferWidth = 1920;
	deviceParams.BackBufferHeight = 1080;
	//deviceParams.StartFullscreen = false;
	deviceParams.VsyncEnabled = true;
	deviceParams.EnablePerMonitorDPI = true;
	deviceParams.SupportExplicitDisplayScaling = true;
	deviceParams.EnableDebugRuntime = true;
	deviceParams.EnableGPUValidation = true;

	SharedPtr<DeviceManager> deviceManager{ MakeShared<DeviceManager>() };
	if (false == deviceManager->CreateWindowDeviceAndSwapChain(deviceParams, g_WindowTitle))
		LOG_ERROR("Failed to create device manager");

	{
		BasicTriangle example{ deviceManager.get() };
		if (example.Init()) {
			deviceManager->AddRenderPassToBack(&example);
			deviceManager->RunMessageLoop();
			deviceManager->RemoveRenderPass(&example);
		}
	}

	deviceManager->Shutdown();

	return 0;//Windows win app must retunr by myself,other will return 120
}

