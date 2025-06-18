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

#endif // PARTING_MODULE_BUILD

using CurrentAPI = RHI::D3D12Tag;

using IRenderPass = Parting::IRenderPass<RHI::D3D12Tag>;
using ApplicationBase = Parting::ApplicationBase<RHI::D3D12Tag>;

using DeviceManager = Parting::ManageTypeTraits<CurrentAPI>::DeviceManager;

constexpr const char* g_WindowTitle{ "Parting Engine" };


class ThreadedRendering :public Parting::ApplicationBase<CurrentAPI> {
	using Imp_CommandList = RHI::RHITypeTraits<CurrentAPI>::Imp_CommandList;
	using Imp_Texture = RHI::RHITypeTraits<CurrentAPI>::Imp_Texture;
	using Imp_Buffer = RHI::RHITypeTraits<CurrentAPI>::Imp_Buffer;
	using Imp_FrameBuffer = RHI::RHITypeTraits<CurrentAPI>::Imp_FrameBuffer;

public:
	using ApplicationBase::ApplicationBase;

	~ThreadedRendering(void) = default;

public:
	bool Init(void) {
		Path sceneFileName{ Get_CatallogDirectory().parent_path() / "Media/glTF-Sample-Assets/Models/Sponza/glTF/Sponza.gltf" };
		Path frameworkShaderPath{ Get_CatallogDirectory() / "Shaders/Framework" / RHI::RHITypeTraits<CurrentAPI>::ShaderType };//ShaderType

		this->m_RootFS = MakeShared<RootFileSystem>();
		this->m_RootFS->Mount("/Shaders/Parting", frameworkShaderPath);

		this->m_Executor = MakeUnique<decltype(this->m_Executor)::element_type>();

		this->m_ShaderFactory = MakeShared<decltype(this->m_ShaderFactory)::element_type>(this->m_DeviceManager->Get_Device(), this->m_RootFS, "/Shaders");
		this->m_CommonPasses = MakeShared<decltype(this->m_CommonPasses)::element_type>(this->m_DeviceManager->Get_Device(), this->m_ShaderFactory);
		this->m_BindingCache = MakeUnique<decltype(this->m_BindingCache)::element_type>(this->m_DeviceManager->Get_Device());

		auto nativeFS{ MakeShared<NativeFileSystem>() };
		this->m_TextureCache = MakeShared<decltype(this->m_TextureCache)::element_type>(this->m_DeviceManager->Get_Device(), nativeFS);

		this->m_IsAsyncLoad = false;
		this->BeginLoadingScene(nativeFS, sceneFileName);

		this->m_Scene->FinishedLoading(this->Get_FrameIndex());

		this->m_Camera.LookAt(Math::VecF3{ 0.f, 1.8f, 0.f }, Math::VecF3{ 1.f, 1.8f, 0.f });
		this->m_Camera.Set_MoveSpeed(3.f);

		this->m_CommandList = this->m_DeviceManager->Get_Device()->CreateCommandList();
		for (auto& commandList : this->m_FaceCommandLists)
			commandList = this->m_DeviceManager->Get_Device()->CreateCommandList();

		this->m_ForwardShadingPass = MakeUnique<decltype(this->m_ForwardShadingPass)::element_type>(this->m_DeviceManager->Get_Device(), this->m_CommonPasses);
		decltype(this->m_ForwardShadingPass)::element_type::CreateParameters forwardParams{ .NumConstantBufferVersions{ 128 } };
		this->m_ForwardShadingPass->Init(*this->m_ShaderFactory, forwardParams);

		this->CreateRenderTargets();

		return true;
	}

	void CreateRenderTargets(void) {
		RHI::RHITextureDescBuilder TextureDescBuilder{}; TextureDescBuilder
			.Set_Width(1024)
			.Set_Height(1024)
			.Set_ArraySize(6)
			.Set_Dimension(RHI::RHITextureDimension::TextureCube)
			.Set_ClearValue(Color{ 0.f })
			.Set_IsRenderTarget(true)
			.Set_KeepInitialState(true);

		this->m_ColorBuffer = this->m_DeviceManager->Get_Device()->CreateTexture(TextureDescBuilder
			.Set_DebugName(String{ "ColorBuffer" })
			.Set_Format(RHI::RHIFormat::SRGBA8_UNORM)
			.Set_InitialState(RHI::RHIResourceState::RenderTarget)
			.Build()
		);

		this->m_DepthBuffer = this->m_DeviceManager->Get_Device()->CreateTexture(TextureDescBuilder
			.Set_DebugName(String{ "DepthBuffer" })
			.Set_Format(RHI::RHIFormat::D32)
			.Set_InitialState(RHI::RHIResourceState::DepthWrite)
			.Build()
		);

		this->m_CubemapView.Set_ArrayViewports(1024, 0);

		this->m_FrameBuffer = MakeUnique<decltype(this->m_FrameBuffer)::element_type>(this->m_DeviceManager->Get_Device());
		this->m_FrameBuffer->RenderTargets.assign({ this->m_ColorBuffer });
		this->m_FrameBuffer->DepthStencil = this->m_DepthBuffer;
	}

