#pragma once

#include <F4SE/F4SE.h>

class mePlugin :
	public REX::TSingleton<mePlugin>
{
	enum class State : uint32_t
	{
		kPreInit = 1 << 0,
		kInit = 1 << 1,
		kShutdown = 1 << 2,
		kRendererInit = 1 << 3,
		kRendererFailed = 1 << 4,
	};

	REX::W32::HMODULE handleCurrentDll{ nullptr };
	REX::TEnumSet<State> state{};

	bool shutdown{ false };

	void RegisterWidgetInDMUIPlatform() noexcept;
	static void PostInitImportantSystem() noexcept;
	static void PostUpdateData() noexcept;
	static void F4SEMessageListener(F4SE::MessagingInterface::Message* a_msg) noexcept;

	mePlugin(mePlugin&&) = delete;
	mePlugin(const mePlugin&) = delete;
	mePlugin& operator=(mePlugin&&) = delete;
	mePlugin& operator=(const mePlugin&) = delete;
public:
	mePlugin() = default;
	virtual ~mePlugin() = default;

	void SetHandleCurrentDll(REX::W32::HMODULE a_handleCurrentDll) noexcept;
	[[nodiscard]] REX::W32::HMODULE GetHandleCurrentDll() const noexcept;

	[[nodiscard]] bool Init(const F4SE::LoadInterface* a_f4se) noexcept;
	[[nodiscard]] bool PreInit(const F4SE::PreLoadInterface* a_preloadf4se) noexcept;
	void Kill() noexcept;
};