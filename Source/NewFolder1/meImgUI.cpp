#include "meImgUI.h"
#include "meExplorer.h"
#include "meUtils.h"

#include <RE/B/BSGraphics.h>

#include <Windows.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_dx11.h>
#include <backends/imgui_impl_win32.h>

extern IMGUI_IMPL_API LRESULT WINAPI ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

uintptr_t g_detourWndProc = 0;

int32_t meImgUI::proc(REX::W32::HWND hWnd, uint32_t uMsg, uint64_t wParam, uint64_t lParam) noexcept
{
	static auto TerminateGame = []() {
		auto bShift = (GetAsyncKeyState(VK_SHIFT) & 0x8000) == 0x8000;
		auto bMenu = (GetAsyncKeyState(VK_MENU) & 0x8000) == 0x8000;
		auto bControl = (GetAsyncKeyState(VK_CONTROL) & 0x8000) == 0x8000;

		// ALT + F4 (send system)
		if (!bShift && !bControl && bMenu)
		{
			ClipCursor(nullptr);
			TerminateProcess(GetCurrentProcess(), 0);
		}
		};

	auto explorer = meExplorer::GetSingleton();
	if (explorer && explorer->IsOpen())
	{
		if (ImGui_ImplWin32_WndProcHandler(reinterpret_cast<HWND>(hWnd), uMsg, wParam, lParam))
			return S_FALSE;

		auto& io = ImGui::GetIO();

		switch (uMsg)
		{
		case WM_KEYUP:
		case WM_KEYDOWN:
		case WM_SYSKEYUP:
		case WM_SYSKEYDOWN:
			if (io.WantCaptureKeyboard && (wParam != VK_F4))
				return S_OK;
			break;
		case WM_MOUSEMOVE:
		case WM_MOUSELEAVE:
		case WM_MOUSEHOVER:
		case WM_MOUSEWHEEL:
		case WM_MOUSEHWHEEL:
		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
		case WM_MBUTTONDBLCLK:
		case WM_RBUTTONDBLCLK:
		case WM_NCLBUTTONUP:
		case WM_NCMBUTTONUP:
		case WM_NCRBUTTONUP:
		case WM_NCLBUTTONDOWN:
		case WM_NCMBUTTONDOWN:
		case WM_NCRBUTTONDOWN:
			if (io.WantCaptureMouse)
				return S_OK;
			break;
		case WM_ACTIVATE:
			if (wParam != WA_INACTIVE)
				meEventManager::GetSingleton()->PushEvent(new (std::nothrow) meInactiveGameEvent);
			else
				meEventManager::GetSingleton()->PushEvent(new (std::nothrow) meActiveGameEvent);
			break;
		default:
			break;
		}
	}
	else
	{
		//switch (uMsg)
		//{
		//case WM_KEYUP:
		//case WM_KEYDOWN:
		//case WM_SYSKEYUP:
		//case WM_SYSKEYDOWN:
		//	//if (wParam == VK_F4)
		//	//	TerminateGame();
		//	break;
		//}
	}

	return 0;
}

void meImgUI::InitSDM() noexcept
{
	if (g_detourWndProc)
		return;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	auto& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;		// Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;		// Enable Gamepad Controls

	io.Fonts->AddFontDefault();
	
	//auto target = REL::ID{ 719782, 2228990 }.address();
	//meUtils::DetourClassJump(target, &proc, &g_detourWndProc);
}

void meImgUI::KillSDM() noexcept
{
	if (!g_detourWndProc)
		return;

	meImgUI::GetSingleton()->Shutdown();
	ImGui::DestroyContext();

	meUtils::DetourRemove(g_detourWndProc);
}

bool meImgUI::Initialize() noexcept
{
	if (initDX11) return true;

	auto rendererData = reinterpret_cast<RE::BSGraphics::RendererData*>(REL::ID{ 235166, 2704527 }.address());
	if (!rendererData) return false;

	// Make process DPI aware and obtain main monitor scale
	ImGui_ImplWin32_EnableDpiAwareness();
	ImGui_ImplWin32_Init(rendererData->renderWindow[0].hwnd);
	ImGui_ImplDX11_Init((ID3D11Device*)rendererData->device, (ID3D11DeviceContext*)rendererData->context);

	return initDX11 = true;
}

void meImgUI::Shutdown() noexcept
{
	if (initDX11)
	{
		initDX11 = false;

		// Cleanup
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
	}
}