#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"

PARTING_SUBMODULE(DeviceManager, D3D12)

PARTING_IMPORT GLFWWrapper;

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;
PARTING_IMPORT Container;

PARTING_IMPORT RHI;

PARTING_SUBMODE_IMPORT Base;


#else
#pragma once

#include "Core/ModuleBuild.h"


#include "Core/Utility/Include/UtilityMacros.h"
//Global
#include "Engine/Application/Module/GLFWWrapper.h"

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#include "Core/Container/Module/Container.h"
#include "Core/String/Module/String.h"

#include "RHI/Module/RHI.h"

#include "D3D12RHI/Module/D3D12RHI.h"

#include "Engine/Application/Module/DeviceManager-Base.h"

#endif // PARTING_MODULE_BUILD


namespace Parting {

	class D3D12DeviceManager;

	template<>
	struct ManageTypeTraits<RHI::D3D12Tag> {
		using DeviceManager = typename Parting::D3D12DeviceManager;
	};


	class D3D12DeviceManager final :public DeviceManagerBase<D3D12DeviceManager, RHI::D3D12Tag> {
		friend class DeviceManagerBase<D3D12DeviceManager, RHI::D3D12Tag>;
		using Imp_Texture = typename RHI::RHITypeTraits<RHI::D3D12Tag>::Imp_Texture;
		using Imp_Device = typename RHI::RHITypeTraits<RHI::D3D12Tag>::Imp_Device;
	public:
		D3D12DeviceManager(void) = default;
		~D3D12DeviceManager(void) = default;

	public:


	public:


	private:
		static bool MoveWindowOntoAdapter(IDXGIAdapter* targetAdapter, RECT& rect) {//TODO 
			ASSERT(nullptr != targetAdapter);

			HRESULT hres{ 0 };
			unsigned int outputNo = 0;
			while (D3D12_SUCCESS(hres)) {
				RHI::RefCountPtr<IDXGIOutput> pOutput;
				hres = targetAdapter->EnumOutputs(outputNo++, &pOutput);

				if (D3D12_SUCCESS(hres) && nullptr != pOutput) {
					DXGI_OUTPUT_DESC OutputDesc;
					pOutput->GetDesc(&OutputDesc);
					const RECT desktop = OutputDesc.DesktopCoordinates;

					const int centreX = static_cast<int>(desktop.left) + static_cast<int>(desktop.right - desktop.left) / 2;
					const int centreY = static_cast<int>(desktop.top) + static_cast<int>(desktop.bottom - desktop.top) / 2;
					const int winW = rect.right - rect.left;
					const int winH = rect.bottom - rect.top;
					const int left = centreX - winW / 2;
					const int right = left + winW;
					const int top = centreY - winH / 2;
					const int bottom = top + winH;
					rect.left = Math::Max(left, static_cast<int>(desktop.left));
					rect.right = Math::Min(right, static_cast<int>(desktop.right));
					rect.bottom = Math::Min(bottom, static_cast<int>(desktop.bottom));
					rect.top = Math::Max(top, static_cast<int>(desktop.top));

					// If there is more than one output, go with the first found.  Multi-monitor support could go here.
					return true;
				}
			}

			return false;
		}

		bool CreateRenderTargets(void);

		void ReleaseRenderTargets(void);

		void ReportLiveObjects(void);

	private:
		bool										m_TearingSupported{ false };

		HWND										m_hWnd{ nullptr };
		RHI::RefCountPtr<IDXGIFactory6>				m_DXFIFactory6;
		RHI::RefCountPtr<IDXGIAdapter>				m_DXGIAdapter;
		RHI::RefCountPtr<ID3D12Device>				m_D3D12Device;

		RHI::RefCountPtr<ID3D12CommandQueue>		m_GraphicsQueue;
		RHI::RefCountPtr<ID3D12CommandQueue>		m_ComputeQueue;
		RHI::RefCountPtr<ID3D12CommandQueue>		m_CopyQueue;

		DXGI_SWAP_CHAIN_DESC1						m_SwapChainDesc{};
		DXGI_SWAP_CHAIN_FULLSCREEN_DESC				m_FullScreenDesc{};
		RHI::RefCountPtr<IDXGISwapChain3>			m_SwapChain;


		RHI::RefCountPtr<Imp_Device>				m_RHIDevice;//TODO this pace to uncon ,meybe can
		Vector<RHI::RefCountPtr<ID3D12Resource>>	m_SwapChainBuffers;
		Vector<RHI::RefCountPtr<Imp_Texture>>		m_RHISwapChainBuffers;
		RHI::RefCountPtr<ID3D12Fence>				m_FrameFence;
		Vector<HANDLE>								m_FrameFenceEvents;

		UINT64										m_FrameCount{ 1 };

		String										m_RendererString;

