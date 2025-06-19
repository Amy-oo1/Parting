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


class RenderTargets final : public Parting::GBufferRenderTargets<CurrentAPI> {
	using Imp_Device = RHI::RHITypeTraits<CurrentAPI>::Imp_Device;
	using Imp_CommandList = RHI::RHITypeTraits<CurrentAPI>::Imp_CommandList;
	using Imp_Texture = RHI::RHITypeTraits<CurrentAPI>::Imp_Texture;
	using Imp_Heap = RHI::RHITypeTraits<CurrentAPI>::Imp_Heap;
public:
	RHI::RefCountPtr<Imp_Texture> HDRColor;
	RHI::RefCountPtr<Imp_Texture> LDRColor;
	RHI::RefCountPtr<Imp_Texture> MaterialIDs;
	RHI::RefCountPtr<Imp_Texture> ResolvedColor;
	RHI::RefCountPtr<Imp_Texture> TemporalFeedback1;
	RHI::RefCountPtr<Imp_Texture> TemporalFeedback2;
	RHI::RefCountPtr<Imp_Texture> AmbientOcclusion;

	RHI::RefCountPtr<Imp_Heap> Heap;

	SharedPtr<Parting::FrameBufferFactory<CurrentAPI>> ForwardFrameBuffer;
	SharedPtr<Parting::FrameBufferFactory<CurrentAPI>> HDRFrameBuffer;
	SharedPtr<Parting::FrameBufferFactory<CurrentAPI>> LDRFrameBuffer;
	SharedPtr<Parting::FrameBufferFactory<CurrentAPI>> ResolvedFrameBuffer;
	SharedPtr<Parting::FrameBufferFactory<CurrentAPI>> MaterialIDFrameBuffer;


	STDNODISCARD bool Is_UpdateRequired(Math::VecU2 size, Uint32 sampleCount) const {
		if (Math::Any(this->m_Size != size) || this->m_SampleCount != sampleCount)
			return true;

		return false;
	}


	virtual void Init(
		Imp_Device* device,
		Math::VecU2 size,
		Uint32 sampleCount,
		bool enableMotionVectors,
		bool useReverseProjection
	) override {
		this->GBufferRenderTargets<CurrentAPI>::Init(device, size, sampleCount, enableMotionVectors, useReverseProjection);

		RHI::RHITextureDescBuilder textureBuilder{}; textureBuilder
			.Set_Width(size.X).Set_Height(size.Y)
			.Set_SampleCount(sampleCount)
			.Set_Dimension(sampleCount > 1 ? RHI::RHITextureDimension::Texture2DMS : RHI::RHITextureDimension::Texture2D)
			.Set_IsRenderTarget(true)
			.Set_IsVirtual(device->QueryFeatureSupport(RHI::RHIFeature::VirtualResources))
			.Set_ClearValue(Color{ 0.f })
			.Set_InitialState(RHI::RHIResourceState::RenderTarget)
			.Set_KeepInitialState(true);

		this->HDRColor = device->CreateTexture(textureBuilder
			.Set_Format(RHI::RHIFormat::RGBA16_FLOAT)
			.Set_DebugName("HdrColor")
			.Set_IsUAV(1 == sampleCount)
			.Build()
		);

		this->MaterialIDs = device->CreateTexture(textureBuilder
			.Set_Format(RHI::RHIFormat::RG16_UINT)
			.Set_DebugName("MaterialIDs")
			.Set_IsUAV(false)
			.Build()
		);

		// The render targets below this point are non-MSAA
		textureBuilder.Set_SampleCount(1).Set_Dimension(RHI::RHITextureDimension::Texture2D);

		this->ResolvedColor = device->CreateTexture(textureBuilder
			.Set_Format(RHI::RHIFormat::RGBA16_FLOAT)
			.Set_DebugName("ResolvedColor")
			.Set_MipLevels(Parting::GetMipLevelsNum(size.X, size.Y))
			.Set_IsUAV(true)
			.Build()
		);

		this->TemporalFeedback1 = device->CreateTexture(textureBuilder
			.Set_Format(RHI::RHIFormat::RGBA16_SNORM)
			.Set_DebugName("TemporalFeedback1")
			.Set_MipLevels(1)
			.Set_IsUAV(true)
			.Build()
		);

		this->TemporalFeedback2 = device->CreateTexture(textureBuilder.Set_DebugName("TemporalFeedback2").Build());



		this->LDRColor = device->CreateTexture(textureBuilder
			.Set_Format(RHI::RHIFormat::SRGBA8_UNORM)
			.Set_DebugName("LdrColor")
			.Set_IsUAV(false)
			.Build()
		);

		this->AmbientOcclusion = device->CreateTexture(textureBuilder
			.Set_Format(RHI::RHIFormat::R8_UNORM)
			.Set_DebugName("AmbientOcclusion")
			.Set_IsUAV(true)
			.Build()
		);

		if (device->QueryFeatureSupport(RHI::RHIFeature::VirtualResources)) {
			Uint64 heapSize{ 0 };

			Array<Imp_Texture*, 7>textures{
				this->HDRColor,
				this->MaterialIDs,
				this->ResolvedColor,
				this->TemporalFeedback1,
				this->TemporalFeedback2,
				this->LDRColor,
				this->AmbientOcclusion
			};

			for (auto texture : textures) {
				RHI::RHIMemoryRequirements memReq{ device->Get_TextureMemoryRequirements(texture) };
				heapSize = Math::Align(heapSize, memReq.Alignment);
				heapSize += memReq.Size;
			}

			this->Heap = device->CreateHeap(RHI::RHIHeapDesc{
					.Type = RHI::RHIHeapType::DeviceLocal,
					.Size = heapSize,
					.DebugName = _W("RenderTargetHeap")
				}
			);

			Uint64 offset{ 0 };
			for (auto texture : textures) {
				RHI::RHIMemoryRequirements memReq{ device->Get_TextureMemoryRequirements(texture) };
				offset = Math::Align(offset, memReq.Alignment);

				device->BindTextureMemory(texture, this->Heap, offset);

				offset += memReq.Size;
			}
		}

		this->ForwardFrameBuffer = MakeShared<Parting::FrameBufferFactory<CurrentAPI>>(device);
		this->ForwardFrameBuffer->RenderTargets.assign({ this->HDRColor });
		this->ForwardFrameBuffer->DepthStencil = this->Depth;

		this->HDRFrameBuffer = MakeShared<Parting::FrameBufferFactory<CurrentAPI>>(device);
		this->HDRFrameBuffer->RenderTargets.assign({ this->HDRColor });

		this->LDRFrameBuffer = MakeShared<Parting::FrameBufferFactory<CurrentAPI>>(device);
		this->LDRFrameBuffer->RenderTargets.assign({ this->LDRColor });

		this->ResolvedFrameBuffer = MakeShared<Parting::FrameBufferFactory<CurrentAPI>>(device);
		this->ResolvedFrameBuffer->RenderTargets.assign({ this->ResolvedColor });

		this->MaterialIDFrameBuffer = MakeShared<Parting::FrameBufferFactory<CurrentAPI>>(device);
		this->MaterialIDFrameBuffer->RenderTargets.assign({ this->MaterialIDs });
		this->MaterialIDFrameBuffer->DepthStencil = this->Depth;
	}

