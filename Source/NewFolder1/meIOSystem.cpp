#include "meIOSystem.h"
#include "meHotkeySystem.h"
#include "meExplorer.h"
#include <detours/Detours.h>
#include <Windows.h>

void meIOSystem::BSPCMouseDevice__ProcessWinPCRawInput(void* a_this, void* a_input) noexcept
{
	auto system = meIOSystem::GetSingleton();
	if (!system->HasBlockSignal())
		system->BSPCMouseDevice__ProcessWinPCRawInputOrig(a_this, a_input);
}

void meIOSystem::BSPCKeyboardDevice__ProcessWinPCRawInput(void* a_this, void* a_input) noexcept
{
	auto input = reinterpret_cast<PRAWINPUT>(a_input);
	if ((input->header.dwType == RIM_TYPEKEYBOARD) && (input->data.keyboard.Message == WM_KEYUP))
	{
		meHotkeySystem::Hotkey hkOpen{ input->data.keyboard.VKey,
			(GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0,
			(GetAsyncKeyState(VK_MENU) & 0x8000) != 0,
			(GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0 };

		auto hotkey = meHotkeySystem::GetSingleton();
		if (hotkey->GetHotkeyForOpen() == hkOpen)
			meExplorer::GetSingleton()->Open();
	}

	auto system = meIOSystem::GetSingleton();
	if (!system->HasBlockSignal())
		system->BSPCKeyboardDevice__ProcessWinPCRawInputOrig(a_this, a_input);
}

void meIOSystem::InitSDM()
{
	auto system = meIOSystem::GetSingleton();
	REL::ID meIOSystem__idWndProc{ 719782, 2228990 };
	REL::Relocation meIOSystem__Target1{ meIOSystem__idWndProc, REL::Offset{ 0x37B } };
	REL::Relocation meIOSystem__Target2{ meIOSystem__idWndProc, REL::Offset{ 0x39D } };

	system->detours[0] = Detours::X64::DetourFunction(meIOSystem__Target1.address(),
			reinterpret_cast<uintptr_t>(&BSPCMouseDevice__ProcessWinPCRawInput),
			Detours::X64Option::USE_REL32_CALL);
	system->detours[1] = Detours::X64::DetourFunction(meIOSystem__Target2.address(),
			reinterpret_cast<uintptr_t>(&BSPCKeyboardDevice__ProcessWinPCRawInput),
			Detours::X64Option::USE_REL32_CALL);

	*(uintptr_t*)&system->BSPCMouseDevice__ProcessWinPCRawInputOrig = REL::ID{ 42661, 2268463 }.address();
	*(uintptr_t*)&system->BSPCKeyboardDevice__ProcessWinPCRawInputOrig = REL::ID{ 789103, 2268457 }.address();
}

void meIOSystem::KillSDM()
{
	auto system = meIOSystem::GetSingleton();

	Detours::X64::DetourRemove(system->detours[0]);
	Detours::X64::DetourRemove(system->detours[1]);
}

void meIOSystem::SetBlockSignal(bool a_value) noexcept
{
	blockSignal.store(a_value);
}

bool meIOSystem::HasBlockSignal() const noexcept
{
	return blockSignal.load();
}
