#pragma once

#include <REX/REX.h>

#include <RE/B/BSSpinLock.h>
#include <RE/B/BGSKeywordForm.h>
#include <RE/T/TESFile.h>
#include <RE/T/TESFullName.h>
#include <RE/T/TESObjectWEAP.h>
#include <RE/T/TESObjectARMO.h>

#include <vector>
#include <array>
#include <unordered_map>
#include <algorithm>
#include <optional>
#include <memory>

enum class meItemType : uint32_t
{
	kArmorItem		= RE::ENUM_FORM_ID::kARMO,
	kBookItem		= RE::ENUM_FORM_ID::kBOOK,
	kMiscItem		= RE::ENUM_FORM_ID::kMISC,
	kWeaponItem		= RE::ENUM_FORM_ID::kWEAP,
	kAmmoItem		= RE::ENUM_FORM_ID::kAMMO,
	kKeyItem		= RE::ENUM_FORM_ID::kKEYM,
	kAlchemyItem	= RE::ENUM_FORM_ID::kALCH,
	kNoteItem		= RE::ENUM_FORM_ID::kNOTE,
	kMax			= 8,
	kFirst			= kArmorItem,
	kLast			= kNoteItem,
};

constexpr std::array<meItemType, std::to_underlying(meItemType::kMax)> meItemTypes =
{
	meItemType::kArmorItem,
	meItemType::kBookItem,
	meItemType::kMiscItem,
	meItemType::kWeaponItem,
	meItemType::kAmmoItem,
	meItemType::kKeyItem,
	meItemType::kAlchemyItem,
	meItemType::kNoteItem,
};

enum class meItemFlag : uint32_t
{
	kQuestItem		= 0 << 1,
};

struct meItem
{
	uint32_t formId;
	std::string editorId;
	std::string name;
	meItemType type;
	float weight;
	int32_t price;
	REX::TEnumSet<meItemFlag> flag;
	const RE::TESForm* form;

	inline bool HasQuestItem() const noexcept { return flag.any(meItemFlag::kQuestItem); }
};

class meDataStorage;

using meItemList = std::vector<meItem>;
class meModStorage
{
	std::string filename{};
	std::string author{};
	std::string summary{};
	RE::BSSpinLock locker{};
	const RE::TESFile* file{ nullptr };
	std::array<meItemList, std::to_underlying(meItemType::kMax)> arrList{};

	static int CompareItems(const void* a, const void* b);
	static int GetIndexList(meItemType a_type) noexcept;
	static bool IsValidForm(const RE::TESForm* a_form) noexcept;

	meModStorage() = delete;
	meModStorage(const meModStorage&) = delete;
	meModStorage(meModStorage&&) = delete;
	meModStorage& operator=(const meModStorage&) = delete;
	meModStorage& operator=(meModStorage&&) = delete;

	void AddItem(meItemType a_type, const RE::TESForm* a_form) noexcept;
public:
	friend class meDataStorage;

	meModStorage(const RE::TESFile* a_plugin) noexcept;

	void Clear() noexcept;

	inline std::string GetFileName() const noexcept { return filename; }
	inline std::string GetAuthor() const noexcept { return author; }
	inline std::string GetSummary() const noexcept { return summary; }
	inline const RE::TESFile* GetFile() const noexcept { return file; }

	uint32_t GetItemCount(meItemType a_type) const noexcept;
	void GetAllItems(meItemType a_type, meItemList& a_list, bool a_sorted = true) const noexcept;
	void Sort() noexcept;

	std::optional<meItem> GetItem(meItemType a_type, uint32_t a_formId) const noexcept;
	std::optional<uint32_t> GetIndex() const noexcept;
};

using meModList = std::unordered_map<std::string, std::shared_ptr<meModStorage>>;
using meModSortedList = std::vector<std::shared_ptr<meModStorage>>;
class meDataStorage :
	public REX::TSingleton<meDataStorage>
{
	meModList mods{};
	meModSortedList sortedMods{};
	uint32_t dmodNum{ 0 }, lmodNum{ 0 };
	RE::BSSpinLock locker{};

	void AddItem(meItemType a_type, const RE::TESForm* a_form) noexcept;
	void AddRange(meItemType a_type, const RE::BSTArray<RE::TESForm*>& a_arr) noexcept;
public:
	meDataStorage() = default;

	uint32_t GetModCount() const noexcept;
	const std::shared_ptr<meModStorage> GetMod(const std::string& a_filename) const noexcept;
	const std::shared_ptr<meModStorage> GetModByIndex(uint32_t a_idx) const noexcept;
	uint32_t GetItemCount(meItemType a_type) const noexcept;
	void GetAllItems(meItemType a_type, meItemList& a_list, bool a_sorted = true) const noexcept;

	inline uint32_t GetDefaultModCount() const noexcept { return dmodNum; }
	inline uint32_t GetLightModCount() const noexcept { return lmodNum; }

	inline meModSortedList::iterator begin() noexcept { return sortedMods.begin(); }
	inline meModSortedList::iterator end() noexcept { return sortedMods.end(); }
	inline meModSortedList::const_iterator cbegin() noexcept { return sortedMods.cbegin(); }
	inline meModSortedList::const_iterator cend() noexcept { return sortedMods.cend(); }

	void Lock() const noexcept { const_cast<meDataStorage*>(this)->locker.lock(); }
	void Unlock() const noexcept { const_cast<meDataStorage*>(this)->locker.unlock(); }

	void InitSDM() noexcept;
	void KillSDM() noexcept;
};