	virtual void Clear(Imp_CommandList* commandList) {
		this->GBufferRenderTargets::Clear(commandList);

		commandList->ClearTextureFloat(this->HDRColor, RHI::g_AllSubResourceSet, Color{ 0.f });
	}


};



enum class AntiAliasingMode :Uint8 {
	NONE,
	TEMPORAL,
	MSAA_2X,
	MSAA_4X,
	MSAA_8X
};



struct SmaplerUIData final {
	SharedPtr<Parting::SceneCamera<CurrentAPI>>	ActiveSceneCamera;

	AntiAliasingMode												AntiAliasingMode{ AntiAliasingMode::TEMPORAL };
	Parting::TemporalAntiAliasingJitter								TemporalAntiAliasingJitter{ Parting::TemporalAntiAliasingJitter::MSAA };

	bool															ShowUI{ true /*false*/ };
	bool															ShowConsole{ false };
	bool															UseDeferredShading{ true /*false*/ };
	bool															Stereo{ false };
	bool															EnableSSAO{ /*true*/ false };
	bool															UseThirdPersonCamera{ false };
	bool															EnableAnimations{ false };
	bool															ShaderReloadRequested{ false };

	bool															EnableVsync{ true };

	bool															EnableProceduralSky{ true /*false*/ };
	bool															EnableBloom{ /*true*/ false };
	float															BloomSigma{ 32.f };
	float															BloomAlpha{ 0.05f };
	bool															EnableTranslucency{ /*true*/ false };
	bool															EnableMaterialEvents{ false };
	bool															EnableShadows{ /*true*/ false };
	float															AmbientIntensity{ 1.0f };
	bool															EnableLightProbe{ /*true*/ false };
	float															LightProbeDiffuseScale{ 1.f };
	float															LightProbeSpecularScale{ 1.f };
	float															CsmExponent{ 4.f };
	bool															DisplayShadowMap{ false };
	bool															TestMipMapGen{ false };

	Parting::SkyPass<CurrentAPI>::Parameters						SkyParams;
	Parting::SSAOPass<CurrentAPI>::Parameters						SSAOParams;
	Parting::TemporalAntiAliasingPass<CurrentAPI>::Parameters		TemporalAntiAliasingParams;
	Parting::ToneMappingPass<CurrentAPI>::Parameters				ToneMappingParams;

};



class FeatureSample :public Parting::ApplicationBase<CurrentAPI> {
	using Imp_CommandList = RHI::RHITypeTraits<CurrentAPI>::Imp_CommandList;
	using Imp_Texture = RHI::RHITypeTraits<CurrentAPI>::Imp_Texture;
	using Imp_Buffer = RHI::RHITypeTraits<CurrentAPI>::Imp_Buffer;
	using Imp_FrameBuffer = RHI::RHITypeTraits<CurrentAPI>::Imp_FrameBuffer;
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

		this->m_TextureCache = MakeShared<Parting::TextureCache<CurrentAPI>>(this->m_DeviceManager->Get_Device(), this->m_NativeFs);

		this->m_ShaderFactory = MakeShared<Parting::ShaderFactory<CurrentAPI>>(this->m_DeviceManager->Get_Device(), this->m_RootFs, "/Shaders");
		this->m_CommonPasses = MakeShared<Parting::CommonRenderPasses<CurrentAPI>>(this->m_DeviceManager->Get_Device(), this->m_ShaderFactory);

		this->m_OpaqueDrawStrategy = MakeShared<decltype(this->m_OpaqueDrawStrategy)::element_type>();
		this->m_TransparentDrawStrategy = MakeShared<decltype(this->m_TransparentDrawStrategy)::element_type>();

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
	SharedPtr<IFileSystem> Get_RootFS(void) const { return this->m_RootFs; }

	decltype(auto) Get_ShaderFactory(void)const { return this->m_ShaderFactory; }

	const Path& Get_SceneDir(void)const { return this->m_SceneDir; }

	const String& Get_CurrentSceneName(void) const { return this->m_CurrentSceneName; }

	Span<const String> Get_AvailableScenes(void) const { return Span<const String>{this->m_SceneFilesAvailable.data(), this->m_SceneFilesAvailable.size()}; }

	const Vector<SharedPtr<Parting::LightProbe<CurrentAPI>>>& Get_LightProbes(void) const { return this->m_LightProbes; }

	bool Is_SceneLoading(void) const { return nullptr != this->m_SceneLoadingThread; }

	const SharedPtr<Parting::Scene<CurrentAPI>>& Get_Scene(void) const { return this->m_Scene; }

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
			auto probe{ MakeShared<Parting::LightProbe<CurrentAPI>>() };

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

	bool SetView(void) {
		if (nullptr != this->m_TemporalAntiAliasingPass)
			this->m_TemporalAntiAliasingPass->Set_Jitter(this->m_UIData.TemporalAntiAliasingJitter);

		Math::VecF2 renderTargetSize{ Math::VecF2{ this->m_RenderTargets->Get_Size()} };

		Math::VecF2 pixelOffset{
			this->m_UIData.AntiAliasingMode == AntiAliasingMode::TEMPORAL && nullptr != this->m_TemporalAntiAliasingPass
			? m_TemporalAntiAliasingPass->Get_CurrentPixelOffset()
			: Math::VecF2::Zero()
		};

		SharedPtr<Parting::StereoPlanarView> stereoView{ DynamicPointerCast<Parting::StereoPlanarView>(this->m_View) };
		SharedPtr<Parting::PlanarView> planarView{ DynamicPointerCast<Parting::PlanarView>(this->m_View) };


		Math::AffineF3 viewMatrix{ Math::AffineF3::Identity() };
		float verticalFov{ Math::Radians(this->m_CameraVerticalFov) };
		float zNear{ 0.01f };
		if (nullptr != this->m_UIData.ActiveSceneCamera) {
			auto perspectiveCamera{ DynamicPointerCast<Parting::PerspectiveCamera<CurrentAPI>>(this->m_UIData.ActiveSceneCamera) };
			if (nullptr != perspectiveCamera) {
				zNear = perspectiveCamera->ZNear;
				verticalFov = perspectiveCamera->VerticalFOV;
			}

			viewMatrix = this->m_UIData.ActiveSceneCamera->Get_WorldToViewMatrix();
		}
		else
			viewMatrix = this->m_UIData.UseThirdPersonCamera ? this->m_ThirdPersonCamera.Get_WorldToViewMatrix() : this->m_FirstPersonCamera.Get_WorldToViewMatrix();

		bool topologyChanged{ false };

		if (this->m_UIData.Stereo) {
			ASSERT(false);
		}
		else {
			if (nullptr == planarView) {
				this->m_View = planarView = MakeShared<Parting::PlanarView>();
				this->m_ViewPrevious = MakeShared<Parting::PlanarView>();
				topologyChanged = true;
			}

			Math::MatF44 projection{ Math::PerspProjD3DStyleReverse(verticalFov, renderTargetSize.X / renderTargetSize.Y, zNear) };

			planarView->Set_Viewport(RHI::RHIViewport::Build(renderTargetSize.X, renderTargetSize.Y));
			planarView->Set_PixelOffset(pixelOffset);

			planarView->Set_Matrices(viewMatrix, projection);
			planarView->UpdateCache();

			this->m_ThirdPersonCamera.Set_View(*planarView);

			if (topologyChanged)
				*StaticPointerCast<Parting::PlanarView>(this->m_ViewPrevious) = *StaticPointerCast<Parting::PlanarView>(this->m_View);
		}

		return topologyChanged;
	}

