#include "meDMUIPageManager.h"

#include <algorithm>

void meDMUIPageManager::DoBeginDraw(const meDMUIPageBased& a_sender)
{
	meDMUIPageManager::GetSingleton()->updateFrame.store(true);
}

void meDMUIPageManager::DoEndDraw(const meDMUIPageBased& a_sender)
{
	meDMUIPageManager::GetSingleton()->updateFrame.store(false);
}

bool meDMUIPageManager::IsProcessingDraw() const noexcept
{
	return updateFrame.load();
}

bool meDMUIPageManager::IsProcessingUpdate() const noexcept
{
	return update.load();
}

void meDMUIPageManager::RegisterPage(const std::shared_ptr<meDMUIPageBased>& a_page) noexcept
{
	if (!a_page)
		return;

	if (std::find(pageList.begin(), pageList.end(), a_page) != pageList.end())
	{
		REX::WARN("PME::RegisterPage() attempt duplicate page");
		return;
	}

	a_page->OnBeginDraw = DoBeginDraw;
	a_page->OnEndDraw	= DoEndDraw;

	pageList.emplace_back(a_page);

	REX::INFO("PME::RegisterPage() register page");
}

void meDMUIPageManager::DrawPage(uint32_t a_idx) const noexcept
{
	if (static_cast<size_t>(a_idx) < pageList.size())
		pageList[a_idx]->DrawPage();
}

void meDMUIPageManager::SetProcessingUpdate(bool a_value) noexcept
{
	update.store(a_value);
}
