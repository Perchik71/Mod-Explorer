#pragma once

#include "meDMUIPageBased.h"
#include "../meDataStorage.h"

#include <atomic>

class meDMUIPageExplorer :
	public meDMUIPageBased
{
public:
	enum class TRequestAsync : uint32_t
	{
		kNone = 0,
		kUpdateItems,
		kSearchPlugins,
		kSearchItems
	};
private:
	std::atomic_int8_t selectedShopTypeId = -1;
	std::atomic_int32_t selectedShopItemId = -1;
	std::atomic_int32_t selectedPluginId = -1;
	std::atomic<TRequestAsync> currentEvent{ TRequestAsync::kNone };
	meModSortedList pluginSearchShopList{};
	meItemList itemShopList;
	std::array<char, 128> textExplorerSearchPlugin{};
	std::array<char, 128> textExplorerSearchPluginDone{};
	std::array<char, 128> textExplorerSearchItem{};
	std::array<char, 128> textExplorerSearchItemDone{};
	std::shared_ptr<dmui::Texture> textureStar{};

	void DoDrawPage() override;
	void DoRequestUpdate() override;

	void DoRequestUpdateItems();
	void DoRequestSearchPlugins();
	void DoRequestSearchItems();

	meDMUIPageExplorer(const meDMUIPageExplorer&) = delete;
	meDMUIPageExplorer(meDMUIPageExplorer&&) = delete;
	meDMUIPageExplorer& operator=(const meDMUIPageExplorer&) = delete;
	meDMUIPageExplorer& operator=(meDMUIPageExplorer&&) = delete;
public:
	meDMUIPageExplorer(const std::shared_ptr<dmui::Client>& a_client);
	~meDMUIPageExplorer() = default;

	int32_t GetSelectedPluginId() const noexcept;
	void SetSelectedPluginId(int32_t a_idx) noexcept;
	int8_t GetSelectedShopTypeId() const noexcept;
	void SetSelectedShopTypeId(int8_t a_idx) noexcept;
	meItemList& GetItemShopList() noexcept;
	meModSortedList& GetPluginSearchShopList() noexcept;
	const char* GetTextSearchPlugin() const noexcept;
	const char* GetTextSearchItem() const noexcept;

	void RequestAsyncUpdateItem(TRequestAsync a_requestEvent = TRequestAsync::kUpdateItems);
	[[nodiscard]] bool IsProcessingUpdateItems() const noexcept;
	[[nodiscard]] bool IsProcessingSearchPlugins() const noexcept;
	[[nodiscard]] bool IsProcessingSearchItems() const noexcept;
	void ResetState();
};