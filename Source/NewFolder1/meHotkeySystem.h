#pragma once

class meHotkeySystem :
	public REX::TSingleton<meHotkeySystem>
{
	meHotkeySystem(const meHotkeySystem&) = delete;
	meHotkeySystem(meHotkeySystem&&) = delete;
	meHotkeySystem& operator=(const meHotkeySystem&) = delete;
	meHotkeySystem& operator=(meHotkeySystem&&) = delete;
public:
	enum class Modificator : uint8_t
	{
		kCtrl = 1 << 0,
		kAlt = 1 << 1,
		kShift = 1 << 2,
	};

	struct Hotkey
	{
		uint8_t modificator{ 0 };
		uint8_t vkCode{ 0 };
		
		bool operator==(const Hotkey& a_hotkey) const noexcept;
		bool operator!=(const Hotkey& a_hotkey) const noexcept;

		constexpr Hotkey() noexcept = default;
		Hotkey(uint16_t a_vk, bool a_ctrl, bool a_alt, bool a_shift) noexcept;
	};

	meHotkeySystem() = default;
	virtual ~meHotkeySystem() = default;

	static Hotkey StringToHotkey(const std::string& str) noexcept;
	static std::string HotkeyToString(const Hotkey& hotkey) noexcept;

	static void InitSDM() noexcept;
	static void KillSDM() noexcept;

	virtual void SetHotkeyForOpen(const Hotkey& a_hotkey) noexcept;
	virtual Hotkey GetHotkeyForOpen() noexcept;
private:
	Hotkey open{};
};