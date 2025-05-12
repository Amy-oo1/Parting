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

#include "Engine/Render/Module/DrawStrategy.h"
#include "Engine/Render/Module/ShadowMap.h"

#include "Engine/Engine/Module/BindingCache.h"
#include "Engine/Engine/Module/DescriptorTableManager.h"
#include "Engine/Engine/Module/ShaderFactory.h"

#include "Engine/Application/Module/Application.h"

#endif // PARTING_MODULE_BUILD

using CurrentAPI = RHI::D3D12Tag;

using IRenderPass = Parting::IRenderPass<RHI::D3D12Tag>;
using ApplicationBase = Parting::ApplicationBase<RHI::D3D12Tag>;

using DeviceManager = Parting::ManageTypeTraits<CurrentAPI>::DeviceManager;

struct SmaplerUIData final {
	bool								ShowUI{ true };
	bool								ShowConsole{ false };
	bool								UseDeferredShading{ true };
	bool								Stereo{ false };
	bool								EnableSsao{ true };

};



class FeatureSample :public Parting::ApplicationBase<CurrentAPI> {
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




	}



private:
	SmaplerUIData& m_UIData;
	Parting::BindingCache<CurrentAPI>				m_BindingCache;

	SharedPtr<RootFileSystem>						m_RootFs;
	SharedPtr<NativeFileSystem>						m_NativeFs;

	Path											m_SceneDir;
	Vector<String>									m_SceneFilesAvailable;
	SharedPtr<Parting::ShaderFactory<CurrentAPI>>	m_ShaderFactory;

	SharedPtr<Parting::InstancedOpaqueDrawStrategy> m_OpaqueDrawStrategy;
	SharedPtr<Parting::TransparentDrawStrategy> m_TransparentDrawStrategy;

	SharedPtr<Parting::CascadedShadowMap<CurrentAPI>>  m_ShadowMap;

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