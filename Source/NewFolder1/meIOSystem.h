#pragma once

class meIOSystem :
	public REX::TSingleton<meIOSystem>
{
	static void BSPCMouseDevice__ProcessWinPCRawInput(void* a_this, void* a_input) noexcept;
	static void BSPCKeyboardDevice__ProcessWinPCRawInput(void* a_this, void* a_input) noexcept;

	uintptr_t detours[2]{};

	decltype(&BSPCMouseDevice__ProcessWinPCRawInput) BSPCMouseDevice__ProcessWinPCRawInputOrig{ nullptr };
	decltype(&BSPCKeyboardDevice__ProcessWinPCRawInput) BSPCKeyboardDevice__ProcessWinPCRawInputOrig{ nullptr };

	static uint32_t HandlerRawInputData(void* rawinput, uint32_t cmd, void* data, uint32_t* dataSize,
		uint32_t headerSize) noexcept;

	std::atomic_bool blockSignal{};

	meIOSystem(const meIOSystem&) = delete;
	meIOSystem(meIOSystem&&) = delete;
	meIOSystem& operator=(const meIOSystem&) = delete;
	meIOSystem& operator=(meIOSystem&&) = delete;
public:
	meIOSystem() = default;
	virtual ~meIOSystem() = default;

	static void InitSDM();
	static void KillSDM();

	virtual void SetBlockSignal(bool a_value) noexcept;
	virtual bool HasBlockSignal() const noexcept;
};