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
#include "Engine/Application/Module/DeviceManager.h"

#include "Engine/Render/Module/UIRender-RHI.h"

#endif // PARTING_MODULE_BUILD

namespace Parting {

	class RegisteredFont final {
		template<RHI::APITagConcept APITag>
		friend class UIRenderer;

	public:
		// Creates an invalid font that will not add any ImGUI fonts
		RegisteredFont(void) = default;

		// Creates a default font with the given size
		explicit RegisteredFont(float size) :
			m_IsDefault{ true },
			m_SizeAtDefaultScale{ size } {
		}

		// Creates a custom font
		RegisteredFont(SharedPtr<IBlob> data, bool isCompressed, float size) :
			m_Data{ data },
			m_IsCompressed{ isCompressed },
			m_SizeAtDefaultScale{ size }
		{
		}
		~RegisteredFont(void) = default;

	public:
		void CreateScaledFont(float displayScale);

		void ReleaseScaledFont(void) { this->m_ImGuiFont = nullptr; }


		// Returns true if the custom font data has been successfully loaded.
		// This doesn't necessarily mean that the font data is valid: the actual font object is only created
		// in the first call to ImGui_Renderer::Animate(...). After that, use Get_ScaledFont()
		// to test if the font is valid.
		bool HasFontData(void) const { return this->m_Data != nullptr; }

		// Returns the ImFont object that can be used with ImGUI.
		// Note that the returned pointer is transient and will change when screen DPI changes,
		// or when new fonts are loaded. Do not cache the returned value between frames.
		// The returned pointer may be NULL if the font has failed to load, which is OK for ImGUI's PushFont(...)
		ImFont* Get_ScaledFont(void) { return this->m_ImGuiFont; }

	private:
		SharedPtr<IBlob> m_Data;
		const bool m_IsDefault{ false };
		const bool m_IsCompressed{ false };
		const float m_SizeAtDefaultScale{ 0.f };

		ImFont* m_ImGuiFont{ nullptr };
	};

	void RegisteredFont::CreateScaledFont(float displayScale) {
		ImFontConfig fontConfig{};// has a con func...
		fontConfig.SizePixels = this->m_SizeAtDefaultScale * displayScale;

		this->m_ImGuiFont = nullptr;

		if (nullptr != this->m_Data) {
			fontConfig.FontDataOwnedByAtlas = false;
			if (this->m_IsCompressed)
				this->m_ImGuiFont = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(static_cast<const void*>(this->m_Data->Get_Data()), static_cast<Int32>(this->m_Data->Get_Size()), 0.f, &fontConfig);
			else
				this->m_ImGuiFont = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(const_cast<void*>(static_cast<const void*>(this->m_Data->Get_Data())), static_cast<Int32>(this->m_Data->Get_Size()), 0.f, &fontConfig);
		}
		else if (this->m_IsDefault)
			this->m_ImGuiFont = ImGui::GetIO().Fonts->AddFontDefault(&fontConfig);

		if (nullptr != this->m_ImGuiFont)//succ 
			ImGui::GetIO().Fonts->TexID = nullptr;
	}





	template<RHI::APITagConcept APITag>
	class UIRenderer : public IRenderPass<APITag> {
		using Imp_CommandList = typename RHI::RHITypeTraits<APITag>::Imp_CommandList;
		using Imp_FrameBuffer = typename RHI::RHITypeTraits<APITag>::Imp_FrameBuffer;

	public:
		UIRenderer(typename ManageTypeTraits<APITag>::DeviceManager* deviceManager) :
			IRenderPass<APITag>{ deviceManager },
			m_SupportExplicitDisplayScaling{ deviceManager->Get_DeviceParams().SupportExplicitDisplayScaling } {

			ImGui::CreateContext();

			this->m_DefaultFont = MakeShared<RegisteredFont>(13.f);
			this->m_Fonts.push_back(this->m_DefaultFont);
		}
		~UIRenderer(void) { ImGui::DestroyContext(); }


