#pragma once

#include "meDMUIPageBased.h"

#include <atomic>
#include <array>

class meDMUIPageBasket :
	public meDMUIPageBased
{
	std::array<bool, 2> openedPopups{};
	std::atomic_int32_t selectedItemId = -1;
	std::shared_ptr<dmui::Texture> textureStar{};
	std::shared_ptr<dmui::Texture> textureBasket{};

	constexpr static auto failed_id1 = "ff##dearmodding.modexplorer.page.basket.body.popup.takefree.failed";
	constexpr static auto failed_id2 = "sss##dearmodding.modexplorer.page.basket.body.popup.buy.failed";

	void DoDrawPage() override;
	void DoRequestUpdate() override;
	void DoDrawPopupFailed_1() noexcept;
	void DoDrawPopupFailed_2() noexcept;

	meDMUIPageBasket(const meDMUIPageBasket&) = delete;
	meDMUIPageBasket(meDMUIPageBasket&&) = delete;
	meDMUIPageBasket& operator=(const meDMUIPageBasket&) = delete;
	meDMUIPageBasket& operator=(meDMUIPageBasket&&) = delete;
public:
	meDMUIPageBasket(const std::shared_ptr<dmui::Client>& a_client);
	~meDMUIPageBasket() = default;

	int32_t GetSelectedItemId() const noexcept;
	void SetSelectedItemId(int32_t a_idx) noexcept;
};