	void RenderCubeFace(Uint32 face) {
		const Parting::IView* faceView{ this->m_CubemapView.Get_ChildView(Parting::ViewType::PLANAR, face) };

		auto commandList{ this->m_FaceCommandLists[face].Get() };
		commandList->Open();
		commandList->ClearDepthStencilTexture(this->m_DepthBuffer, faceView->Get_Subresources(), 0.f, Optional<Uint8>{ NullOpt });
		commandList->ClearTextureFloat(this->m_ColorBuffer, faceView->Get_Subresources(), Color{ 0.f });

		decltype(this->m_ForwardShadingPass)::element_type::Context context;
		this->m_ForwardShadingPass->PrepareLights(context, commandList, {}, 1.0f, 0.3f, {});//NOTE : No Light

		commandList->SetEnableAutomaticBarriers(false);
		commandList->SetResourceStatesForFramebuffer(this->m_FrameBuffer->Get_FrameBuffer(*faceView));
		commandList->CommitBarriers();

		Parting::InstancedOpaqueDrawStrategy<CurrentAPI> strategy;

		Parting::IGeometryPass<CurrentAPI>::RenderCompositeView(
			commandList,
			faceView,
			faceView,
			*this->m_FrameBuffer,
			this->m_Scene->Get_SceneGraph()->Get_RootNode(),
			strategy,
			*this->m_ForwardShadingPass,
			context
		);

		commandList->SetEnableAutomaticBarriers(true);

		commandList->Close();
	}

private:
	SharedPtr<RootFileSystem> m_RootFS;

	RHI::RefCountPtr<Imp_CommandList> m_CommandList;
	Array<RHI::RefCountPtr<Imp_CommandList>, 6> m_FaceCommandLists;

	bool m_UseThreads{ true };

	UniquePtr<tf::Executor> m_Executor;

	RHI::RefCountPtr<Imp_Texture> m_DepthBuffer;
	RHI::RefCountPtr<Imp_Texture> m_ColorBuffer;
	UniquePtr<Parting::FrameBufferFactory<CurrentAPI>> m_FrameBuffer;

	UniquePtr<Parting::ForwardShadingPass<CurrentAPI>> m_ForwardShadingPass;
	SharedPtr<Parting::ShaderFactory<CurrentAPI>> m_ShaderFactory;
	UniquePtr<Parting::Scene<CurrentAPI>> m_Scene;
	UniquePtr<Parting::BindingCache<CurrentAPI>> m_BindingCache;

	Parting::FirstPersonCamera m_Camera;
	Parting::CubemapView m_CubemapView;

public:
	bool KeyboardUpdate(Int32 key, Int32 scancode, Int32 action, Int32 mods) override {
		this->m_Camera.KeyboardUpdate(key, scancode, action, mods);

		if (GLFW_KEY_SPACE == key && GLFW_PRESS == action)
			this->m_UseThreads = !this->m_UseThreads;

		return true;
	}

	bool MousePosUpdate(double xpos, double ypos) override {
		this->m_Camera.MousePosUpdate(xpos, ypos);

		return true;
	}

	bool MouseButtonUpdate(Int32 button, Int32 action, Int32 mods) override {
		this->m_Camera.MouseButtonUpdate(button, action, mods);

		return true;
	}

public:
	bool LoadScene(SharedPtr<IFileSystem> fs, const Path& sceneFileName) override {
		this->m_Scene = MakeUnique<decltype(this->m_Scene)::element_type>(this->m_DeviceManager->Get_Device(), *this->m_ShaderFactory, fs, this->m_TextureCache);

		if (this->m_Scene->Load(sceneFileName))
			return true;

		return false;
	}

