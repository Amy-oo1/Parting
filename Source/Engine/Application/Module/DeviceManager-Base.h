#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"

PARTING_SUBMODULE(DeviceManager, Base)

PARTING_IMPORT GLFWWrapper;

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;
PARTING_IMPORT Container;
PARTING_IMPORT Logger;

PARTING_IMPORT RHI;


#else
#pragma once

#include "Core/ModuleBuild.h"


#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"
//Global
#include "Engine/Application/Module/GLFWWrapper.h"

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#include "Core/Container/Module/Container.h"
#include "Core/Logger/Module/Logger.h"

#include "RHI/Module/RHI.h"

#include "Core/Algorithm/Module/Algorithm.h"
#include "Core/VectorMath/Module/VectorMath.h"

#endif // PARTING_MODULE_BUILD


namespace Parting {

	struct FormatInfo final {
		RHI::RHIFormat Format;
		Uint32 RedBits;
		Uint32 GreenBits;
		Uint32 BlueBits;
		Uint32 AlphaBits;
		Uint32 DepthBits;
		Uint32 StencilBits;
	};

	HEADER_INLINE constexpr Array<FormatInfo, 27> WindowFormatInfos{
		FormatInfo{ RHI::RHIFormat::UNKNOWN,            0,  0,  0,  0,  0,  0, },
		FormatInfo{ RHI::RHIFormat::R8_UINT,            8,  0,  0,  0,  0,  0, },
		FormatInfo{ RHI::RHIFormat::RG8_UINT,           8,  8,  0,  0,  0,  0, },
		FormatInfo{ RHI::RHIFormat::RG8_UNORM,          8,  8,  0,  0,  0,  0, },
		FormatInfo{ RHI::RHIFormat::R16_UINT,          16,  0,  0,  0,  0,  0, },
		FormatInfo{ RHI::RHIFormat::R16_UNORM,         16,  0,  0,  0,  0,  0, },
		FormatInfo{ RHI::RHIFormat::R16_FLOAT,         16,  0,  0,  0,  0,  0, },
		FormatInfo{ RHI::RHIFormat::RGBA8_UNORM,        8,  8,  8,  8,  0,  0, },
		FormatInfo{ RHI::RHIFormat::RGBA8_SNORM,        8,  8,  8,  8,  0,  0, },
		FormatInfo{ RHI::RHIFormat::BGRA8_UNORM,        8,  8,  8,  8,  0,  0, },
		FormatInfo{ RHI::RHIFormat::SRGBA8_UNORM,       8,  8,  8,  8,  0,  0, },
		FormatInfo{ RHI::RHIFormat::SBGRA8_UNORM,       8,  8,  8,  8,  0,  0, },
		FormatInfo{ RHI::RHIFormat::R10G10B10A2_UNORM, 10, 10, 10,  2,  0,  0, },
		FormatInfo{ RHI::RHIFormat::R11G11B10_FLOAT,   11, 11, 10,  0,  0,  0, },
		FormatInfo{ RHI::RHIFormat::RG16_UINT,         16, 16,  0,  0,  0,  0, },
		FormatInfo{ RHI::RHIFormat::RG16_FLOAT,        16, 16,  0,  0,  0,  0, },
		FormatInfo{ RHI::RHIFormat::R32_UINT,          32,  0,  0,  0,  0,  0, },
		FormatInfo{ RHI::RHIFormat::R32_FLOAT,         32,  0,  0,  0,  0,  0, },
		FormatInfo{ RHI::RHIFormat::RGBA16_FLOAT,      16, 16, 16, 16,  0,  0, },
		FormatInfo{ RHI::RHIFormat::RGBA16_UNORM,      16, 16, 16, 16,  0,  0, },
		FormatInfo{ RHI::RHIFormat::RGBA16_SNORM,      16, 16, 16, 16,  0,  0, },
		FormatInfo{ RHI::RHIFormat::RG32_UINT,         32, 32,  0,  0,  0,  0, },
		FormatInfo{ RHI::RHIFormat::RG32_FLOAT,        32, 32,  0,  0,  0,  0, },
		FormatInfo{ RHI::RHIFormat::RGB32_UINT,        32, 32, 32,  0,  0,  0, },
		FormatInfo{ RHI::RHIFormat::RGB32_FLOAT,       32, 32, 32,  0,  0,  0, },
		FormatInfo{ RHI::RHIFormat::RGBA32_UINT,       32, 32, 32, 32,  0,  0, },
		FormatInfo{ RHI::RHIFormat::RGBA32_FLOAT,      32, 32, 32, 32,  0,  0, },
	};

	void ApplyDeadZone(Math::VecF2& v, float deadZone = 0.1f) { v *= Math::Max(Math::Length(v) - deadZone, 0.f) / (1.f - deadZone); }

	template<RHI::APITagConcept APITag>
	struct ManageTypeTraits;


	template<RHI::APITagConcept APITag>
	class IRenderPass;


	template<RHI::APITagConcept APITag>
	class JoyStickManager final :public ::MoveAbleOnly {
	private:
		JoyStickManager(void) = default;
	public:
		~JoyStickManager(void) = default;

		static JoyStickManager<APITag>* Get(void) {
			static JoyStickManager<APITag> singleton;

			return &singleton;
		}

	public:

		void UpdateAllJoysticks(const List<IRenderPass<APITag>*>& passes) {
			for (auto Id : this->m_JoystickIDs)
				this->UpdateJoystick(Id, passes);
		}

