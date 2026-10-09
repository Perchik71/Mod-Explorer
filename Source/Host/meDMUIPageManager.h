#pragma once

#include <vector>
#include <memory>
#include "meDMUIPageBased.h"

#include <REX/REX.h>

class meDMUIPageManager :
	public REX::TSingleton<meDMUIPageManager>
{
	std::atomic_bool updateFrame = false;
	std::atomic_bool update = false;
	std::vector<std::shared_ptr<meDMUIPageBased>> pageList{};

	meDMUIPageManager(const meDMUIPageManager&) = delete;
	meDMUIPageManager(meDMUIPageManager&&) = delete;
	meDMUIPageManager& operator=(const meDMUIPageManager&) = delete;
	meDMUIPageManager& operator=(meDMUIPageManager&&) = delete;

	static void DoBeginDraw(const meDMUIPageBased& a_sender);
	static void DoEndDraw(const meDMUIPageBased& a_sender);
public:
	meDMUIPageManager() = default;
	~meDMUIPageManager() = default;

	bool IsProcessingDraw() const noexcept;
	bool IsProcessingUpdate() const noexcept;

	void RegisterPage(const std::shared_ptr<meDMUIPageBased>& a_page) noexcept;
	void DrawPage(uint32_t a_idx) const noexcept;

	void SetProcessingUpdate(bool a_value) noexcept;
};