	void CreateRenderPasses(bool& exposureResetRequired) {
		constexpr Uint32 motionVectorStencilMask{ 0x01 };


		decltype(this->m_ForwardPass)::element_type::CreateParameters ForwardParams{
			.TrackLiveness { false }
		};
		this->m_ForwardPass = MakeUnique<decltype(this->m_ForwardPass)::element_type>(this->m_DeviceManager->Get_Device(), this->m_CommonPasses);
		this->m_ForwardPass->Init(*this->m_ShaderFactory, ForwardParams);

		decltype(this->m_GBufferPass)::element_type::CreateParameters GBufferParams{
			.EnableMotionVectors{ true },
			.StencilWriteMask{ motionVectorStencilMask },
		};
		this->m_GBufferPass = MakeUnique<decltype(this->m_GBufferPass)::element_type>(this->m_DeviceManager->Get_Device(), this->m_CommonPasses);
		this->m_GBufferPass->Init(*this->m_ShaderFactory, GBufferParams);

		GBufferParams.EnableMotionVectors = false;
		this->m_MaterialIDPass = MakeUnique<decltype(this->m_MaterialIDPass)::element_type>(this->m_DeviceManager->Get_Device(), this->m_CommonPasses);
		this->m_MaterialIDPass->Init(*this->m_ShaderFactory, GBufferParams);

		this->m_PixelReadbackPass = MakeUnique<decltype(this->m_PixelReadbackPass)::element_type>(this->m_DeviceManager->Get_Device(), this->m_ShaderFactory, this->m_RenderTargets->MaterialIDs, RHI::RHIFormat::RGBA32_UINT);
		this->m_MipMapGenPass = MakeUnique<decltype(this->m_MipMapGenPass)::element_type>(this->m_DeviceManager->Get_Device(), this->m_ShaderFactory, this->m_RenderTargets->ResolvedColor, Parting::MipMapGenPass<CurrentAPI>::Mode::Color);

		this->m_DeferredLightingPass = MakeUnique<decltype(this->m_DeferredLightingPass)::element_type>(this->m_DeviceManager->Get_Device(), this->m_CommonPasses);
		this->m_DeferredLightingPass->Init(this->m_ShaderFactory);

		this->m_SkyPass = MakeUnique<decltype(this->m_SkyPass)::element_type>(this->m_DeviceManager->Get_Device(), this->m_ShaderFactory, this->m_CommonPasses, this->m_RenderTargets->ForwardFrameBuffer, *this->m_View);

		{
			decltype(this->m_TemporalAntiAliasingPass)::element_type::CreateParameters taaParams;
			taaParams.SourceDepth = m_RenderTargets->Depth;
			taaParams.MotionVectors = m_RenderTargets->MotionVectors;
			taaParams.UnresolvedColor = m_RenderTargets->HDRColor;
			taaParams.ResolvedColor = m_RenderTargets->ResolvedColor;
			taaParams.Feedback1 = m_RenderTargets->TemporalFeedback1;
			taaParams.Feedback2 = m_RenderTargets->TemporalFeedback2;
			taaParams.MotionVectorStencilMask = motionVectorStencilMask;
			taaParams.UseCatmullRomFilter = true;

			this->m_TemporalAntiAliasingPass = MakeUnique<decltype(this->m_TemporalAntiAliasingPass)::element_type>(this->m_DeviceManager->Get_Device(), this->m_ShaderFactory, this->m_CommonPasses, *this->m_View, taaParams);
		}

		if (this->m_RenderTargets->Get_SampleCount() == 1)
			this->m_SSAOPass = MakeUnique<decltype(this->m_SSAOPass)::element_type>(this->m_DeviceManager->Get_Device(), this->m_ShaderFactory, this->m_CommonPasses, this->m_RenderTargets->Depth, this->m_RenderTargets->GBufferNormals, this->m_RenderTargets->AmbientOcclusion);

		this->m_LightProbePass = MakeUnique<decltype(this->m_LightProbePass)::element_type>(this->m_DeviceManager->Get_Device(), this->m_ShaderFactory, this->m_CommonPasses);

		Imp_Buffer* exposureBuffer{ nullptr };
		if (nullptr != this->m_ToneMappingPass)
			exposureBuffer = this->m_ToneMappingPass->Get_ExposureBuffer();
		else
			exposureResetRequired = true;

		decltype(this->m_ToneMappingPass)::element_type::CreateParameters toneMappingParams;
		toneMappingParams.ExposureBufferOverride = exposureBuffer;
		this->m_ToneMappingPass = MakeUnique<decltype(this->m_ToneMappingPass)::element_type>(this->m_DeviceManager->Get_Device(), this->m_ShaderFactory, this->m_CommonPasses, this->m_RenderTargets->LDRFrameBuffer, *this->m_View, toneMappingParams);

		this->m_BloomPass = MakeUnique<decltype(this->m_BloomPass)::element_type>(this->m_DeviceManager->Get_Device(), this->m_ShaderFactory, this->m_CommonPasses, this->m_RenderTargets->ResolvedFrameBuffer, *this->m_View);

		this->m_PreviousViewsValid = false;
	}



	SmaplerUIData& m_UIData;
	Parting::BindingCache<CurrentAPI>							m_BindingCache;