	public:
		bool Initialize(SharedPtr<ShaderFactory<APITag>> shaderFactory);

		// Loads a TTF font from file and registers it with the ImGui_Renderer.
	  // To use the font with ImGUI at runtime, call RegisteredFont::GetScaledFont().
		SharedPtr<RegisteredFont> CreateFontFromFile(IFileSystem& fs, const Path& fontFile, float fontSize);

		void BeginFullScreenWindow(void);

		void DrawScreenCenteredText(const char* text);

		void EndFullScreenWindow(void);

	private:


	private:
		UniquePtr<UIRHI<APITag>> m_UIRHI;

		// buffer mouse click and keypress events to make sure we don't lose events which last less than a full frame
		Array<bool, 3> MouseDown{ false,false,false };
		Array<bool, GLFW_KEY_LAST + 1> KeyDown{};//All default as false

		Vector<SharedPtr<RegisteredFont>> m_Fonts;

		SharedPtr<RegisteredFont> m_DefaultFont;

		bool m_SupportExplicitDisplayScaling;
		bool m_BeginFrameCalled{ false };


	public:
		void BackBufferResizing(void) override { if (nullptr != this->m_UIRHI) this->m_UIRHI->BackBufferResizing(); }

		void DisplayScaleChanged(float scaleX, float scaleY) override;

		void Animate(float elapsedTimeSeconds) override;

		void Render(Imp_FrameBuffer* framebuffer) override;

	public:
		virtual void BuildUI(void) = 0;

	};




	template<RHI::APITagConcept APITag>
	inline bool UIRenderer<APITag>::Initialize(SharedPtr<ShaderFactory<APITag>> shaderFactory) {
		this->m_UIRHI = MakeUnique<UIRHI<APITag>>();

		// Set up keyboard mapping.
		// ImGui will use those indices to peek into the io.KeyDown[] array
		// that we will update during the application lifetime.
		ImGuiIO& io = ImGui::GetIO();
		io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
		io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
		io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
		io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
		io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
		io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
		io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
		io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
		io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
		io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
		io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
		io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
		io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
		io.KeyMap[ImGuiKey_A] = 'A';
		io.KeyMap[ImGuiKey_C] = 'C';
		io.KeyMap[ImGuiKey_V] = 'V';
		io.KeyMap[ImGuiKey_X] = 'X';
		io.KeyMap[ImGuiKey_Y] = 'Y';
		io.KeyMap[ImGuiKey_Z] = 'Z';

		return this->m_UIRHI->Initialize(this->m_DeviceManager->Get_Device(), shaderFactory);
	}

	template<RHI::APITagConcept APITag>
	inline SharedPtr<RegisteredFont> UIRenderer<APITag>::CreateFontFromFile(IFileSystem& fs, const Path& fontFile, float fontSize) {
		if (auto fontData{ fs.ReadFile(fontFile) }; nullptr != fontData) {

			auto font{ MakeShared<RegisteredFont>(fontData, false, fontSize) };
			this->m_Fonts.push_back(font);

			return font;
		}

		return MakeShared<RegisteredFont>();
	}

	template<RHI::APITagConcept APITag>
	inline void UIRenderer<APITag>::BeginFullScreenWindow(void) {
		ImGuiIO const& io{ ImGui::GetIO() };
		ImGui::SetNextWindowPos(ImVec2{ 0.f, 0.f }, ImGuiCond_Always);
		ImGui::SetNextWindowSize(
			ImVec2{ io.DisplaySize.x / io.DisplayFramebufferScale.x,io.DisplaySize.y / io.DisplayFramebufferScale.y },
			ImGuiCond_Always
		);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
		ImGui::SetNextWindowBgAlpha(0.f);
		ImGui::Begin(" ", 0, ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);
	}

