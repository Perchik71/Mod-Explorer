#pragma once

class meImgUI :
	public REX::TSingleton<meImgUI>
{
	bool initDX11{ false };

	static int32_t proc(REX::W32::HWND, uint32_t, uint64_t, uint64_t) noexcept;

	meImgUI(const meImgUI&) = delete;
	meImgUI(meImgUI&&) = delete;
	meImgUI& operator=(const meImgUI&) = delete;
	meImgUI& operator=(meImgUI&&) = delete;
public:
	meImgUI() = default;
	virtual ~meImgUI() = default;

	static void InitSDM() noexcept;
	static void KillSDM() noexcept;

	bool Initialize() noexcept;
	void Shutdown() noexcept;
};