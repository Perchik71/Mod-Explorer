#pragma once

#include <RE/T/TESForm.h>

class meItem
{
	RE::TESForm* form{};
	std::string fullname{};
	std::string editorId{};
	//uint32_t hash
public:
	meItem() = default;
	~meItem() = default;
};

using meItemList = std::unordered_map<std::string, meItem>;

class meStorageItems
{
	enum class State
	{
		kInit = 1 << 0,
		kShutdown = 1 << 1,
		kUpdate = 1 << 2,
	};

	std::array<meItemList, 8> arrList{};

	meStorageItems(const meStorageItems&) = delete;
	meStorageItems(meStorageItems&&) = delete;
	meStorageItems& operator=(const meStorageItems&) = delete;
	meStorageItems& operator=(meStorageItems&&) = delete;
public:
	meStorageItems() = default;
	virtual ~meStorageItems() = default;

	static void InitSDM() noexcept;
	static void KillSDM() noexcept;
};