		void EraseDisconnectedJoysticks(void) {
			while (!this->m_RemovedJoysticks.empty()) {
				auto id{ m_RemovedJoysticks.back() };
				this->m_RemovedJoysticks.pop_back();

				if (auto it = ::STDFind(this->m_JoystickIDs.begin(), this->m_JoystickIDs.end(), id); it != this->m_JoystickIDs.end())
					this->m_JoystickIDs.erase(it);
			}
		}
		void EnumerateJoysticks(void) {
			// The glfw header says nothing about what values to expect for joystick IDs. Empirically, having connected two
			// simultaneously, glfw just seems to number them starting at 0.
			for (int Index = 0; Index < GLFW_JOYSTICK_LAST; ++Index)
				if (::glfwJoystickPresent(Index))
					this->m_JoystickIDs.push_back(Index);
		}

		void ConnectJoystick(Int32 id) { this->m_JoystickIDs.push_back(id); }
		void DisconnectJoystick(Int32 id) { this->m_RemovedJoysticks.push_back(id); }

	private:
		void UpdateJoystick(Uint32 j, const List<IRenderPass<APITag>*>& passes);//deferred

		List<Uint32> m_JoystickIDs, m_RemovedJoysticks;
	};

	PARTING_EXPORT struct InstanceParameters {
		bool EnableDebugRuntime{ false };
		bool EnableWarningsAsErrors{ false };
		bool EnablePerMonitorDPI{ false };

		//Dx
		bool EnableGPUValidation{ false };
	};

	PARTING_EXPORT struct DeviceCreationParameters final : public InstanceParameters {
		//NOTE :WIndow attributes
		bool StartMaximized{ false };	// ignores backbuffer width/height to be monitor size
		bool StartFullscreen{ false };
		bool StartBorderless{ false };
		bool AllowModeSwitch{ false };
		Int32 WindowPosX{ -1 };			// means use default placement
		Int32 WindowPosY{ -1 };
		Uint32 BackBufferWidth{ 1280 };
		Uint32 BackBufferHeight{ 720 };

		//NOTE :SwapChain attributes
		Uint32 SwapChainBufferCount{ 3 };
		RHI::RHIFormat SwapChainFormat{ RHI::RHIFormat::SRGBA8_UNORM };
		Uint32 SwapChainSampleCount{ 1 };
		Uint32 SwapChainSampleQuality{ 0 };

		Uint32 RefreshRate{ 0 };
		bool VsyncEnabled{ false };

		bool EnableComputeQueue{ false };
		bool EnableCopyQueue{ false };
		Uint8 AdapterIndex{ 0 };

		// Set this to true if the application implements UI scaling for DPI explicitly instead of relying
		// on ImGUI's DisplayFramebufferScale. This produces crisp text and lines at any scale
		// but requires considerable changes to applications that rely on the old behavior:
		// all UI sizes and offsets need to be computed as multiples of some scaled parameter,
		// such as ImGui::GetFontSize(). Note that the ImGUI style is automatically reset and scaled in 
		// ImGui_Renderer::DisplayScaleChanged(...).
		//
		// See ImGUI FAQ for more info:
		//   https://github.com/ocornut/imgui/blob/master/docs/FAQ.md#q-how-should-i-handle-dpi-in-my-application
		bool SupportExplicitDisplayScaling{ false };

		// Enables automatic resizing of the application window according to the DPI scaling of the monitor
		// that it is located on. When set to true and the app launches on a monitor with >100% scale, 
		// the initial window size will be larger than specified in 'backBufferWidth' and 'backBufferHeight' parameters.
		bool ResizeWindowWithDisplayScale{ false };

		//NOTE :Dx
		D3D_FEATURE_LEVEL FeatureLevel{ D3D_FEATURE_LEVEL_12_1 };
		DXGI_USAGE SwapChainUsage{ DXGI_USAGE_SHADER_INPUT | DXGI_USAGE_RENDER_TARGET_OUTPUT };

		//NOTE :Vk
		Uint32 MaxFramesInFlight{ 2 };
	};

	template<typename Derived, RHI::APITagConcept APITag>
	class DeviceManagerBase {
		using Imp_Texture = typename RHI::RHITypeTraits<APITag>::Imp_Texture;
		using Imp_FrameBuffer = typename RHI::RHITypeTraits<APITag>::Imp_FrameBuffer;
		using Imp_Device = typename RHI::RHITypeTraits<APITag>::Imp_Device;
		using Imp_CommandList = typename RHI::RHITypeTraits<APITag>::Imp_CommandList;
	protected:
		DeviceManagerBase(void) = default;

		PARTING_VIRTUAL ~DeviceManagerBase(void) = default;

	public:
		STDNODISCARD bool CreateWindowDeviceAndSwapChain(const DeviceCreationParameters& params, const char* windowTitle);

		STDNODISCARD bool CreateInstance(const InstanceParameters& params);

		void UpdateWindowSize(void);

		void AddRenderPassToBack(IRenderPass<APITag>* pRenderPass);

		void RemoveRenderPass(IRenderPass<APITag>* pRenderPass);

		void Shutdown(void);

		void RunMessageLoop(void);

		void Set_VsyncEnabled(bool enabled) { this->m_RequestedVSync = enabled; }


	public:
		STDNODISCARD Uint32 Get_FrameIndex(void) const { return this->m_FrameIndex; }

		STDNODISCARD DeviceCreationParameters Get_DeviceParams(void) const { return this->m_DeviceParams; }

		STDNODISCARD Tuple<float, float> Get_DPIScaleInfo(void) const { return { this->m_DPIScaleFactorX, this->m_DPIScaleFactorY }; }