	SharedPtr<RootFileSystem>									m_RootFs;
	SharedPtr<NativeFileSystem>									m_NativeFs;

	Path														m_SceneDir;
	Vector<String>												m_SceneFilesAvailable;
	SharedPtr<Parting::ShaderFactory<CurrentAPI>>				m_ShaderFactory;


	SharedPtr<Parting::InstancedOpaqueDrawStrategy<CurrentAPI>>	m_OpaqueDrawStrategy;
	SharedPtr<Parting::TransparentDrawStrategy<CurrentAPI>>		m_TransparentDrawStrategy;

	SharedPtr<Parting::CascadedShadowMap<CurrentAPI>>			m_ShadowMap;
	SharedPtr<Parting::FrameBufferFactory<CurrentAPI>>			m_ShadowFramebuffer;
	SharedPtr<Parting::DepthPass<CurrentAPI>>					m_ShadowDepthPass;

	RHI::RefCountPtr<Imp_CommandList> 							m_CommandList;

	bool														m_PreviousViewsValid{ false };
	Parting::FirstPersonCamera									m_FirstPersonCamera;
	Parting::ThirdPersonCamera									m_ThirdPersonCamera;

	SharedPtr<Parting::Scene<CurrentAPI>>						m_Scene;
	String														m_CurrentSceneName;
	SharedPtr<Parting::DirectionalLight<CurrentAPI>>			m_SunLight;

	Vector<SharedPtr<Parting::LightProbe<CurrentAPI>>>			m_LightProbes;
	RHI::RefCountPtr<Imp_Texture>								m_LightProbeDiffuseTexture;
	RHI::RefCountPtr<Imp_Texture>								m_LightProbeSpecularTexture;

	UniquePtr<RenderTargets>									m_RenderTargets;
	UniquePtr<Parting::ForwardShadingPass<CurrentAPI>>			m_ForwardPass;
	UniquePtr<Parting::GBufferFillPass<CurrentAPI>>				m_GBufferPass;
	UniquePtr<Parting::DeferredLightingPass<CurrentAPI>>		m_DeferredLightingPass;
	UniquePtr<Parting::SkyPass<CurrentAPI>>						m_SkyPass;
	UniquePtr<Parting::MaterialIDPass<CurrentAPI>>				m_MaterialIDPass;
	UniquePtr<Parting::PixelReadbackPass<CurrentAPI>>			m_PixelReadbackPass;
	UniquePtr<Parting::MipMapGenPass<CurrentAPI>>				m_MipMapGenPass;
	UniquePtr<Parting::ToneMappingPass<CurrentAPI>>				m_ToneMappingPass;
	UniquePtr<Parting::SSAOPass<CurrentAPI>>					m_SSAOPass;
	UniquePtr<Parting::LightProbeProcessingPass<CurrentAPI>>	m_LightProbePass;
	UniquePtr<Parting::TemporalAntiAliasingPass<CurrentAPI>>	m_TemporalAntiAliasingPass;
	UniquePtr<Parting::BloomPass<CurrentAPI>>					m_BloomPass;

	SharedPtr<Parting::IView>									m_View;
	SharedPtr<Parting::IView>									m_ViewPrevious;


	float														m_WallclockTime{ 0.f };

	float														m_CameraVerticalFov{ 60.f };
	Math::VecF3													m_AmbientTop{ Math::VecF3::Zero() };
	Math::VecF3													m_AmbientBottom{ Math::VecF3::Zero() };
	Math::VecU2													m_PickPosition{ Math::VecU2::Zero() };
	bool														m_Pick{ false };


public:
	bool KeyboardUpdate(Int32 key, Int32 scancode, Int32 action, Int32 mods) {
		if (GLFW_KEY_ESCAPE == key && GLFW_PRESS == action) {
			this->m_UIData.ShowUI = !this->m_UIData.ShowUI;
			return true;
		}

		if (GLFW_KEY_GRAVE_ACCENT == key && GLFW_PRESS == action) {
			this->m_UIData.ShowConsole = !this->m_UIData.ShowConsole;
			return true;
		}//TODO :Remove

		if (GLFW_KEY_SPACE == key && GLFW_PRESS == action) {
			this->m_UIData.EnableAnimations = !this->m_UIData.EnableAnimations;
			return true;
		}

		if (GLFW_KEY_T == key && GLFW_PRESS == action) {
			this->CopyActiveCameraToFirstPerson();
			if (nullptr != this->m_UIData.ActiveSceneCamera) {
				this->m_UIData.UseThirdPersonCamera = false;
				this->m_UIData.ActiveSceneCamera = nullptr;
			}
			else
				this->m_UIData.UseThirdPersonCamera = !this->m_UIData.UseThirdPersonCamera;
			return true;
		}

		if (nullptr == this->m_UIData.ActiveSceneCamera) {
			if (this->m_UIData.UseThirdPersonCamera)
				this->m_ThirdPersonCamera.KeyboardUpdate(key, scancode, action, mods);
			else
				this->m_FirstPersonCamera.KeyboardUpdate(key, scancode, action, mods);
		}
		return true;
	}

	bool MousePosUpdate(double xpos, double ypos) {
		if (nullptr == this->m_UIData.ActiveSceneCamera) {
			if (this->m_UIData.UseThirdPersonCamera)
				this->m_ThirdPersonCamera.MousePosUpdate(xpos, ypos);
			else
				this->m_FirstPersonCamera.MousePosUpdate(xpos, ypos);
		}
		this->m_PickPosition = Math::VecU2{ static_cast<Uint32>(xpos), static_cast<Uint32>(ypos) };

		return true;
	}

	bool MouseScrollUpdate(double xoffset, double yoffset) {
		if (nullptr == this->m_UIData.ActiveSceneCamera) {
			if (this->m_UIData.UseThirdPersonCamera)
				this->m_ThirdPersonCamera.MouseScrollUpdate(xoffset, yoffset);
			else
				this->m_FirstPersonCamera.MouseScrollUpdate(xoffset, yoffset);
		}

		return true;
	}

	bool MouseButtonUpdate(Int32 button, Int32 action, Int32 mods) {
		if (nullptr == this->m_UIData.ActiveSceneCamera) {
			if (this->m_UIData.UseThirdPersonCamera)
				this->m_ThirdPersonCamera.MouseButtonUpdate(button, action, mods);
			else
				this->m_FirstPersonCamera.MouseButtonUpdate(button, action, mods);
		}

		if (GLFW_PRESS == action && GLFW_MOUSE_BUTTON_2 == button)
			this->m_Pick = true;

		return true;
	}