	private:
		bool Imp_CreateInstance(void);
		bool Imp_CreateDevice(void);
		bool Imp_CreateSwapChain(void);
		void Imp_ResizeSwapChain(void);
		Uint32 Imp_Get_BackBufferCount(void);
		Imp_Device* Imp_Get_Device(void) { return this->m_RHIDevice.Get(); }
		Imp_Texture* Imp_Get_BackBuffer(Uint32 index) { ASSERT(index < this->m_RHISwapChainBuffers.size());	return this->m_RHISwapChainBuffers[index].Get(); }

		void Imp_DestroyDeviceAndSwapChain(void);
		void Imp_Shutdown(void);
	};




	//Src

	inline bool D3D12DeviceManager::CreateRenderTargets(void) {
		this->m_SwapChainBuffers.resize(this->m_SwapChainDesc.BufferCount);
		this->m_RHISwapChainBuffers.resize(this->m_SwapChainDesc.BufferCount);

		for (Uint32 Index = 0; Index < this->m_SwapChainDesc.BufferCount; ++Index) {
			D3D12_CHECK(this->m_SwapChain->GetBuffer(Index, PARTING_IID_PPV_ARGS(&this->m_SwapChainBuffers[Index])));

			RHI::RHITextureDesc textureDesc{
				.Extent { this->m_DeviceParams.BackBufferWidth, this->m_DeviceParams.BackBufferHeight },
				.SampleCount { this->m_DeviceParams.SwapChainSampleCount },
				.SampleQuality { this->m_DeviceParams.SwapChainSampleQuality },
				.Format { this->m_DeviceParams.SwapChainFormat },
				.IsRenderTarget { true },
				.InitialState	{ RHI::RHIResourceState::Present },
				.KeepInitialState { true },
			};

			this->m_RHISwapChainBuffers[Index] = this->m_RHIDevice->CreateHandleForNativeTexture(RHI::RHIObjectType::D3D12_Resource, RHI::RHIObject{ .Pointer{ this->m_SwapChainBuffers[Index] } }, textureDesc);
		}

		return true;
	}

	inline void D3D12DeviceManager::ReleaseRenderTargets(void) {
		if (nullptr != this->m_RHIDevice) {
			// Make sure that all frames have finished rendering
			ASSERT(true == this->m_RHIDevice->WaitForIdle());

			// Release all in-flight references to the render targets
			this->m_RHIDevice->RunGarbageCollection();
		}

		// Set the events so that WaitForSingleObject in OneFrame will not hang later
		for (auto e : this->m_FrameFenceEvents)
			SetEvent(e);

		// Release the old buffers because ResizeBuffers requires that
		this->m_RHISwapChainBuffers.clear();
		this->m_SwapChainBuffers.clear();
	}

	inline void D3D12DeviceManager::ReportLiveObjects(void){
		RHI::RefCountPtr<IDXGIDebug> pDebug;
		DXGIGetDebugInterface1(0, PARTING_IID_PPV_ARGS(&pDebug));

		if (nullptr != pDebug) {
			DXGI_DEBUG_RLO_FLAGS flags{ static_cast<DXGI_DEBUG_RLO_FLAGS>(DXGI_DEBUG_RLO_IGNORE_INTERNAL | DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_DETAIL) };
			D3D12_CHECK(pDebug->ReportLiveObjects(DXGI_DEBUG_ALL, flags));
		}
	}


	//Imp 



	bool D3D12DeviceManager::Imp_CreateInstance(void) {
		D3D12_CHECK(CreateDXGIFactory2(this->m_DeviceParams.EnableDebugRuntime ? DXGI_CREATE_FACTORY_DEBUG : 0, PARTING_IID_PPV_ARGS(&this->m_DXFIFactory6)));

		return true;
	}

	inline bool D3D12DeviceManager::Imp_CreateDevice(void) {
		if (this->m_DeviceParams.EnableDebugRuntime) {
			RHI::RefCountPtr<ID3D12Debug> pDebug;
			D3D12_CHECK(D3D12GetDebugInterface(PARTING_IID_PPV_ARGS(&pDebug)));
			pDebug->EnableDebugLayer();
		}

		if (this->m_DeviceParams.EnableGPUValidation) {
			RHI::RefCountPtr<ID3D12Debug3> debugController3;
			D3D12_CHECK(D3D12GetDebugInterface(PARTING_IID_PPV_ARGS(&debugController3)));
			debugController3->SetEnableGPUBasedValidation(true);
		}

		D3D12_CHECK(this->m_DXFIFactory6->EnumAdapterByGpuPreference(this->m_DeviceParams.AdapterIndex, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&this->m_DXGIAdapter)));

		{
			DXGI_ADAPTER_DESC aDesc;
			this->m_DXGIAdapter->GetDesc(&aDesc);
			this->m_RendererString = WStringToString(WString{ aDesc.Description });
		}