	void Animate(float fElapsedTimeSeconds) override {
		this->m_Camera.Animate(fElapsedTimeSeconds);

		this->m_DeviceManager->Set_InformativeWindowTitle(g_WindowTitle, true, this->m_UseThreads ? "(With threads)" : "(No threads)");//TODO :
	}

	void BackBufferResizing() override {
		this->m_BindingCache->Clear();
	}

	void Render(Imp_FrameBuffer* framebuffer) override {
		Math::AffineF3 viewMatrix{ this->m_Camera.Get_WorldToViewMatrix() };
		this->m_CubemapView.Set_Transform(viewMatrix, 0.1f, 100.f);
		this->m_CubemapView.UpdateCache();

		tf::Taskflow taskFlow;
		if (this->m_UseThreads) {
			for (Uint32 face = 0; face < 6; ++face)
				taskFlow.emplace(
					[this, face](void)
					{ this->RenderCubeFace(face); }
				);

			this->m_Executor->run(taskFlow);
		}
		else {
			for (Uint32 face = 0; face < 6; ++face)
				this->RenderCubeFace(face);
		}

		this->m_CommandList->Open();

		constexpr Array<Pair<Uint32, Uint32>, 6> faceLayout{
			Pair<Uint32, Uint32>{ 3u, 1u },
			Pair<Uint32, Uint32>{ 1u, 1u },
			Pair<Uint32, Uint32>{ 2u, 0u },
			Pair<Uint32, Uint32>{ 2u, 2u },
			Pair<Uint32, Uint32>{ 2u, 1u },
			Pair<Uint32, Uint32>{ 0u, 1u }
		};

		const auto& fbinfo{ framebuffer->Get_Info() };
		Uint32 faceSize{ Math::Min(fbinfo.Width / 4, fbinfo.Height / 3) };

		for (Uint32 face = 0; face < 6; ++face) {
			RHI::RHIViewport viewport;
			viewport.MinX = float(faceLayout[face].first * faceSize);
			viewport.MaxX = viewport.MinX + float(faceSize);
			viewport.MinY = float(faceLayout[face].second * faceSize);
			viewport.MaxY = viewport.MinY + float(faceSize);
			viewport.MinZ = 0.f;
			viewport.MaxZ = 1.f;

			Parting::BLITParameters<CurrentAPI> blitParams{
				.TargetFrameBuffer{ framebuffer },
				.TargetViewport{ viewport },
				.SourceTexture{ this->m_ColorBuffer },
				.SourceArraySlice{ face },
			};
			this->m_CommonPasses->BLITTexture(this->m_CommandList, blitParams, this->m_BindingCache.get());
		}

		this->m_CommandList->Close();

		if (this->m_UseThreads)
			this->m_Executor->wait_for_all();

		Array<Imp_CommandList*, 7> commandLists{
			this->m_FaceCommandLists[0],
			this->m_FaceCommandLists[1],
			this->m_FaceCommandLists[2],
			this->m_FaceCommandLists[3],
			this->m_FaceCommandLists[4],
			this->m_FaceCommandLists[5],
			this->m_CommandList
		};

		this->m_DeviceManager->Get_Device()->ExecuteCommandLists(commandLists.data(), static_cast<Uint32>(commandLists.size()));
	}
};


int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {

	Parting::DeviceCreationParameters deviceParams;

	deviceParams.BackBufferWidth = 1024;
	deviceParams.BackBufferHeight = 768;

	deviceParams.StartFullscreen = false;
	deviceParams.VsyncEnabled = true;
	deviceParams.EnablePerMonitorDPI = true;
	deviceParams.SupportExplicitDisplayScaling = true;

	/*deviceParams.EnableDebugRuntime = true;
	deviceParams.EnableGPUValidation = true;*/

	SharedPtr<DeviceManager> deviceManager{ MakeShared<DeviceManager>() };
	if (false == deviceManager->CreateWindowDeviceAndSwapChain(deviceParams, g_WindowTitle))
		LOG_ERROR("Failed to create device manager");

	{
		ThreadedRendering example{ deviceManager.get() };
		if (example.Init()) {
			deviceManager->AddRenderPassToBack(&example);
			deviceManager->RunMessageLoop();
			deviceManager->RemoveRenderPass(&example);
		}
	}

	deviceManager->Shutdown();

	return 0;//Windows win app must retunr by myself,other will return 120
}