	void Animate(float fElapsedTimeSeconds) override {
		if (nullptr == this->m_UIData.ActiveSceneCamera) {
			if (this->m_UIData.UseThirdPersonCamera)
				this->m_ThirdPersonCamera.Animate(fElapsedTimeSeconds);
			else
				this->m_FirstPersonCamera.Animate(fElapsedTimeSeconds);
		}

		if (nullptr != this->m_ToneMappingPass)
			this->m_ToneMappingPass->AdvanceFrame(fElapsedTimeSeconds);

		if (this->m_IsAsyncLoad && this->m_UIData.EnableAnimations) {
			this->m_WallclockTime += fElapsedTimeSeconds;

			ASSERT(false);
		}
	}


public:
	void RenderSplashScreen(Imp_FrameBuffer* framebuffer) override {
		//TODO : add splash screen
		//this->m_Scene->RenderSplashScreen(framebuffer);
	}

	void RenderScene(Imp_FrameBuffer* framebuffer) override {
		this->m_Scene->RefreshSceneGraph(this->Get_FrameIndex());

		const auto [windowWidth, windowHeight] { this->m_DeviceManager->Get_WindowDimensions()};
		bool exposureResetRequired{ false };
		{
			Uint32 sampleCount{ 1 };
			switch (this->m_UIData.AntiAliasingMode) {
			case AntiAliasingMode::MSAA_2X: sampleCount = 2; break;
			case AntiAliasingMode::MSAA_4X: sampleCount = 4; break;
			case AntiAliasingMode::MSAA_8X: sampleCount = 8; break;
			default:break;
			}

			bool needNewPasses{ false };

			if (nullptr == this->m_RenderTargets || this->m_RenderTargets->Is_UpdateRequired(Math::VecU2{ windowWidth, windowHeight }, sampleCount)) {
				needNewPasses = true;

				this->m_BindingCache.Clear();

				this->m_RenderTargets.reset();
				this->m_RenderTargets = MakeUnique<RenderTargets>();
				this->m_RenderTargets->Init(
					this->m_DeviceManager->Get_Device(),
					Math::VecU2{ windowWidth, windowHeight },
					sampleCount,
					true,
					true
				);
			}

			if (this->SetView())
				needNewPasses = true;

			if (this->m_UIData.ShaderReloadRequested) {
				this->m_ShaderFactory->ClearCache();
				needNewPasses = true;
				ASSERT(false);
			}

			if (needNewPasses)
				this->CreateRenderPasses(exposureResetRequired);

			this->m_UIData.ShaderReloadRequested = false;
		}

		this->m_CommandList->Open();

		/*auto FrameIndex{ this->Get_FrameIndex() }; */// TODO Here func has a bug
		this->m_Scene->RefreshBuffers(this->m_CommandList, this->Get_FrameIndex());

		auto framebufferTexture{ framebuffer->Get_Desc().ColorAttachments[0].Texture };
		this->m_CommandList->ClearTextureFloat(framebufferTexture, RHI::g_AllSubResourceSet, Color{ 0.f });

		this->m_AmbientTop = this->m_UIData.AmbientIntensity * this->m_UIData.SkyParams.SkyColor * this->m_UIData.SkyParams.Brightness;
		this->m_AmbientBottom = this->m_UIData.AmbientIntensity * this->m_UIData.SkyParams.GroundColor * this->m_UIData.SkyParams.Brightness;

		if (this->m_UIData.EnableShadows) {
			this->m_SunLight->ShadowMap = this->m_ShadowMap;
			Math::BoxF3 sceneBounds{ this->m_Scene->Get_SceneGraph()->Get_RootNode()->Get_GlobalBoundingBox() };

			Math::Frustum projectionFrustum{ this->m_View->Get_ProjectionFrustum() };
			constexpr float maxShadowDistance{ 100.f };

			Math::AffineF3 viewMatrixInv{ this->m_View->Get_ChildView(Parting::ViewType::PLANAR, 0)->Get_InverseViewMatrix() };

			const float zRange{ Math::Length(sceneBounds.Diagonal()) };
			this->m_ShadowMap->SetupForPlanarViewStable(*this->m_SunLight, projectionFrustum, viewMatrixInv, maxShadowDistance, zRange, zRange, m_UIData.CsmExponent);

			this->m_ShadowMap->Clear(this->m_CommandList.Get());

			decltype(this->m_ShadowDepthPass)::element_type::Context context;

			Parting::IGeometryPass<CurrentAPI>::RenderCompositeView(this->m_CommandList,
				&this->m_ShadowMap->Get_View(), nullptr,
				*this->m_ShadowFramebuffer,
				this->m_Scene->Get_SceneGraph()->Get_RootNode(),
				*this->m_OpaqueDrawStrategy,
				*this->m_ShadowDepthPass,
				context,
				"ShadowMap",
				this->m_UIData.EnableMaterialEvents
			);
		}
		else
			this->m_SunLight->ShadowMap = nullptr;

		Vector<SharedPtr<Parting::LightProbe<CurrentAPI>>> lightProbes;
		if (this->m_UIData.EnableLightProbe) {
			for (auto& probe : this->m_LightProbes)
				if (probe->Enabled) {
					probe->DiffuseScale = this->m_UIData.LightProbeDiffuseScale;
					probe->SpecularScale = this->m_UIData.LightProbeSpecularScale;
					lightProbes.push_back(probe);
				}
		}

		this->m_RenderTargets->Clear(this->m_CommandList);

		decltype(this->m_ForwardPass)::element_type::Context forwardContext;

		if (exposureResetRequired)
			this->m_ToneMappingPass->ResetExposure(this->m_CommandList, 0.5f);
		if (!this->m_UIData.UseDeferredShading || this->m_UIData.EnableTranslucency)
			this->m_ForwardPass->PrepareLights(forwardContext, this->m_CommandList, this->m_Scene->Get_SceneGraph()->Get_Lights(), this->m_AmbientTop, this->m_AmbientBottom, lightProbes);


		if (this->m_UIData.UseDeferredShading) {

			decltype(this->m_GBufferPass)::element_type::Context gbufferContext;

			Parting::IGeometryPass<CurrentAPI>::RenderCompositeView(this->m_CommandList,
				this->m_View.get(), this->m_ViewPrevious.get(),
				*this->m_RenderTargets->GBufferFrameBuffer,
				this->m_Scene->Get_SceneGraph()->Get_RootNode(),
				*this->m_OpaqueDrawStrategy,
				*this->m_GBufferPass,
				gbufferContext,
				"GBufferFill",
				this->m_UIData.EnableMaterialEvents
			);

			Imp_Texture* ambientOcclusionTarget{ nullptr };
			if (this->m_UIData.EnableSSAO && nullptr != this->m_SSAOPass) {
				this->m_SSAOPass->Render(this->m_CommandList, this->m_UIData.SSAOParams, *this->m_View);
				ambientOcclusionTarget = this->m_RenderTargets->AmbientOcclusion;
			}

			decltype(this->m_DeferredLightingPass)::element_type::Inputs deferredInputs;
			deferredInputs.Set_GBuffer(*this->m_RenderTargets);
			deferredInputs.AmbientOcclusion = this->m_UIData.EnableSSAO ? this->m_RenderTargets->AmbientOcclusion : nullptr;
			deferredInputs.AmbientColorTop = this->m_AmbientTop;
			deferredInputs.AmbientColorBottom = this->m_AmbientBottom;
			deferredInputs.Lights = &this->m_Scene->Get_SceneGraph()->Get_Lights();
			deferredInputs.LightProbes = this->m_UIData.EnableLightProbe ? &this->m_LightProbes : nullptr;
			deferredInputs.Output = this->m_RenderTargets->HDRColor;

			this->m_DeferredLightingPass->Render(this->m_CommandList, *this->m_View, deferredInputs);
		}
		else {
			Parting::IGeometryPass<CurrentAPI>::RenderCompositeView(this->m_CommandList,
				this->m_View.get(), this->m_ViewPrevious.get(),
				*this->m_RenderTargets->ForwardFrameBuffer,
				this->m_Scene->Get_SceneGraph()->Get_RootNode(),
				*this->m_OpaqueDrawStrategy,
				*this->m_ForwardPass,
				forwardContext,
				"ForwardOpaque",
				this->m_UIData.EnableMaterialEvents
			);
		}

		if (this->m_Pick) {
			this->m_CommandList->ClearTextureUInt(this->m_RenderTargets->MaterialIDs, RHI::g_AllSubResourceSet, 0xffffu);

			decltype(this->m_MaterialIDPass)::element_type::Context materialIdContext;

			Parting::IGeometryPass<CurrentAPI>::RenderCompositeView(this->m_CommandList,
				this->m_View.get(), this->m_ViewPrevious.get(),
				*this->m_RenderTargets->MaterialIDFrameBuffer,
				this->m_Scene->Get_SceneGraph()->Get_RootNode(),
				*this->m_OpaqueDrawStrategy,
				*this->m_MaterialIDPass,
				materialIdContext,
				"MaterialID"
			);

			if (this->m_UIData.EnableTranslucency) {
				Parting::IGeometryPass<CurrentAPI>::RenderCompositeView(this->m_CommandList,
					this->m_View.get(), this->m_ViewPrevious.get(),
					*this->m_RenderTargets->MaterialIDFrameBuffer,
					this->m_Scene->Get_SceneGraph()->Get_RootNode(),
					*this->m_TransparentDrawStrategy,
					*this->m_MaterialIDPass,
					materialIdContext,
					"MaterialID - Translucent"
				);
			}

			this->m_PixelReadbackPass->Capture(this->m_CommandList, this->m_PickPosition);
		}

		if (this->m_UIData.EnableProceduralSky)
			this->m_SkyPass->Render(this->m_CommandList, *this->m_View, *this->m_SunLight, this->m_UIData.SkyParams);

		if (this->m_UIData.EnableTranslucency) {
			Parting::IGeometryPass<CurrentAPI>::RenderCompositeView(this->m_CommandList,
				this->m_View.get(), this->m_ViewPrevious.get(),
				*this->m_RenderTargets->ForwardFrameBuffer,
				this->m_Scene->Get_SceneGraph()->Get_RootNode(),
				*this->m_TransparentDrawStrategy,
				*this->m_ForwardPass,
				forwardContext,
				"ForwardTransparent",
				this->m_UIData.EnableMaterialEvents
			);
		}

		Imp_Texture* finalHdrColor{ this->m_RenderTargets->HDRColor };

		if (this->m_UIData.AntiAliasingMode == AntiAliasingMode::TEMPORAL) {
			if (this->m_PreviousViewsValid)
				this->m_TemporalAntiAliasingPass->RenderMotionVectors(this->m_CommandList, *this->m_View, *this->m_ViewPrevious);

			this->m_TemporalAntiAliasingPass->TemporalResolve(this->m_CommandList, this->m_UIData.TemporalAntiAliasingParams, this->m_PreviousViewsValid, *this->m_View, *this->m_View);

			finalHdrColor = this->m_RenderTargets->ResolvedColor;

			if (this->m_UIData.EnableBloom)
				this->m_BloomPass->Render(this->m_CommandList, this->m_RenderTargets->ResolvedFrameBuffer, *this->m_View, this->m_RenderTargets->ResolvedColor, this->m_UIData.BloomSigma, this->m_UIData.BloomAlpha);

			this->m_PreviousViewsValid = true;
		}
		else {
			auto finalHdrFramebuffer{ m_RenderTargets->HDRFrameBuffer };

			if (this->m_RenderTargets->Get_SampleCount() > 1) {
				this->m_CommandList->ResolveTexture(m_RenderTargets->ResolvedColor, RHI::RHITextureSubresourceSet{}, m_RenderTargets->HDRColor, RHI::RHITextureSubresourceSet{});//(0.1.0.1)
				finalHdrColor = m_RenderTargets->ResolvedColor;
				finalHdrFramebuffer = this->m_RenderTargets->ResolvedFrameBuffer;
			}

			if (this->m_UIData.EnableBloom)
				this->m_BloomPass->Render(this->m_CommandList, finalHdrFramebuffer, *this->m_View, finalHdrColor, this->m_UIData.BloomSigma, this->m_UIData.BloomAlpha);

			m_PreviousViewsValid = false;
		}

		auto toneMappingParams{ this->m_UIData.ToneMappingParams };
		if (exposureResetRequired) {
			toneMappingParams.EyeAdaptationSpeedUp = 0.f;
			toneMappingParams.EyeAdaptationSpeedDown = 0.f;
		}
		this->m_ToneMappingPass->SimpleRender(this->m_CommandList, toneMappingParams, *this->m_View, finalHdrColor);

		this->m_CommonPasses->BLITTexture(this->m_CommandList, framebuffer, this->m_RenderTargets->LDRColor, &this->m_BindingCache);

		if (this->m_UIData.TestMipMapGen) {
			ASSERT(false);
			/*this->m_MipMapGenPass->Dispatch(this->m_CommandList);
			this->m_MipMapGenPass->Display(this->m_CommonPasses, this->m_CommandList, framebuffer);*/
		}

		if (this->m_UIData.DisplayShadowMap) {
			ASSERT(false);
			/*for (Uint32 cascade = 0; cascade < 4; ++cascade){
				auto viewport = RHI::RHIViewport(
					10.f + 266.f * cascade,
					266.f * (1 + cascade),
					this-WindowViewport.maxY - 266.f,
					WindowViewport.maxY - 10.f, 0.f, 1.f
				);

				engine::BlitParameters blitParams;
				blitParams.targetFramebuffer = framebuffer;
				blitParams.targetViewport = viewport;
				blitParams.sourceTexture = m_ShadowMap->GetTexture();
				blitParams.sourceArraySlice = cascade;
				m_CommonPasses->BlitTexture(m_CommandList, blitParams, &m_BindingCache);
			}*/
		}

		this->m_CommandList->Close();
		this->m_DeviceManager->Get_Device()->ExecuteCommandList(this->m_CommandList);

		if (this->m_Pick) {
			ASSERT(false);
		}

		this->m_TemporalAntiAliasingPass->AdvanceFrame();
		::Swap(this->m_View, this->m_ViewPrevious);

		this->m_DeviceManager->Set_VsyncEnabled(this->m_UIData.EnableVsync);
	}

