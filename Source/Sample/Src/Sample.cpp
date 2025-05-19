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

#include "Engine/Render/Module/Sence.h"

#include "Engine/Application/Module/Application.h"

#endif // PARTING_MODULE_BUILD

using CurrentAPI = RHI::D3D12Tag;

using IRenderPass = Parting::IRenderPass<RHI::D3D12Tag>;
using ApplicationBase = Parting::ApplicationBase<RHI::D3D12Tag>;

using DeviceManager = Parting::ManageTypeTraits<CurrentAPI>::DeviceManager;

struct SmaplerUIData final {
	SharedPtr<Parting::SceneCamera<CurrentAPI>>	ActiveSceneCamera;


	bool										ShowUI{ true };
	bool										ShowConsole{ false };
	bool										UseDeferredShading{ true };
	bool										Stereo{ false };
	bool										EnableSsao{ true };
	bool										UseThirdPersonCamera{ false };

};



class FeatureSample :public Parting::ApplicationBase<CurrentAPI> {
	using Imp_CommandList = RHI::RHITypeTraits<CurrentAPI>::Imp_CommandList;
	using Imp_Texture = RHI::RHITypeTraits<CurrentAPI>::Imp_Texture;
public:
	FeatureSample(::DeviceManager* deviceManager, SmaplerUIData& ui, const String& sceneName) :
		ApplicationBase{ deviceManager },
		m_UIData{ ui },
		m_BindingCache{ deviceManager->Get_Device() },
		m_RootFs{ MakeShared<RootFileSystem>() },
		m_NativeFs{ MakeShared<NativeFileSystem>() } {

		auto MediaPir{ Get_CatallogDirectory().parent_path() / "Media" };
		auto FrameworkShaderDir{ Get_CatallogDirectory() / "Shaders/Framework" / RHI::RHITypeTraits<CurrentAPI>::ShaderType };//ShaderType

		this->m_RootFs->Mount("/Media", MediaPir);
		this->m_RootFs->Mount("/Shaders/Parting", FrameworkShaderDir);

		this->m_SceneDir = MediaPir / "glTF-Sample-Assets/Models/";
		this->m_SceneFilesAvailable = this->FindScenes(*this->m_NativeFs, this->m_SceneDir);

		if (sceneName.empty() && this->m_SceneFilesAvailable.empty())
			LOG_ERROR("No scene files found in " /*this->m_SceneDir.c_str()*/);

		this->m_TextureCache = MakeShared<Parting::TextureCache<CurrentAPI>>(this->m_DeviceManager->Get_Device(), this->m_NativeFs, nullptr);

		this->m_ShaderFactory = MakeShared<Parting::ShaderFactory<CurrentAPI>>(this->m_DeviceManager->Get_Device(), this->m_RootFs, "/Shaders");
		this->m_CommonPasses = MakeShared<Parting::CommonRenderPasses<CurrentAPI>>(this->m_DeviceManager->Get_Device(), this->m_ShaderFactory);

		this->m_OpaqueDrawStrategy = MakeShared<Parting::InstancedOpaqueDrawStrategy>();
		this->m_TransparentDrawStrategy = MakeShared<Parting::TransparentDrawStrategy>();

		constexpr Array<RHI::RHIFormat, 4> shadowMapFormats{
			RHI::RHIFormat::D24S8,
			RHI::RHIFormat::D32,
			RHI::RHIFormat::D16,
			RHI::RHIFormat::D32S8
		};

		constexpr RHI::RHIFormatSupport shadowMapFeatures{
			RHI::RHIFormatSupport::Texture |
			RHI::RHIFormatSupport::DepthStencil |
			RHI::RHIFormatSupport::ShaderLoad
		};

		auto shadowMapFormat{ this->m_DeviceManager->Get_Device()->ChooseFormat(shadowMapFeatures, shadowMapFormats.data(), static_cast<Uint32>(shadowMapFormats.size())) };


		this->m_ShadowMap = std::make_shared<Parting::CascadedShadowMap<CurrentAPI>>(this->m_DeviceManager->Get_Device(), 2048, 4, 0, shadowMapFormat);
		this->m_ShadowMap->SetupProxyView();

		this->m_ShadowFramebuffer = MakeShared<Parting::FrameBufferFactory<CurrentAPI>>(this->m_DeviceManager->Get_Device());
		this->m_ShadowFramebuffer->DepthStencil = this->m_ShadowMap->Get_Texture();

		this->m_ShadowDepthPass = MakeShared<Parting::DepthPass<CurrentAPI>>(this->m_DeviceManager->Get_Device(), this->m_CommonPasses);
		this->m_ShadowDepthPass->DeferInit(*this->m_ShaderFactory, Parting::DepthPass<CurrentAPI>::CreateParameters{.DepthBias{ 100 }, .SlopeScaledDepthBias{ 4.f } });

		this->m_CommandList = this->m_DeviceManager->Get_Device()->CreateCommandList();

		this->m_FirstPersonCamera.Set_MoveSpeed(3.f);
		this->m_ThirdPersonCamera.Set_MoveSpeed(3.f);

		this->Set_AsyncLoad(false);
		/*this->Set_AsyncLoad(true);*/

		if (sceneName.empty())
			this->SetCurrentScene(ApplicationBase<CurrentAPI>::FindPreferredScene(this->m_SceneFilesAvailable, "Sponza.gltf"));
		else
			this->SetCurrentScene(sceneName);

		this->CreateLightProbes(4);
	}


public:
	bool LoadScene(SharedPtr<IFileSystem> fs, const Path& sceneFileName) override {
		//TODO :add Time Show

		this->m_Scene = MakeUnique<Parting::Scene<CurrentAPI>>(this->m_DeviceManager->Get_Device(), *this->m_ShaderFactory, fs, this->m_TextureCache, nullptr, nullptr);

		//TODO add time cast info

		return this->m_Scene->Load(sceneFileName);
	}

