#pragma once

#include "meDMUIPageBased.h"

#include <atomic>

class meDMUIPageBasket :
	public meDMUIPageBased
{
	std::atomic_int32_t selectedItemId = -1;
	std::shared_ptr<dmui::Texture> textureStar{};
	std::shared_ptr<dmui::Texture> textureBasket{};

	void DoDrawPage() override;
	void DoRequestUpdate() override;

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