		STDNODISCARD Tuple<Uint32, Uint32> Get_WindowDimensions(void)const { return { this->m_DeviceParams.BackBufferWidth, this->m_DeviceParams.BackBufferHeight }; }


		STDNODISCARD double Get_AverageFrameTimeSeconds(void) const { return this->m_AverageFrameTime; }

		STDNODISCARD double Get_PreviousFrameTimestamp(void) const { return this->m_PreviousFrameTimestamp; }

		void Set_WindowTitle(const StringView& title);

		void Set_InformativeWindowTitle(const StringView& applicationName, bool includeFramerate = true, const StringView& extraInfo = StringView{});

	protected:
		void Animate(double elapsedTime);

		void Render(void);

		void DisplayScaleChanged(void);

		bool ShouldRenderUnfocused(void);

		bool AnimateRenderPresent(void);

		void UpdateAverageFrameTime(double elapsedTime);

		void BackBufferResizing(void);

		void BackBufferResized(void);

	protected:
		void WindowPosCallback(Int32 x, Int32 y);
		void WindowCloseCallback(void) { LOG_INFO("TODO"); }
		void WindowRefreshCallback() { LOG_INFO("TODO"); }
		void WindowFocusCallback(Int32 focused) { LOG_INFO("TODO"); }
		void WindowIconifyCallback(Int32 iconified) { LOG_INFO("TODO"); }
		void KeyboardUpdate(Int32 key, Int32 scancode, Int32 action, Int32 mods);
		void KeyboardCharInput(Uint32 unicode, Int32 mods);
		void MousePosUpdate(double xpos, double ypos);
		void MouseButtonUpdate(Int32 button, Int32 action, Int32 mods);
		void MouseScrollUpdate(double xoffset, double yoffset);

	protected:
		static void ErrorCallback_GLFW(Int32 error, const char* description) { LOG_ERROR("GLFW Error"); }
		static void WindowPosCallback_GLFW(GLFWwindow* window, Int32 xpos, Int32 ypos) { reinterpret_cast<DeviceManagerBase<Derived, APITag>*>(::glfwGetWindowUserPointer(window))->WindowPosCallback(xpos, ypos); }
		static void WindowCloseCallback_GLFW(GLFWwindow* window) { reinterpret_cast<DeviceManagerBase<Derived, APITag>*>(::glfwGetWindowUserPointer(window))->WindowCloseCallback(); }
		static void WindowRefreshCallback_GLFW(GLFWwindow* window) { reinterpret_cast<DeviceManagerBase<Derived, APITag>*>(::glfwGetWindowUserPointer(window))->WindowRefreshCallback(); }
		static void WindowFocusCallback_GLFW(GLFWwindow* window, Int32 focused) { reinterpret_cast<DeviceManagerBase<Derived, APITag>*>(::glfwGetWindowUserPointer(window))->WindowFocusCallback(focused); }
		static void WindowIconifyCallback_GLFW(GLFWwindow* window, Int32 iconified) { reinterpret_cast<DeviceManagerBase<Derived, APITag>*>(::glfwGetWindowUserPointer(window))->WindowIconifyCallback(iconified); }
		static void KeyCallback_GLFW(GLFWwindow* window, Int32 key, Int32 scancode, Int32 action, Int32 mods) { reinterpret_cast<DeviceManagerBase<Derived, APITag>*>(::glfwGetWindowUserPointer(window))->KeyboardUpdate(key, scancode, action, mods); }
		static void CharModsCallback_GLFW(GLFWwindow* window, Uint32 unicode, Int32 mods) { reinterpret_cast<DeviceManagerBase<Derived, APITag>*>(::glfwGetWindowUserPointer(window))->KeyboardCharInput(unicode, mods); }
		static void MousePosCallback_GLFW(GLFWwindow* window, double xpos, double ypos) { reinterpret_cast<DeviceManagerBase<Derived, APITag>*>(::glfwGetWindowUserPointer(window))->MousePosUpdate(xpos, ypos); }
		static void MouseButtonCallback_GLFW(GLFWwindow* window, int button, Int32 action, Int32 mods) { reinterpret_cast<DeviceManagerBase<Derived, APITag>*>(::glfwGetWindowUserPointer(window))->MouseButtonUpdate(button, action, mods); }
		static void MouseScrollCallback_GLFW(GLFWwindow* window, double xoffset, double yoffset) { reinterpret_cast<DeviceManagerBase<Derived, APITag>*>(::glfwGetWindowUserPointer(window))->MouseScrollUpdate(xoffset, yoffset); }
		static void JoystickConnectionCallback_GLFW(Int32 joyId, Int32 connectDisconnect) {
			if (GLFW_CONNECTED == connectDisconnect)
				JoyStickManager<APITag>::Get()->ConnectJoystick(joyId);
			if (GLFW_DISCONNECTED == connectDisconnect)
				JoyStickManager<APITag>::Get()->DisconnectJoystick(joyId);
		}

	public:
		bool CreateDevice(void) { return this->Get_Derived()->Imp_CreateDevice(); }
		bool CreateSwapChain(void) { return this->Get_Derived()->Imp_CreateSwapChain(); }
		void ResizeSwapChain(void) { this->Get_Derived()->Imp_ResizeSwapChain(); }
		Uint32 Get_BackBufferCount(void) { return this->Get_Derived()->Imp_Get_BackBufferCount(); }
		Imp_Device* Get_Device(void) { return this->Get_Derived()->Imp_Get_Device(); }
		Imp_Texture* Get_BackBuffer(Uint32 index) { return this->Get_Derived()->Imp_Get_BackBuffer(index); }
		bool BeginFrame(void) { return this->Get_Derived()->Imp_BeginFrame(); }
		bool Present(void) { return this->Get_Derived()->Imp_Present(); }
		Uint32 Get_CurrentBackBufferIndex(void) { return this->Get_Derived()->Imp_Get_CurrentBackBufferIndex(); }