	virtual void SceneLoaded(void) override {
		this->Parting::ApplicationBase<CurrentAPI>::SceneLoaded();

		this->m_Scene->FinishedLoading(this->Get_FrameIndex());

		this->m_WallclockTime = 0.f;
		this->m_PreviousViewsValid = false;

		for (const auto& Light : this->m_Scene->Get_SceneGraph()->Get_Lights())
			if (LightType_Directional == Light->Get_LightType()) {
				this->m_SunLight = StaticPointerCast<Parting::DirectionalLight<CurrentAPI>>(Light);
				if (this->m_SunLight->Irradiance <= 0.f)
					this->m_SunLight->Irradiance = 1.f;
				break;
			}

		if (nullptr == this->m_SunLight) {
			m_SunLight = MakeShared<Parting::DirectionalLight<CurrentAPI>>();
			m_SunLight->AngularSize = 0.53f;
			m_SunLight->Irradiance = 1.f;

			auto node{ MakeShared<Parting::SceneGraphNode<CurrentAPI>>() };
			node->Set_Leaf(this->m_SunLight);
			this->m_SunLight->Set_Direction(Math::VecD3{ 0.1, -0.9, 0.1 });
			this->m_SunLight->Set_Name("Sun");
			this->m_Scene->Get_SceneGraph()->Attach(this->m_Scene->Get_SceneGraph()->Get_RootNode(), node);
		}

		auto cameras{ this->m_Scene->Get_SceneGraph()->Get_Cameras() };
		if (!cameras.empty())
			this->m_UIData.ActiveSceneCamera = cameras[0];
		else {
			this->m_UIData.ActiveSceneCamera.reset();

			this->m_FirstPersonCamera.LookAt(
				Math::VecF3{ 0.f, 1.8f, 0.f },
				Math::VecF3{ 1.f, 1.8f, 0.f }
			);
			this->m_CameraVerticalFov = 60.f;
		}

		this->m_ThirdPersonCamera.Set_Rotation(Math::Radians(135.f), Math::Radians(20.f));
		this->PointThirdPersonCameraAt(this->m_Scene->Get_SceneGraph()->Get_RootNode());
		this->m_UIData.UseThirdPersonCamera = true;

		this->CopyActiveCameraToFirstPerson();

		//TODO :PrintSceneGraph

	}

	void SetCurrentScene(const String& sceneName) {
		if (sceneName == this->m_CurrentSceneName)
			return;

		this->BeginLoadingScene(this->m_NativeFs, this->m_CurrentSceneName = sceneName);
	}



	void PointThirdPersonCameraAt(const SharedPtr<Parting::SceneGraphNode<CurrentAPI>>& node) {
		Math::BoxF3 bounds{ node->Get_GlobalBoundingBox() };
		this->m_ThirdPersonCamera.Set_TargetPosition(bounds.Get_Center());

		float radius{ Math::Length(bounds.Diagonal()) * 0.5f };
		float distance{ radius / Math::Sin(Math::Radians(this->m_CameraVerticalFov * 0.5f)) };

		this->m_ThirdPersonCamera.Set_Distance(distance);
		this->m_ThirdPersonCamera.Animate(0.f);
	}

	void CopyActiveCameraToFirstPerson(void) {
		if (nullptr != this->m_UIData.ActiveSceneCamera) {
			Math::AffineF3 viewToWorld{ this->m_UIData.ActiveSceneCamera->Get_ViewToWorldMatrix() };
			Math::VecF3 cameraPos{ viewToWorld.m_Translation };

			this->m_FirstPersonCamera.LookAt(cameraPos, cameraPos + viewToWorld.m_Linear.Row2, viewToWorld.m_Linear.Row1);
		}
		else if (this->m_UIData.UseThirdPersonCamera)
			this->m_FirstPersonCamera.LookAt(
				this->m_ThirdPersonCamera.Get_Position(),
				this->m_ThirdPersonCamera.Get_Position() + this->m_ThirdPersonCamera.Get_Dir(),
				this->m_ThirdPersonCamera.Get_Up()
			);
	}