	template<RHI::APITagConcept APITag>
	inline void UIRenderer<APITag>::DrawScreenCenteredText(const char* text){
		ImGuiIO const& io{ ImGui::GetIO() };
		ImVec2 textSize{ ImGui::CalcTextSize(text) };
		ImGui::SetCursorPosX((io.DisplaySize.x / io.DisplayFramebufferScale.x - textSize.x) * 0.5f);
		ImGui::SetCursorPosY((io.DisplaySize.y / io.DisplayFramebufferScale.y - textSize.y) * 0.5f);
		ImGui::TextUnformatted(text);
	}

	template<RHI::APITagConcept APITag>
	inline void UIRenderer<APITag>::EndFullScreenWindow(void){
		ImGui::End();
		ImGui::PopStyleVar();
	}

	template<RHI::APITagConcept APITag>
	inline void UIRenderer<APITag>::DisplayScaleChanged(float scaleX, float scaleY) {
		// Apps that don't implement explicit scaling won't expect the fonts to be resized etc.
		if (!this->m_SupportExplicitDisplayScaling)
			return;

		auto& io{ ImGui::GetIO() };

		// Clear the ImGui font atlas and invalidate the font texture
		// to re-register and re-rasterize all fonts on the next frame (see Animate)
		io.Fonts->Clear();
		io.Fonts->TexID = nullptr;

		for (auto& font : this->m_Fonts)
			font->ReleaseScaledFont();

		ImGui::GetStyle() = ImGuiStyle{};
		ImGui::GetStyle().ScaleAllSizes(scaleX);
	}

	template<RHI::APITagConcept APITag>
	inline void UIRenderer<APITag>::Animate(float elapsedTimeSeconds) {
		// multiple Animate may be called before the first Render due to the m_SkipRenderOnFirstFrame extension
		// ensure each RHI->BeginFrame matches with exactly one RHI->Render
		if (nullptr == this->m_UIRHI || this->m_BeginFrameCalled)
			return;

		// Make sure that all registered fonts have corresponding ImFont objects at the current DPI scale
		const auto [scaleX, scaleY] {this->m_DeviceManager->Get_DPIScaleInfo()};
		for (auto& font : this->m_Fonts)
			if (nullptr == font->Get_ScaledFont())
				font->CreateScaledFont(this->m_SupportExplicitDisplayScaling ? scaleX : 1.f);

		// Creates the font texture if it's not yet valid
		this->m_UIRHI->UpdateFontTexture();

		const auto [w, h] { this->m_DeviceManager->Get_WindowDimensions()};

		ImGuiIO& io{ ImGui::GetIO() };
		io.DisplaySize = ImVec2{ static_cast<float>(w), static_cast<float>(h) };
		if (!this->m_SupportExplicitDisplayScaling) {
			io.DisplayFramebufferScale.x = scaleX;
			io.DisplayFramebufferScale.y = scaleY;
		}

		io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
		io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
		io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
		io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];

		io.DeltaTime = elapsedTimeSeconds;
		io.MouseDrawCursor = false;

		ImGui::NewFrame();

		this->m_BeginFrameCalled = true;
	}

	template<RHI::APITagConcept APITag>
	inline void UIRenderer<APITag>::Render(Imp_FrameBuffer* framebuffer) {
		if (nullptr == this->m_UIRHI) 
			return;

		this->BuildUI();

		ImGui::Render();
		this->m_UIRHI->Render(framebuffer);
		this->m_BeginFrameCalled = false;

		// reconcile mouse button states
		auto& io{ ImGui::GetIO() };
		for (Uint64 Index = 0; Index < this->MouseDown.size(); ++Index)
			if (io.MouseDown[Index] == true && this->MouseDown[Index] == false)
				io.MouseDown[Index] = false;

		// reconcile key states
		for (Uint64 Index = 0; Index < this->KeyDown.size(); ++Index)
			if (io.KeysDown[Index] == true && this->KeyDown[Index] == false)
				io.KeysDown[Index] = false;
	}



}