		void DestroyDeviceAndSwapChain(void) { this->Get_Derived()->Imp_DestroyDeviceAndSwapChain(); }
	protected:
		// useful for apps that require 2 frames worth of simulation data before first render
		// apps should extend the DeviceManager classes, and constructor initialized this to true to opt in to the behavior
		//NOTE : Instance
		DeviceCreationParameters m_DeviceParams;
		bool m_InstanceCreated{ false };

		//NOTE :Window
		GLFWwindow* m_Window{ nullptr };
		String m_WindowTitle;
		bool m_WindowVisible{ false };
		bool m_WindowIsInFocus{ true };
		bool m_EnableRenderDuringWindowMovement{ false };

		//NOTE : DPI current DPI scale info (updated when window moves)
		float m_DPIScaleFactorX{ 1.f };
		float m_DPIScaleFactorY{ 1.f };
		float m_PrevDPIScaleFactorX{ 0.f };
		float m_PrevDPIScaleFactorY{ 0.f };
		bool m_RequestedVSync{ false };

		//NOTE :Time timestamp in seconds for the previous frame
		double m_PreviousFrameTimestamp{ 0.0 };
		double m_AverageFrameTime{ 0.0 };
		double m_AverageTimeUpdateInterval{ 0.5 };
		double m_FrameTimeSum{ 0.0 };
		Uint32 m_NumberOfAccumulatedFrames{ 0 };

		//NOTE :Frame
		Uint32 m_FrameIndex{ 0 };
		Vector<RHI::RefCountPtr<Imp_FrameBuffer>> m_SwapChainFrameBuffers;

		//NOTE :Render
		bool m_SkipRenderOnFirstFrame{ false };
		List<IRenderPass<APITag>*> m_vRenderPasses;

		// GetFrameIndex cannot be used inside of these callbacks, hence the additional passing of frameID
	   // Refer to AnimateRenderPresent implementation for more details
		struct PipelineCallbacks {
			Function<void(DeviceManagerBase&, Uint32)> BeforeFrame{ nullptr };
			Function<void(DeviceManagerBase&, Uint32)> BeforeAnimate{ nullptr };
			Function<void(DeviceManagerBase&, Uint32)> AfterAnimate{ nullptr };
			Function<void(DeviceManagerBase&, Uint32)> BeforeRender{ nullptr };
			Function<void(DeviceManagerBase&, Uint32)> AfterRender{ nullptr };
			Function<void(DeviceManagerBase&, Uint32)> BeforePresent{ nullptr };
			Function<void(DeviceManagerBase&, Uint32)> AfterPresent{ nullptr };
		} m_Callbacks;

	private:
		STDNODISCARD Derived* Get_Derived(void)noexcept { return static_cast<Derived*>(this); }
		STDNODISCARD const Derived* Get_Derived(void)const noexcept { return static_cast<const Derived*>(this); }

	private:
		bool Imp_CreateInstance(void) { LOG_ERROR("No Imp"); return false; }
		bool Imp_CreateDevice(void) { LOG_ERROR("No Imp"); return false; }
		bool Imp_CreateSwapChain(void) { LOG_ERROR("No Imp"); return false; }
		void Imp_ResizeSwapChain(void) { LOG_ERROR("No Imp"); }
		Uint32 Imp_Get_BackBufferCount(void) { LOG_ERROR("No Imp"); return 0; }
		Imp_Device* Imp_Get_Device(void) { LOG_ERROR("No Imp"); return nullptr; }
		bool Imp_BeginFrame(void) { LOG_ERROR("No Imp"); return false; }
		bool Imp_Present(void) { LOG_ERROR("No Imp"); return false; }
		Imp_Texture* Imp_Get_BackBuffer(Uint32 index) { LOG_ERROR("No Imp"); return nullptr; }
		Uint32 Imp_Get_CurrentBackBufferIndex(void) { LOG_ERROR("No Imp"); return 0; }
	};

	template<RHI::APITagConcept APITag>
	class IRenderPass {
		using Imp_FrameBuffer = typename RHI::RHITypeTraits<APITag>::Imp_FrameBuffer;
		using DeviceManager = typename ManageTypeTraits<APITag>::DeviceManager;
	public:
		IRenderPass(DeviceManager* devicemamage) : m_DeviceManager{ devicemamage } {}
		virtual ~IRenderPass(void) = default;

	public:
		STDNODISCARD Uint32 Get_FrameIndex(void) const { return this->m_DeviceManager->Get_FrameIndex(); }


	public:
		virtual void SetLatewarpOptions(void) {}
		virtual bool ShouldRenderUnfocused(void) { return false; }
		virtual void Render(Imp_FrameBuffer* framebuffer) {}
		virtual void Animate(float fElapsedTimeSeconds) {}
		virtual void BackBufferResizing(void) {}
		virtual void BackBufferResized(Uint32 width, Uint32 height, Uint32 sampleCount) {}

		// Called before Animate() when a DPI change was detected
		virtual void DisplayScaleChanged(float scaleX, float scaleY) {}