	void CreateLightProbes(Uint32 numProbes) {
		auto device{ this->m_DeviceManager->Get_Device() };

		RHI::RHITextureDescBuilder cubemapDescBuilder{};
		cubemapDescBuilder
			.Set_ArraySize(6 * numProbes)
			.Set_Format(RHI::RHIFormat::RGBA16_FLOAT)
			.Set_Dimension(RHI::RHITextureDimension::TextureCubeArray)
			.Set_IsRenderTarget(true)
			.Set_InitialState(RHI::RHIResourceState::ShaderResource)
			.Set_KeepInitialState(true);

		constexpr Uint32 diffuseMapSize = { 256 };
		constexpr Uint32 diffuseMapMipLevels{ 1 };

		constexpr Uint32 specularMapSize{ 512 };
		constexpr Uint32 specularMapMipLevels{ 8 };


		this->m_LightProbeDiffuseTexture = device->CreateTexture(cubemapDescBuilder
			.Set_Width(diffuseMapSize).Set_Height(diffuseMapSize)
			.Set_MipLevels(diffuseMapMipLevels)
			.Build()
		);


		this->m_LightProbeSpecularTexture = device->CreateTexture(cubemapDescBuilder
			.Set_Width(specularMapSize).Set_Height(specularMapSize)
			.Set_MipLevels(specularMapMipLevels)
			.Build()
		);

		this->m_LightProbes.clear();

		for (Uint32 Index = 0; Index < numProbes; ++Index) {
			SharedPtr<Parting::LightProbe<CurrentAPI>> probe = MakeShared<Parting::LightProbe<CurrentAPI>>();

			probe->Name = IntegralToString(Index + 1);
			probe->DiffuseMap = m_LightProbeDiffuseTexture;
			probe->SpecularMap = m_LightProbeSpecularTexture;
			probe->DiffuseArrayIndex = Index;
			probe->SpecularArrayIndex = Index;
			probe->Bounds = Math::Frustum::Empty();
			probe->Enabled = false;

			this->m_LightProbes.push_back(probe);
		}
	}


private:
	SmaplerUIData& m_UIData;
	Parting::BindingCache<CurrentAPI>					m_BindingCache;

	SharedPtr<RootFileSystem>							m_RootFs;
	SharedPtr<NativeFileSystem>							m_NativeFs;

	Path												m_SceneDir;
	Vector<String>										m_SceneFilesAvailable;
	SharedPtr<Parting::ShaderFactory<CurrentAPI>>		m_ShaderFactory;


	SharedPtr<Parting::InstancedOpaqueDrawStrategy>		m_OpaqueDrawStrategy;
	SharedPtr<Parting::TransparentDrawStrategy>			m_TransparentDrawStrategy;

	SharedPtr<Parting::CascadedShadowMap<CurrentAPI>>	m_ShadowMap;
	SharedPtr<Parting::FrameBufferFactory<CurrentAPI>>	m_ShadowFramebuffer;
	SharedPtr<Parting::DepthPass<CurrentAPI>>			m_ShadowDepthPass;

	RHI::RefCountPtr<Imp_CommandList> 					m_CommandList;

	bool												m_PreviousViewsValid{ false };
	Parting::FirstPersonCamera							m_FirstPersonCamera;
	Parting::ThirdPersonCamera							m_ThirdPersonCamera;

	SharedPtr<Parting::Scene<CurrentAPI>>				m_Scene;
	String												m_CurrentSceneName;
	SharedPtr<Parting::DirectionalLight<CurrentAPI>>	m_SunLight;

	Vector<SharedPtr<Parting::LightProbe<CurrentAPI>>>	m_LightProbes;
	RHI::RefCountPtr<Imp_Texture>						m_LightProbeDiffuseTexture;
	RHI::RefCountPtr<Imp_Texture>						m_LightProbeSpecularTexture;


	float												m_WallclockTime{ 0.f };

	float												m_CameraVerticalFov{ 60.f };

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

	String Title{ "Parting Engine" };

	DeviceManager deviceManager{};
	if (false == deviceManager.CreateWindowDeviceAndSwapChain(deviceParams, Title.c_str()))
		LOG_ERROR("Failed to create device manager");

	{
		SmaplerUIData UIData;
		FeatureSample sample{ &deviceManager, UIData, "" };


	}


	deviceManager.Shutdown();

	return 0;//Windows win app must retunr by myself,other will return 120
}