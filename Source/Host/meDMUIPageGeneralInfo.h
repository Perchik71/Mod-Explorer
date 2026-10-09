#pragma once

#include "meDMUIPageBased.h"
#include "../meDataStorage.h"
#include <atomic>

class meDMUIPageGeneralInfo :
	public meDMUIPageBased
{
	std::atomic_int32_t selectedPluginId = -1;
	std::shared_ptr<dmui::Texture> textureArmor{};
	std::shared_ptr<dmui::Texture> textureBook{};
	std::shared_ptr<dmui::Texture> textureMisc{};
	std::shared_ptr<dmui::Texture> textureWeapon{};
	std::shared_ptr<dmui::Texture> textureAmmo{};
	std::shared_ptr<dmui::Texture> textureKey{};
	std::shared_ptr<dmui::Texture> textureAlchemy{};
	std::shared_ptr<dmui::Texture> textureNote{};

	void DoDrawPage() override;
	void DoRequestUpdate() override;

	meDMUIPageGeneralInfo(const meDMUIPageGeneralInfo&) = delete;
	meDMUIPageGeneralInfo(meDMUIPageGeneralInfo&&) = delete;
	meDMUIPageGeneralInfo& operator=(const meDMUIPageGeneralInfo&) = delete;
	meDMUIPageGeneralInfo& operator=(meDMUIPageGeneralInfo&&) = delete;
public:
	meDMUIPageGeneralInfo(const std::shared_ptr<dmui::Client>& a_client);
	~meDMUIPageGeneralInfo() = default;

	int32_t GetSelectedPluginId() const noexcept;
	void SetSelectedPluginId(int32_t a_idx) noexcept;
};