		virtual bool KeyboardUpdate(int key, int scancode, int action, int mods) { return false; }
		virtual bool KeyboardCharInput(unsigned int unicode, int mods) { return false; }
		virtual bool MousePosUpdate(double xpos, double ypos) { return false; }
		virtual bool MouseScrollUpdate(double xoffset, double yoffset) { return false; }
		virtual bool MouseButtonUpdate(int button, int action, int mods) { return false; }
		virtual bool JoystickButtonUpdate(Uint32 button, bool pressed) { return false; }
		virtual bool JoystickAxisUpdate(Uint32 axis, float value) { return false; }

	protected:
		DeviceManager* m_DeviceManager{ nullptr };
	};



	template<RHI::APITagConcept APITag>
	inline void JoyStickManager<APITag>::UpdateJoystick(Uint32 j, const List<IRenderPass<APITag>*>& passes) {
		GLFWgamepadstate gamepadState;
		::glfwGetGamepadState(j, &gamepadState);

		float* axisValues{ gamepadState.axes };

		auto updateAxis = [&](Uint32 axis, float axisVal) {
			for (auto& Pass : passes)
				if (Pass->JoystickAxisUpdate(axis, axisVal))
					break;
			};

		{
			Math::VecF2 v{ axisValues[GLFW_GAMEPAD_AXIS_LEFT_X], axisValues[GLFW_GAMEPAD_AXIS_LEFT_Y] };
			ApplyDeadZone(v);
			updateAxis(GLFW_GAMEPAD_AXIS_LEFT_X, v.X);
			updateAxis(GLFW_GAMEPAD_AXIS_LEFT_Y, v.Y);
		}

		{
			Math::VecF2 v{ axisValues[GLFW_GAMEPAD_AXIS_RIGHT_X], axisValues[GLFW_GAMEPAD_AXIS_RIGHT_Y] };
			ApplyDeadZone(v);
			updateAxis(GLFW_GAMEPAD_AXIS_RIGHT_X, v.X);
			updateAxis(GLFW_GAMEPAD_AXIS_RIGHT_Y, v.Y);
		}

		updateAxis(GLFW_GAMEPAD_AXIS_LEFT_TRIGGER, axisValues[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER]);
		updateAxis(GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER, axisValues[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER]);

		for (Uint32 buttonIndex = 0; const auto & button : gamepadState.buttons) {
			for (auto& pass : passes)
				if (pass->JoystickButtonUpdate(buttonIndex, GLFW_PRESS == button))
					break;

			++buttonIndex;
		}
	}





	template<typename Derived, RHI::APITagConcept APITag>
	inline bool DeviceManagerBase<Derived, APITag>::CreateWindowDeviceAndSwapChain(const DeviceCreationParameters& params, const char* windowTitle) {
		this->m_DeviceParams = params;
		this->m_RequestedVSync = params.VsyncEnabled;

		if (false == this->CreateInstance(this->m_DeviceParams))
			return false;

		::glfwSetErrorCallback(DeviceManagerBase::ErrorCallback_GLFW);

		::glfwDefaultWindowHints();

		bool foundFormat{ false };
		for (const auto& info : WindowFormatInfos)
			if (info.Format == params.SwapChainFormat) {
				::glfwWindowHint(GLFW_RED_BITS, info.RedBits);
				::glfwWindowHint(GLFW_GREEN_BITS, info.GreenBits);
				::glfwWindowHint(GLFW_BLUE_BITS, info.BlueBits);
				::glfwWindowHint(GLFW_ALPHA_BITS, info.AlphaBits);
				::glfwWindowHint(GLFW_DEPTH_BITS, info.DepthBits);
				::glfwWindowHint(GLFW_STENCIL_BITS, info.StencilBits);
				foundFormat = true;
				break;
			}

		ASSERT(foundFormat);

		::glfwWindowHint(GLFW_SAMPLES, params.SwapChainSampleCount);
		::glfwWindowHint(GLFW_REFRESH_RATE, params.RefreshRate);
		::glfwWindowHint(GLFW_SCALE_TO_MONITOR, params.ResizeWindowWithDisplayScale);

		::glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		::glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);   // Ignored for fullscreen
		if (params.StartBorderless)
			::glfwWindowHint(GLFW_DECORATED, GLFW_FALSE); // Borderless window

		this->m_Window = ::glfwCreateWindow(
			params.BackBufferWidth, params.BackBufferHeight,
			windowTitle ? windowTitle : "",
			params.StartFullscreen ? ::glfwGetPrimaryMonitor() : nullptr,
			nullptr
		);

		ASSERT(nullptr != m_Window);

		if (params.StartFullscreen)
			::glfwSetWindowMonitor(
				this->m_Window,
				::glfwGetPrimaryMonitor(),
				0, 0,
				this->m_DeviceParams.BackBufferWidth,
				this->m_DeviceParams.BackBufferHeight,
				this->m_DeviceParams.RefreshRate
			);
		else {
			Int32 fbWidth{ 0 }, fbHeight{ 0 };
			::glfwGetFramebufferSize(this->m_Window, &fbWidth, &fbHeight);
			m_DeviceParams.BackBufferWidth = fbWidth;
			m_DeviceParams.BackBufferHeight = fbHeight;
		}

		if (nullptr != windowTitle)
			this->m_WindowTitle = windowTitle;

		::glfwSetWindowUserPointer(this->m_Window, this);

		if (-1 != params.WindowPosX && -1 != params.WindowPosY)
			::glfwSetWindowPos(this->m_Window, params.WindowPosX, params.WindowPosY);