	bool LoadScene(SharedPtr<IFileSystem> fs, const Path& sceneFileName) override {
		//TODO :add Time Show

		this->m_Scene = MakeUnique<Parting::Scene<CurrentAPI>>(this->m_DeviceManager->Get_Device(), *this->m_ShaderFactory, fs, this->m_TextureCache);

		//TODO add time cast info

		return this->m_Scene->Load(sceneFileName);
	}

	void SceneLoaded(void) override {
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

};

class SamplerUIRender final :public Parting::UIRenderer<CurrentAPI> {
	using Imp_CommandList = RHI::RHITypeTraits<CurrentAPI>::Imp_CommandList;
public:
	SamplerUIRender(::DeviceManager* deviceManager, SharedPtr<FeatureSample> app, SmaplerUIData& ui) :
		UIRenderer{ deviceManager },
		m_APP{ MoveTemp(app) },
		m_UIData{ ui } {

		this->m_CommandList = this->m_DeviceManager->Get_Device()->CreateCommandList();

		this->m_FontOpenSans = this->CreateFontFromFile(*(this->m_APP->Get_RootFS()), "/Media/Fonts/OpenSans/OpenSans-Regular.ttf", 17.f);
		this->m_FontDroidMono = this->CreateFontFromFile(*(this->m_APP->Get_RootFS()), "/Media/Fonts/DroidSans/DroidSans-Mono.ttf", 14.f);

		ImGui::GetIO().IniFilename = nullptr;
	}

public:



private:

private:
	SharedPtr<FeatureSample> m_APP;