		D3D12_CHECK(D3D12CreateDevice(this->m_DXGIAdapter, this->m_DeviceParams.FeatureLevel, IID_PPV_ARGS(&this->m_D3D12Device)));

		if (this->m_DeviceParams.EnableDebugRuntime) {
			RHI::RefCountPtr<ID3D12InfoQueue> pInfoQueue;
			D3D12_CHECK(this->m_D3D12Device->QueryInterface(&pInfoQueue));

			if (this->m_DeviceParams.EnableWarningsAsErrors)
				D3D12_CHECK(pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true));

			D3D12_CHECK(pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true));
			D3D12_CHECK(pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true));

			constexpr Array<D3D12_MESSAGE_ID, 3> disableMessageIDs{
				D3D12_MESSAGE_ID_CLEARDEPTHSTENCILVIEW_MISMATCHINGCLEARVALUE,
				D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
				D3D12_MESSAGE_ID_COMMAND_LIST_STATIC_DESCRIPTOR_RESOURCE_DIMENSION_MISMATCH, // descriptor validation doesn't understand acceleration structures
			};
			D3D12_INFO_QUEUE_FILTER filter{ .DenyList{.NumIDs{ static_cast<Uint32>(disableMessageIDs.size()) }, .pIDList{ const_cast<D3D12_MESSAGE_ID*>(disableMessageIDs.data()) } } };
			pInfoQueue->AddStorageFilterEntries(&filter);
		}

		D3D12_COMMAND_QUEUE_DESC queueDes{
			.Type { D3D12_COMMAND_LIST_TYPE_DIRECT },
			.Priority { D3D12_COMMAND_QUEUE_PRIORITY_NORMAL },
			.Flags { D3D12_COMMAND_QUEUE_FLAG_NONE },
			.NodeMask { 1 },
		};
		D3D12_CHECK(this->m_D3D12Device->CreateCommandQueue(&queueDes, IID_PPV_ARGS(&m_GraphicsQueue)));
		if (m_DeviceParams.EnableComputeQueue) {
			queueDes.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
			D3D12_CHECK(this->m_D3D12Device->CreateCommandQueue(&queueDes, IID_PPV_ARGS(&m_GraphicsQueue)));
		}

		if (m_DeviceParams.EnableCopyQueue) {
			queueDes.Type = D3D12_COMMAND_LIST_TYPE_COPY;
			D3D12_CHECK(this->m_D3D12Device->CreateCommandQueue(&queueDes, IID_PPV_ARGS(&m_GraphicsQueue)));
		}


		RHI::D3D12::D3D12DviceDesc deviceDesc{
			.Device {this->m_D3D12Device },
			.GraphicsQueue { this->m_GraphicsQueue },
			.ComputeQueue { this->m_ComputeQueue },
			.CopyQueue { this->m_CopyQueue },
		};

		deviceDesc.LogBufferLifetime = this->m_DeviceParams.LogBufferLifetime;
		deviceDesc.EnableHeapDirectlyIndexed = this->m_DeviceParams.EnableHeapDirectlyIndexed;

		this->m_RHIDevice = RHI::RefCountPtr<RHI::D3D12::Device>::Create(new RHI::D3D12::Device{ deviceDesc });

		return true;
	}

	inline bool D3D12DeviceManager::Imp_CreateSwapChain(void) {
		Uint32 windowStyle{
			this->m_DeviceParams.StartFullscreen ?
			(WS_POPUP | WS_SYSMENU | WS_VISIBLE)
			: (this->m_DeviceParams.StartMaximized ? (WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_MAXIMIZE) : (WS_OVERLAPPEDWINDOW | WS_VISIBLE))
		};

		RECT rect{ 0, 0, static_cast<LONG>(this->m_DeviceParams.BackBufferWidth), static_cast<LONG>(this->m_DeviceParams.BackBufferHeight) };
		AdjustWindowRect(&rect, windowStyle, FALSE);

		if (D3D12DeviceManager::MoveWindowOntoAdapter(this->m_DXGIAdapter, rect))
			glfwSetWindowPos(this->m_Window, rect.left, rect.top);

		this->m_hWnd = glfwGetWin32Window(m_Window);

		RECT clientRect;
		::GetClientRect(m_hWnd, &clientRect);
		Int32 width{ clientRect.right - clientRect.left };
		Int32 height{ clientRect.bottom - clientRect.top };

		this->m_SwapChainDesc = DXGI_SWAP_CHAIN_DESC1{};
		this->m_SwapChainDesc.Width = width;
		this->m_SwapChainDesc.Height = height;
		this->m_SwapChainDesc.SampleDesc.Count = this->m_DeviceParams.SwapChainSampleCount;
		this->m_SwapChainDesc.SampleDesc.Quality = 0;
		this->m_SwapChainDesc.BufferUsage = this->m_DeviceParams.SwapChainUsage;
		this->m_SwapChainDesc.BufferCount = this->m_DeviceParams.SwapChainBufferCount;
		this->m_SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		this->m_SwapChainDesc.Flags = this->m_DeviceParams.AllowModeSwitch ? DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH : 0;

		// Special processing for sRGB swap chain formats.
		// DXGI will not create a swap chain with an sRGB format, but its contents will be interpreted as sRGB.
		// So we need to use a non-sRGB format here, but store the true sRGB format for later framebuffer creation.
		switch (m_DeviceParams.SwapChainFormat) {
			using enum RHI::RHIFormat;
		case SRGBA8_UNORM:this->m_SwapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; break;
		case SBGRA8_UNORM:this->m_SwapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; break;
		default:this->m_SwapChainDesc.Format = RHI::D3D12::ConvertFormat(this->m_DeviceParams.SwapChainFormat); break;
		}

		RHI::RefCountPtr<IDXGIFactory5> pDxgiFactory5;
		if (D3D12_SUCCESS(this->m_DXFIFactory6->QueryInterface(PARTING_IID_PPV_ARGS(&pDxgiFactory5)))) {
			BOOL supported = 0;
			if (D3D12_SUCCESS(pDxgiFactory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &supported, sizeof(supported))))
				this->m_TearingSupported = (supported != 0);
		}

		if (this->m_TearingSupported)
			this->m_SwapChainDesc.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

		this->m_FullScreenDesc = DXGI_SWAP_CHAIN_FULLSCREEN_DESC{};
		this->m_FullScreenDesc.RefreshRate.Numerator = this->m_DeviceParams.RefreshRate;
		this->m_FullScreenDesc.RefreshRate.Denominator = 1;
		this->m_FullScreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
		this->m_FullScreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		this->m_FullScreenDesc.Windowed = !this->m_DeviceParams.StartFullscreen;

		RHI::RefCountPtr<IDXGISwapChain1> pSwapChain1;
		D3D12_CHECK(m_DXFIFactory6->CreateSwapChainForHwnd(this->m_GraphicsQueue, this->m_hWnd, &this->m_SwapChainDesc, &this->m_FullScreenDesc, nullptr, &pSwapChain1));

		D3D12_CHECK(pSwapChain1->QueryInterface(PARTING_IID_PPV_ARGS(&this->m_SwapChain)));

		if (false == CreateRenderTargets())
			return false;

		D3D12_CHECK(this->m_D3D12Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, PARTING_IID_PPV_ARGS(&this->m_FrameFence)));

		this->m_FrameFenceEvents.resize(this->m_FrameCount);
		for (auto& event : this->m_FrameFenceEvents)
			event = CreateEventW(nullptr, false, true, nullptr);

		return true;
	}

	inline void D3D12DeviceManager::Imp_ResizeSwapChain(void) {
		this->ReleaseRenderTargets();

		if (nullptr == this->m_RHIDevice)
			return;

		if (nullptr == this->m_SwapChain)
			return;

		D3D12_CHECK(this->m_SwapChain->ResizeBuffers(
			this->m_DeviceParams.SwapChainBufferCount,
			this->m_DeviceParams.BackBufferWidth,
			this->m_DeviceParams.BackBufferHeight,
			this->m_SwapChainDesc.Format,
			this->m_SwapChainDesc.Flags
		));

		if (false == CreateRenderTargets())
			LOG_ERROR("Failed to create swapchain buffers");
	}

	inline Uint32 D3D12DeviceManager::Imp_Get_BackBufferCount(void) {
		return this->m_SwapChainDesc.BufferCount;
	}

	inline void D3D12DeviceManager::Imp_DestroyDeviceAndSwapChain(void) {
		this->m_RendererString.clear();

		this->ReleaseRenderTargets();

		this->m_RHIDevice = nullptr;

		for (auto FenveEvent : this->m_FrameFenceEvents) {
			WaitForSingleObject(FenveEvent, 0xFFFFFFFF);
			CloseHandle(FenveEvent);
		}
		this->m_FrameFenceEvents.clear();

		if (nullptr != this->m_SwapChain)
			this->m_SwapChain->SetFullscreenState(false, nullptr);

		this->m_FrameFence = nullptr;
		this->m_SwapChain = nullptr;
		this->m_GraphicsQueue = nullptr;
		this->m_ComputeQueue = nullptr;
		this->m_CopyQueue = nullptr;
		this->m_D3D12Device = nullptr;
	}

	inline void D3D12DeviceManager::Imp_Shutdown(void){
		this->m_DXGIAdapter = nullptr;
		this->m_DXFIFactory6 = nullptr;

		if (m_DeviceParams.EnableDebugRuntime)
			this->ReportLiveObjects();
	}

	
}