		::glfwSetWindowPosCallback(this->m_Window, DeviceManagerBase::WindowPosCallback_GLFW);
		::glfwSetWindowCloseCallback(this->m_Window, DeviceManagerBase::WindowCloseCallback_GLFW);
		::glfwSetWindowFocusCallback(this->m_Window, DeviceManagerBase::WindowFocusCallback_GLFW);
		::glfwSetWindowRefreshCallback(this->m_Window, DeviceManagerBase::WindowRefreshCallback_GLFW);
		::glfwSetWindowIconifyCallback(this->m_Window, DeviceManagerBase::WindowIconifyCallback_GLFW);
		::glfwSetKeyCallback(this->m_Window, DeviceManagerBase::KeyCallback_GLFW);
		::glfwSetCharModsCallback(this->m_Window, DeviceManagerBase::CharModsCallback_GLFW);
		::glfwSetCursorPosCallback(this->m_Window, DeviceManagerBase::MousePosCallback_GLFW);
		::glfwSetMouseButtonCallback(this->m_Window, DeviceManagerBase::MouseButtonCallback_GLFW);
		::glfwSetScrollCallback(this->m_Window, DeviceManagerBase::MouseScrollCallback_GLFW);
		::glfwSetJoystickCallback(DeviceManagerBase::JoystickConnectionCallback_GLFW);

		JoyStickManager<APITag>::Get()->EnumerateJoysticks();

		if (false == this->CreateDevice())
			return false;

		if (false == this->CreateSwapChain())
			return false;

		::glfwShowWindow(this->m_Window);

		if (this->m_DeviceParams.StartMaximized)
			glfwMaximizeWindow(this->m_Window);

		// reset the back buffer size state to enforce a resize event
		this->m_DeviceParams.BackBufferWidth = 0;
		this->m_DeviceParams.BackBufferHeight = 0;

		this->UpdateWindowSize();