	SharedPtr<Parting::RegisteredFont> m_FontOpenSans;
	SharedPtr<Parting::RegisteredFont> m_FontDroidMono;

	/*UniquePtr<ImGui_Console> m_console;*/
	SharedPtr<Parting::Light<CurrentAPI>> m_SelectedLight;

	SmaplerUIData& m_UIData;
	RHI::RefCountPtr<Imp_CommandList> m_CommandList;

private:

public:
	void BuildUI(void)override {
		if (!this->m_UIData.ShowUI)
			return;

		const auto& io{ ImGui::GetIO() };

		if (this->m_APP->Is_SceneLoading()) {
			this->BeginFullScreenWindow();
			ImGui::PushFont(m_FontOpenSans->Get_ScaledFont());

			String messageBuffer{ "TODO" };

			this->DrawScreenCenteredText(messageBuffer);

			ImGui::PopFont();
			this->EndFullScreenWindow();

			return;

		}

		ImGui::PushFont(this->m_FontOpenSans->Get_ScaledFont());

		float const fontSize{ ImGui::GetFontSize() };

		ImGui::SetNextWindowPos(ImVec2{ fontSize * 0.6f, fontSize * 0.6f }, 0);
		ImGui::Begin("Settings", 0, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Text("Renderer: %s", RHI::RHITypeTraits<CurrentAPI>::APIName);
		double frameTime{ this->m_DeviceManager->Get_AverageFrameTimeSeconds() };
		if (frameTime > 0.0)
			ImGui::Text("%.3f ms/frame (%.1f FPS)", frameTime * 1e3, 1.0 / frameTime);

		const String& sceneDir{ this->m_APP->Get_SceneDir().generic_string() };

		//TODO :
		auto getRelativePath = [&sceneDir](String const& name) {
			return
				Path{ name }.parent_path() == sceneDir
				? name.c_str() + sceneDir.size()
				: name.c_str();
			};

		const String currentScene{ this->m_APP->Get_CurrentSceneName() };
		if (ImGui::BeginCombo("Scene", getRelativePath(currentScene))) {
			for (const auto& scene : this->m_APP->Get_AvailableScenes()) {
				bool is_selected{ scene == currentScene };
				if (ImGui::Selectable(getRelativePath(scene), is_selected))
					this->m_APP->SetCurrentScene(scene);
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		if (ImGui::Button("Reload Shaders"))
			this->m_UIData.ShaderReloadRequested = true;

		ImGui::Checkbox("VSync", &this->m_UIData.EnableVsync);
		ImGui::Checkbox("Deferred Shading", &this->m_UIData.UseDeferredShading);
		if (this->m_UIData.AntiAliasingMode >= AntiAliasingMode::MSAA_2X)
			this->m_UIData.UseDeferredShading = false; // Deferred shading doesn't work with MSAA
		ImGui::Checkbox("Stereo", &this->m_UIData.Stereo);
		ImGui::Checkbox("Animations", &this->m_UIData.EnableAnimations);

		if (ImGui::BeginCombo("Camera (T)",
			nullptr != this->m_UIData.ActiveSceneCamera
			? this->m_UIData.ActiveSceneCamera->Get_Name().c_str()
			: this->m_UIData.UseThirdPersonCamera ? "Third-Person" : "First-Person")) {
			if (ImGui::Selectable("First-Person", nullptr == this->m_UIData.ActiveSceneCamera && !this->m_UIData.UseThirdPersonCamera)) {
				this->m_UIData.ActiveSceneCamera.reset();
				this->m_UIData.UseThirdPersonCamera = false;
			}
			if (ImGui::Selectable("Third-Person", nullptr == this->m_UIData.ActiveSceneCamera && this->m_UIData.UseThirdPersonCamera)) {
				this->m_UIData.ActiveSceneCamera.reset();
				this->m_UIData.UseThirdPersonCamera = true;
				this->m_APP->CopyActiveCameraToFirstPerson();
			}
			for (const auto& camera : this->m_APP->Get_Scene()->Get_SceneGraph()->Get_Cameras())
				if (ImGui::Selectable(camera->Get_Name().c_str(), this->m_UIData.ActiveSceneCamera == camera)) {
					this->m_UIData.ActiveSceneCamera = camera;
					this->m_APP->CopyActiveCameraToFirstPerson();
				}
			ImGui::EndCombo();
		}

		ImGui::Combo("AA Mode", (int*)&this->m_UIData.AntiAliasingMode, "None\0TemporalAA\0MSAA 2x\0MSAA 4x\0MSAA 8x\0");//TODO
		ImGui::Combo("TAA Camera Jitter", (int*)&this->m_UIData.TemporalAntiAliasingJitter, "MSAA\0Halton\0R2\0White Noise\0");

		ImGui::SliderFloat("Ambient Intensity", &this->m_UIData.AmbientIntensity, 0.f, 1.f);

		ImGui::Checkbox("Enable Light Probe", &this->m_UIData.EnableLightProbe);
		if (this->m_UIData.EnableLightProbe && ImGui::CollapsingHeader("Light Probe")) {
			ImGui::DragFloat("Diffuse Scale", &this->m_UIData.LightProbeDiffuseScale, 0.01f, 0.f, 10.f);
			ImGui::DragFloat("Specular Scale", &this->m_UIData.LightProbeSpecularScale, 0.01f, 0.f, 10.f);
		}

		ImGui::Checkbox("Enable Procedural Sky", &this->m_UIData.EnableProceduralSky);
		if (this->m_UIData.EnableProceduralSky && ImGui::CollapsingHeader("Sky Parameters")) {
			ImGui::SliderFloat("Brightness", &this->m_UIData.SkyParams.Brightness, 0.f, 1.f);
			ImGui::SliderFloat("Glow Size", &this->m_UIData.SkyParams.GlowSize, 0.f, 90.f);
			ImGui::SliderFloat("Glow Sharpness", &this->m_UIData.SkyParams.GlowSharpness, 1.f, 10.f);
			ImGui::SliderFloat("Glow Intensity", &this->m_UIData.SkyParams.GlowIntensity, 0.f, 1.f);
			ImGui::SliderFloat("Horizon Size", &this->m_UIData.SkyParams.HorizonSize, 0.f, 90.f);
		}
		ImGui::Checkbox("Enable SSAO", &this->m_UIData.EnableSSAO);
		ImGui::Checkbox("Enable Bloom", &this->m_UIData.EnableBloom);
		ImGui::DragFloat("Bloom Sigma", &this->m_UIData.BloomSigma, 0.01f, 0.1f, 100.f);
		ImGui::DragFloat("Bloom Alpha", &this->m_UIData.BloomAlpha, 0.01f, 0.01f, 1.0f);
		ImGui::Checkbox("Enable Shadows", &this->m_UIData.EnableShadows);
		ImGui::Checkbox("Enable Translucency", &this->m_UIData.EnableTranslucency);

		ImGui::Separator();
		ImGui::Checkbox("Temporal AA Clamping", &this->m_UIData.TemporalAntiAliasingParams.EnableHistoryClamping);
		ImGui::Checkbox("Material Events", &this->m_UIData.EnableMaterialEvents);
		ImGui::Separator();

		const auto& lights{ this->m_APP->Get_Scene()->Get_SceneGraph()->Get_Lights() };

		if (!lights.empty() && ImGui::CollapsingHeader("Lights")) {
			if (ImGui::BeginCombo("Select Light", nullptr != this->m_SelectedLight ? this->m_SelectedLight->Get_Name().c_str() : "(None)")) {
				for (const auto& light : lights) {
					bool selected{ this->m_SelectedLight == light };
					ImGui::Selectable(light->Get_Name().c_str(), &selected);
					if (selected) {
						this->m_SelectedLight = light;
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}

			/*if (m_SelectedLight)
				this->m_APP::LightEditor(*m_SelectedLight);*/

			LOG_ERROR("TODO");//TODO
		}

		ImGui::TextUnformatted("Render Light Probe: ");
		for (const auto& probe : this->m_APP->Get_LightProbes()) {
			ImGui::SameLine();
			if (ImGui::Button(probe->Name.c_str()))
				ASSERT(false);
		}

		if (ImGui::Button("Screenshot")) {
			String fileName;
			ASSERT(false);
		}

		ImGui::Separator();
		ImGui::Checkbox("Test MipMapGen Pass", &this->m_UIData.TestMipMapGen);
		ImGui::Checkbox("Display Shadow Map", &this->m_UIData.DisplayShadowMap);

		ImGui::End();

		/*auto material{ this->m_UIData.SelectedMaterial };
		if (material)
		{
			ImGui::SetNextWindowPos(ImVec2(float(width) - fontSize * 0.6f, fontSize * 0.6f), 0, ImVec2(1.f, 0.f));
			ImGui::Begin("Material Editor");
			ImGui::Text("Material %d: %s", material->materialID, material->name.c_str());

			MaterialDomain previousDomain = material->domain;
			material->dirty = donut::app::MaterialEditor(material.get(), true);

			if (previousDomain != material->domain)
				m_app->GetScene()->GetSceneGraph()->GetRootNode()->InvalidateContent();

			ImGui::End();
		}*/

		if (AntiAliasingMode::NONE != this->m_UIData.AntiAliasingMode && AntiAliasingMode::TEMPORAL != this->m_UIData.AntiAliasingMode)
			this->m_UIData.UseDeferredShading = false;

		if (!this->m_UIData.UseDeferredShading)
			this->m_UIData.EnableSSAO = false;

		ImGui::PopFont();
	}
};


int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {


	Parting::DeviceCreationParameters deviceParams;

	deviceParams.BackBufferWidth = 1920;
	deviceParams.BackBufferHeight = 1080;
	deviceParams.StartFullscreen = false;
	deviceParams.VsyncEnabled = true;
	deviceParams.EnablePerMonitorDPI = true;
	deviceParams.SupportExplicitDisplayScaling = true;

	/*deviceParams.EnableDebugRuntime = true;
	deviceParams.EnableGPUValidation = true;*/

	String Title{ "Parting Engine" };

	SharedPtr<DeviceManager> deviceManager{ MakeShared<DeviceManager>() };
	if (false == deviceManager->CreateWindowDeviceAndSwapChain(deviceParams, Title.c_str()))
		LOG_ERROR("Failed to create device manager");

	{
		SmaplerUIData UIData;
		SharedPtr< FeatureSample> sample{ MakeShared<FeatureSample>(deviceManager.get(), UIData, "") };
		SharedPtr<SamplerUIRender> uiRender{ MakeShared<SamplerUIRender>(deviceManager.get(), sample, UIData) };

		uiRender->Initialize(sample->Get_ShaderFactory());

		deviceManager->AddRenderPassToBack(sample.get());
		deviceManager->AddRenderPassToBack(uiRender.get());

		deviceManager->RunMessageLoop();
	}


	deviceManager->Shutdown();

	return 0;//Windows win app must retunr by myself,other will return 120
}