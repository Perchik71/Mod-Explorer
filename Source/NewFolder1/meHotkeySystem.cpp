#include "meHotkeySystem.h"
#include "meUtils.h"

#include <Windows.h>

static REX::TOML::Str<> sHotkeyOpen{ "Hotkey"sv, "sOpen"sv, "F10" };

meHotkeySystem::Hotkey meHotkeySystem::StringToHotkey(const std::string& str) noexcept
{
	Hotkey hotkey{};
	std::string keybind = str;

	std::erase_if(keybind, [](auto& c) { return (c == '\"') || (c == ' '); });

	if (keybind.empty())
		return hotkey;

	std::transform(keybind.begin(), keybind.end(), keybind.begin(), toupper);

	char* context = nullptr;
	const char* t = strtok_s(keybind.data(), "+", &context);

	do
	{
		auto len = strlen(t);

		if (!strcmp(t, "CTRL"))
			hotkey.modificator |= std::to_underlying(Modificator::kCtrl);
		else if (!strcmp(t, "SHIFT"))
			hotkey.modificator |= std::to_underlying(Modificator::kShift);
		else if (!strcmp(t, "ALT"))
			hotkey.modificator |= std::to_underlying(Modificator::kAlt);
		else if (!strcmp(t, "ESC"))
			hotkey.vkCode = VK_ESCAPE;
		else if (!strcmp(t, "SPACE"))
			hotkey.vkCode = VK_SPACE;
		else if (!strcmp(t, "ENTER"))
			hotkey.vkCode = VK_RETURN;
		else if (!strcmp(t, "DEL"))
			hotkey.vkCode = VK_DELETE;
		else if (!strcmp(t, "TAB"))
			hotkey.vkCode = VK_TAB;
		else if (len > 1 && t[0] == 'F')
		{
			// Parse function keys F1 to F12
			auto index = atoi(&t[1]);
			if ((index >= 1) && (index <= 12))
				hotkey.vkCode = VK_F1 + index - 1;
			else
				return {};
		}
		else if (len > 6 && !strncmp(t, "NUMPAD", 6))
		{
			// Parse function keys NUMPAD0 to NUMPAD9
			auto index = atoi(&t[6]);
			if ((index >= 0) && (index <= 9))
				hotkey.vkCode = VK_NUMPAD0 + index;
			else
				return {};
		}
		else if (len == 1)
			// This should be translated with VkKeyScan but virtual keys make things difficult...
			hotkey.vkCode = t[0];
		else
			return {};

	} while (t = strtok_s(nullptr, "+", &context));

	return hotkey;
}

std::string meHotkeySystem::HotkeyToString(const Hotkey& hotkey) noexcept
{
	std::string keybind{};

	if (hotkey.modificator & std::to_underlying(Modificator::kCtrl))
		keybind.append("CTRL");
	if (hotkey.modificator & std::to_underlying(Modificator::kAlt))
	{
		if (!keybind.empty()) keybind.append("+");
		keybind.append("ALT");
	}
	if (hotkey.modificator & std::to_underlying(Modificator::kShift))
	{
		if (!keybind.empty()) keybind.append("+");
		keybind.append("SHIFT");
	}

	if (!keybind.empty()) keybind.append("+");

	switch (hotkey.vkCode)
	{
	case VK_ESCAPE:
		keybind.append("ESC");
		break;
	case VK_SPACE:
		keybind.append("SPACE");
		break;
	case VK_RETURN:
		keybind.append("ENTER");
		break;
	case VK_DELETE:
		keybind.append("DEL");
		break;
	case VK_TAB:
		keybind.append("TAB");
		break;
	case VK_F1:
	case VK_F2:
	case VK_F3:
	case VK_F4:
	case VK_F5:
	case VK_F6:
	case VK_F7:
	case VK_F8:
	case VK_F9:
	case VK_F10:
	case VK_F11:
	case VK_F12:
		keybind.append("F") += std::to_string(hotkey.vkCode - VK_F1 + 1);
		break;
	case VK_NUMPAD0:
	case VK_NUMPAD1:
	case VK_NUMPAD2:
	case VK_NUMPAD3:
	case VK_NUMPAD4:
	case VK_NUMPAD5:
	case VK_NUMPAD6:
	case VK_NUMPAD7:
	case VK_NUMPAD8:
	case VK_NUMPAD9:
		keybind.append("NUMPAD") += std::to_string(hotkey.vkCode - VK_NUMPAD0);
		break;
	default:
		keybind += (char)hotkey.vkCode;
		break;
	}

	return keybind;
}

void meHotkeySystem::InitSDM() noexcept
{
	meHotkeySystem::GetSingleton()->SetHotkeyForOpen(meHotkeySystem::StringToHotkey(sHotkeyOpen.GetValue()));
}

void meHotkeySystem::KillSDM() noexcept
{
	// nope
}

void meHotkeySystem::SetHotkeyForOpen(const Hotkey& a_hotkey) noexcept
{
	open = a_hotkey;
}

meHotkeySystem::Hotkey meHotkeySystem::GetHotkeyForOpen() noexcept
{
	return open;
}

bool meHotkeySystem::Hotkey::operator==(const Hotkey& a_hotkey) const noexcept
{
	return (modificator == a_hotkey.modificator) && (vkCode == a_hotkey.vkCode);
}

bool meHotkeySystem::Hotkey::operator!=(const Hotkey& a_hotkey) const noexcept
{
	return !(*this == a_hotkey);
}

meHotkeySystem::Hotkey::Hotkey(uint16_t a_vk, bool a_ctrl, bool a_alt, bool a_shift) noexcept :
	vkCode(static_cast<uint8_t>(a_vk))
{
	modificator = a_ctrl ? std::to_underlying(Modificator::kCtrl) : 0;
	if (a_alt) modificator |= std::to_underlying(Modificator::kAlt);
	if (a_shift) modificator |= std::to_underlying(Modificator::kShift);
}