		return true;
	}

	template<typename Derived, RHI::APITagConcept APITag>
	inline bool DeviceManagerBase<Derived, APITag>::CreateInstance(const InstanceParameters& params) {
		ASSERT(false == this->m_InstanceCreated);//TODO :Debug

		static_cast<InstanceParameters&>(this->m_DeviceParams) = params;

		if (!params.EnablePerMonitorDPI)
			SetProcessDpiAwareness(PROCESS_DPI_UNAWARE);//NOTE : EXTERN_C

		if (false == glfwInit())
			return false;


		return this->m_InstanceCreated = this->Get_Derived()->Imp_CreateInstance();
	}

	template<typename Derived, RHI::APITagConcept APITag>
	inline void DeviceManagerBase<Derived, APITag>::UpdateWindowSize(void) {
		Int32 width;
		Int32 height;
		::glfwGetWindowSize(this->m_Window, &width, &height);

		if (0 == width || 0 == height) {
			// window is minimized
			this->m_WindowVisible = false;
			return;
		}

		this->m_WindowVisible = true;

		this->m_WindowIsInFocus = (1 == ::glfwGetWindowAttrib(this->m_Window, GLFW_FOCUSED));

		if (static_cast<Int32>(this->m_DeviceParams.BackBufferWidth) != width ||
			static_cast<Int32>(this->m_DeviceParams.BackBufferHeight) != height) {
			// window is not minimized, and the size has changed

			this->BackBufferResizing();

			this->m_DeviceParams.BackBufferWidth = static_cast<Uint32>(width);
			this->m_DeviceParams.BackBufferHeight = static_cast<Uint32>(height);
			this->m_DeviceParams.VsyncEnabled = this->m_RequestedVSync;

			this->ResizeSwapChain();
			this->BackBufferResized();
		}

		this->m_DeviceParams.VsyncEnabled = this->m_RequestedVSync;
	}

	template<typename Derived, RHI::APITagConcept APITag>
	inline void DeviceManagerBase<Derived, APITag>::AddRenderPassToBack(IRenderPass<APITag>* pRenderPass) {
		this->m_vRenderPasses.remove(pRenderPass);
		this->m_vRenderPasses.push_back(pRenderPass);

		pRenderPass->BackBufferResizing();
		pRenderPass->BackBufferResized(this->m_DeviceParams.BackBufferWidth, this->m_DeviceParams.BackBufferHeight, this->m_DeviceParams.SwapChainSampleCount);
	}

	template<typename Derived, RHI::APITagConcept APITag>
	inline void DeviceManagerBase<Derived, APITag>::RemoveRenderPass(IRenderPass<APITag>* pRenderPass) {
		ASSERT(nullptr != pRenderPass);
		this->m_vRenderPasses.remove(pRenderPass);
	}

	template<typename Derived, RHI::APITagConcept APITag>
	inline void DeviceManagerBase<Derived, APITag>::Shutdown(void) {
		this->m_SwapChainFrameBuffers.clear();

		this->DestroyDeviceAndSwapChain();

		if (nullptr != this->m_Window) {
			::glfwDestroyWindow(this->m_Window);
			this->m_Window = nullptr;
		}

		if (this->m_InstanceCreated) {
			::glfwTerminate();
			this->m_InstanceCreated = false;
		}

		this->Get_Derived()->Imp_Shutdown();
	}

	template<typename Derived, RHI::APITagConcept APITag>
	inline void DeviceManagerBase<Derived, APITag>::RunMessageLoop(void) {
		this->m_PreviousFrameTimestamp = glfwGetTime();

		while (!glfwWindowShouldClose(this->m_Window)) {
			if (nullptr != this->m_Callbacks.BeforeFrame)
				this->m_Callbacks.BeforeFrame(*this, this->m_FrameIndex);

			glfwPollEvents();
			this->UpdateWindowSize();

			if (!this->AnimateRenderPresent())
				break;

			this->Get_Device()->WaitForIdle();
		}
	}

	template<typename Derived, RHI::APITagConcept APITag>
	inline void DeviceManagerBase<Derived, APITag>::Set_WindowTitle(const StringView& title) {
		ASSERT(!title.empty());
		if (title == this->m_WindowTitle)
			return;

		glfwSetWindowTitle(this->m_Window, title.data());

		this->m_WindowTitle = title;
	}

	template<typename Derived, RHI::APITagConcept APITag>
	inline void DeviceManagerBase<Derived, APITag>::Set_InformativeWindowTitle(const StringView& applicationName, bool includeFramerate, const StringView& extraInfo) {
		StringStream ss;
		ss << applicationName;
		ss << " (" << RHI::RHITypeTraits<APITag>::APIName;

		if (this->m_DeviceParams.EnableDebugRuntime)
			ss << " ,Debug";

		ss << ")";

		const double frameTime{ this->Get_AverageFrameTimeSeconds() };
		if (includeFramerate && frameTime > 0) {
			const double fps{ 1.0 / frameTime };
			const Uint32 precision{ (fps <= 20.0) ? 1u : 0u };
			ss << " - " << std::fixed << std::setprecision(precision) << fps << " FPS ";
		}

		if (!extraInfo.empty())
			ss << extraInfo;

		this->Set_WindowTitle(ss.str());
	}

	template<typename Derived, RHI::APITagConcept APITag>
	inline void DeviceManagerBase<Derived, APITag>::Animate(double elapsedTime) {
		for (IRenderPass<APITag>* pass : this->m_vRenderPasses) {
			pass->Animate(static_cast<float>(elapsedTime));
			pass->SetLatewarpOptions();
		}
	}

	template<typename Derived, RHI::APITagConcept APITag>
	inline void DeviceManagerBase<Derived, APITag>::Render(void) {
		Imp_FrameBuffer* framebuffer{ this->m_SwapChainFrameBuffers[this->Get_CurrentBackBufferIndex()] };

		for (IRenderPass<APITag>* it : this->m_vRenderPasses)
			it->Render(framebuffer);
	}

	template<typename Derived, RHI::APITagConcept APITag>
	inline void DeviceManagerBase<Derived, APITag>::DisplayScaleChanged(void) {
		for (IRenderPass<APITag>* pass : this->m_vRenderPasses)
			pass->DisplayScaleChanged(this->m_DPIScaleFactorX, this->m_DPIScaleFactorY);
	}

	template<typename Derived, RHI::APITagConcept APITag>
	inline bool DeviceManagerBase<Derived, APITag>::ShouldRenderUnfocused(void) {
		for (IRenderPass<APITag>* pass : this->m_vRenderPasses)
			if (pass->ShouldRenderUnfocused())
				return true;
		return false;
	}

	template<typename Derived, RHI::APITagConcept APITag>
	inline bool DeviceManagerBase<Derived, APITag>::AnimateRenderPresent(void) {
		double CurrentTime{ ::glfwGetTime() };
		double DeltaTime{ CurrentTime - this->m_PreviousFrameTimestamp };

		JoyStickManager<APITag>::Get()->EraseDisconnectedJoysticks();
		JoyStickManager<APITag>::Get()->UpdateAllJoysticks(this->m_vRenderPasses);

		if (this->m_WindowVisible && (this->m_WindowIsInFocus || this->ShouldRenderUnfocused())) {
			if (this->m_PrevDPIScaleFactorX != this->m_DPIScaleFactorX || this->m_PrevDPIScaleFactorY != this->m_DPIScaleFactorY) {
				this->DisplayScaleChanged();
				this->m_PrevDPIScaleFactorX = this->m_DPIScaleFactorX;
				this->m_PrevDPIScaleFactorY = this->m_DPIScaleFactorY;
			}

			if (nullptr != this->m_Callbacks.BeforeAnimate)
				this->m_Callbacks.BeforeAnimate(*this, this->m_FrameIndex);
			this->Animate(DeltaTime);
			if (nullptr != this->m_Callbacks.AfterAnimate)
				this->m_Callbacks.AfterAnimate(*this, this->m_FrameIndex);

			// normal rendering           : A0    R0 P0 A1 R1 P1
			// m_SkipRenderOnFirstFrame on: A0 A1 R0 P0 A2 R1 P1
			// m_SkipRenderOnFirstFrame simulates multi-threaded rendering frame indices, m_FrameIndex becomes the simulation index
			// while the local variable below becomes the render/present index, which will be different only if m_SkipRenderOnFirstFrame is set
			if ((this->m_FrameIndex > 0 || !this->m_SkipRenderOnFirstFrame) && this->BeginFrame()) {
				// first time entering this loop, m_FrameIndex is 1 for m_SkipRenderOnFirstFrame, 0 otherwise;
				auto frameIndex{ this->m_FrameIndex };

				if (this->m_SkipRenderOnFirstFrame)
					--frameIndex;

				if (nullptr != this->m_Callbacks.BeforeRender)
					this->m_Callbacks.BeforeRender(*this, frameIndex);
				this->Render();
				if (this->m_Callbacks.AfterRender)
					this->m_Callbacks.AfterRender(*this, frameIndex);
				if (m_Callbacks.BeforePresent)
					this->m_Callbacks.BeforePresent(*this, frameIndex);
				bool presentSuccess{ this->Present() };
				if (this->m_Callbacks.AfterPresent)
					this->m_Callbacks.AfterPresent(*this, frameIndex);
				if (!presentSuccess)
					return false;
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(0));

		this->Get_Device()->RunGarbageCollection();

		this->UpdateAverageFrameTime(DeltaTime);
		this->m_PreviousFrameTimestamp = CurrentTime;

		++m_FrameIndex;
		return true;

	}

	template<typename Derived, RHI::APITagConcept APITag>
	inline void DeviceManagerBase<Derived, APITag>::UpdateAverageFrameTime(double elapsedTime) {
		this->m_FrameTimeSum += elapsedTime;
		++this->m_NumberOfAccumulatedFrames;
		if (this->m_FrameTimeSum > this->m_AverageTimeUpdateInterval && this->m_NumberOfAccumulatedFrames > 0) {
			this->m_AverageFrameTime = this->m_FrameTimeSum / static_cast<double>(this->m_NumberOfAccumulatedFrames);
			this->m_FrameTimeSum = 0.0;
			this->m_NumberOfAccumulatedFrames = 0;
		}
	}

	template<typename Derived, RHI::APITagConcept APITag>
	inline void DeviceManagerBase<Derived, APITag>::BackBufferResizing(void) {
		this->m_SwapChainFrameBuffers.clear();
		for (IRenderPass<APITag>* pass : this->m_vRenderPasses)
			pass->BackBufferResizing();
	}

	template<typename Derived, RHI::APITagConcept APITag>
	inline void DeviceManagerBase<Derived, APITag>::BackBufferResized(void) {
		for (IRenderPass<APITag>* pass : this->m_vRenderPasses)
			pass->BackBufferResized(this->m_DeviceParams.BackBufferWidth, this->m_DeviceParams.BackBufferHeight, this->m_DeviceParams.SwapChainSampleCount);

		Uint32 backBufferCount{ this->Get_BackBufferCount() };
		ASSERT(backBufferCount > 0);
		this->m_SwapChainFrameBuffers.resize(backBufferCount);

		for (Uint32 index = 0; index < backBufferCount; ++index)
			this->m_SwapChainFrameBuffers[index] = this->Get_Device()->CreateFrameBuffer(RHI::RHIFrameBufferDescBuilder<APITag>{}.AddColorAttachment(this->Get_BackBuffer(index)).Build());
	}

	template<typename Derived, RHI::APITagConcept APITag>
	inline void DeviceManagerBase<Derived, APITag>::WindowPosCallback(Int32 x, Int32 y) {
		if (this->m_DeviceParams.EnablePerMonitorDPI) {
			// Use Windows-specific implementation of DPI query because GLFW has issues:
			// glfwGetWindowMonitor(window) returns NULL for non-fullscreen applications.
			// This custom code allows us to adjust DPI scaling when a window is moved
			// between monitors with different scales.

			HWND hwnd{ ::glfwGetWin32Window(this->m_Window) };
			auto monitor{ ::MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST) };

			Uint32 dpiX;
			Uint32 dpiY;
			GetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY);//Ex C

			this->m_DPIScaleFactorX = dpiX / 96.f;
			this->m_DPIScaleFactorY = dpiY / 96.f;

		}

		if (this->m_EnableRenderDuringWindowMovement && this->m_SwapChainFrameBuffers.size() > 0) {
			if (nullptr != this->m_Callbacks.BeforeFrame)
				this->m_Callbacks.BeforeFrame(*this, m_FrameIndex);
			this->AnimateRenderPresent();
		}
	}

	template<typename Derived, RHI::APITagConcept APITag>
	inline void DeviceManagerBase<Derived, APITag>::KeyboardUpdate(Int32 key, Int32 scancode, Int32 action, Int32 mods) {
		if (-1 == key)
			return;

		for (IRenderPass<APITag>* pass : this->m_vRenderPasses)
			if (pass->KeyboardUpdate(key, scancode, action, mods))
				break;
	}

	template<typename Derived, RHI::APITagConcept APITag>
	inline void DeviceManagerBase<Derived, APITag>::KeyboardCharInput(Uint32 unicode, Int32 mods) {
		for (IRenderPass<APITag>* pass : this->m_vRenderPasses)
			if (pass->KeyboardCharInput(unicode, mods))
				break;
	}

	template<typename Derived, RHI::APITagConcept APITag>
	inline void DeviceManagerBase<Derived, APITag>::MousePosUpdate(double xpos, double ypos) {
		if (!this->m_DeviceParams.SupportExplicitDisplayScaling) {
			xpos /= this->m_DPIScaleFactorX;
			ypos /= this->m_DPIScaleFactorY;
		}

		for (IRenderPass<APITag>* pass : this->m_vRenderPasses)
			if (pass->MousePosUpdate(xpos, ypos))
				break;
	}

	template<typename Derived, RHI::APITagConcept APITag>
	inline void DeviceManagerBase<Derived, APITag>::MouseButtonUpdate(Int32 button, Int32 action, Int32 mods) {
		for (IRenderPass<APITag>* pass : this->m_vRenderPasses)
			if (pass->MouseButtonUpdate(button, action, mods))
				break;
	}

	template<typename Derived, RHI::APITagConcept APITag>
	inline void DeviceManagerBase<Derived, APITag>::MouseScrollUpdate(double xoffset, double yoffset) {
		for (IRenderPass<APITag>* pass : this->m_vRenderPasses)
			if (pass->MouseScrollUpdate(xoffset, yoffset))
				break;
	}



}