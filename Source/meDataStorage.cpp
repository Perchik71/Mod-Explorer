#include "meDataStorage.h"
#include "meUtils.h"

#include <RE/RTTI.h>
#include <RE/IDs_RTTI.h>
#include <RE/T/TESDataHandler.h>
#include <RE/T/TESWeightForm.h>
#include <RE/T/TESValueForm.h>

#include <atomic>
#include <thread>
#include <string_view>

using namespace std::literals;

int meModStorage::CompareItems(const void* a, const void* b)
{
	// Cast void pointers back to integer pointers and dereference them
	auto valA = reinterpret_cast<const meItem*>(a)->formId;
	auto valB = reinterpret_cast<const meItem*>(b)->formId;

	if (valA < valB) return -1;
	if (valA > valB) return 1;
	return 0;
}

int meModStorage::GetIndexList(meItemType a_type) noexcept
{
	switch (a_type)
	{
	case meItemType::kArmorItem:
		return 0;
	case meItemType::kBookItem:
		return 1;
	case meItemType::kMiscItem:
		return 2;
	case meItemType::kWeaponItem:
		return 3;
	case meItemType::kAmmoItem:
		return 4;
	case meItemType::kKeyItem:
		return 5;
	case meItemType::kAlchemyItem:
		return 6;
	case meItemType::kNoteItem:
		return 7;
	default:
		return -1;
	}
}

bool meModStorage::IsValidForm(const RE::TESForm* a_form) noexcept
{
	if (!a_form)
		return false;

	if (!a_form->IsBoundObject() || !a_form->GetPlayable(nullptr))
		return false;

	auto name = (RE::TESFullName*)RE::RTDynamicCast((void*)a_form, 0, (void*)RE::RTTI::TESForm.address(),
		(void*)RE::RTTI::TESFullName.address(), 0);

	if (!name || !strlen(name->fullName.c_str()) || (name->fullName.c_str()[0] == '<'))
		return false;

	return true;
}

void meModStorage::AddItem(meItemType a_type, const RE::TESForm* a_form) noexcept
{
	if (!IsValidForm(a_form))
		return;

	auto id = GetIndexList(a_type);
	if (id == -1)
		return;

	RE::BSAutoLock guard(locker);

	// weird.. it skips some form
	//if (!file->IsFormInMod(a_form->formID))
	//	return;

	meItem item{};
	item.formId = a_form->formID;
	auto edid = a_form->GetFormEditorID();
	item.editorId = edid ? edid : "";
	item.form = a_form;
	item.type = a_type;

	auto pKeyword = (RE::BGSKeywordForm*)RE::RTDynamicCast((void*)a_form, 0, (void*)RE::RTTI::TESForm.address(),
		(void*)RE::RTTI::BGSKeywordForm.address(), 0);
	if (pKeyword)
		for (uint32_t j = 0; j < pKeyword->numKeywords; j++)
		{
			// FeaturedItem [KYWD:001B3FAC], quest item
			if ((pKeyword->keywords[j]->formID == 0x1B3FAC))
				item.flag.set(meItemFlag::kQuestItem);
			// VendorItemKey [KYWD:00022CE3], this a key to buy or enter a house.
			if ((a_form->formType.get() == RE::ENUM_FORM_ID::kKEYM) && (pKeyword->keywords[j]->formID == 0x22CE3))
				return;
		}

	auto name = (RE::TESFullName*)RE::RTDynamicCast((void*)a_form, 0, (void*)RE::RTTI::TESForm.address(),
		(void*)RE::RTTI::TESFullName.address(), 0);
	auto weight = (RE::TESWeightForm*)RE::RTDynamicCast((void*)a_form, 0, (void*)RE::RTTI::TESObjectMISC.address(),
		(void*)RE::RTTI::TESWeightForm.address(), 0);
	auto value = (RE::TESValueForm*)RE::RTDynamicCast((void*)a_form, 0, (void*)RE::RTTI::TESObjectMISC.address(),
		(void*)RE::RTTI::TESValueForm.address(), 0);
	if (name) item.name = name->fullName.c_str();
	if (weight) item.weight = weight->weight;
	if (value) item.price = value->value;

	switch (a_type)
	{
	case meItemType::kArmorItem:
	{
		if (a_form->formType != RE::ENUM_FORM_ID::kARMO)
			break;

		auto& data = reinterpret_cast<const RE::TESObjectARMO*>(a_form)->armorData;
		item.weight = data.weight;
		item.price = data.value;
		arrList[id].push_back(item);

		break;
	}
	case meItemType::kBookItem:
	{
		if (a_form->formType != RE::ENUM_FORM_ID::kBOOK)
			break;

		arrList[id].push_back(item);

		break;
	}
	case meItemType::kMiscItem:
	{
		if (a_form->formType != RE::ENUM_FORM_ID::kMISC)
			break;

		arrList[id].push_back(item);

		break;
	}
	case meItemType::kWeaponItem:
	{
		if (a_form->formType != RE::ENUM_FORM_ID::kWEAP)
			break;

		auto& data = reinterpret_cast<const RE::TESObjectWEAP*>(a_form)->weaponData;
		item.weight = data.weight;
		item.price = data.value;
		arrList[id].push_back(item);

		break;
	}
	case meItemType::kAmmoItem:
	{
		if (a_form->formType != RE::ENUM_FORM_ID::kAMMO)
			break;

		arrList[id].push_back(item);

		break;
	}
	case meItemType::kKeyItem:
	{
		if (a_form->formType != RE::ENUM_FORM_ID::kKEYM)
			break;

		arrList[id].push_back(item);

		break;
	}
	case meItemType::kAlchemyItem:
	{
		if (a_form->formType != RE::ENUM_FORM_ID::kALCH)
			break;

		arrList[id].push_back(item);

		break;
	}
	case meItemType::kNoteItem:
	{
		if (a_form->formType != RE::ENUM_FORM_ID::kNOTE)
			break;

		arrList[id].push_back(item);

		break;
	}
	default:
		break;
	}
}

meModStorage::meModStorage(const RE::TESFile* a_plugin) noexcept :
	file(a_plugin)
{
	if (!a_plugin)
		return;

	filename = a_plugin->filename;
	author = a_plugin->createdBy.empty() ? "" : a_plugin->createdBy.c_str();
	summary = a_plugin->summary.empty() ? "" : a_plugin->summary.c_str();
	
	meUtils::Trim(author);
	meUtils::Trim(summary);
}

void meModStorage::Clear() noexcept
{
	RE::BSAutoLock guard(locker);

	for (auto& list : arrList)
		list.clear();

	filename.clear();
	author.clear();
	summary.clear();
}

uint32_t meModStorage::GetItemCount(meItemType a_type) const noexcept
{
	RE::BSAutoLock guard(const_cast<meModStorage*>(this)->locker);

	if (a_type == meItemType::kMax)
	{
		uint32_t num = 0;
		for (auto& list : arrList)
			num += static_cast<uint32_t>(list.size());
		return num;
	}
	else
	{
		auto id = GetIndexList(a_type);
		if (id == -1)
			return 0;

		return static_cast<uint32_t>((arrList[id]).size());
	}
}

void meModStorage::GetAllItems(meItemType a_type, meItemList& a_list, bool a_sorted) const noexcept
{
	RE::BSAutoLock guard(const_cast<meModStorage*>(this)->locker);

	a_list.clear();

	if (a_type == meItemType::kMax)
	{
		for (auto& list : arrList)
			a_list.append_range(list);
	}
	else
	{
		auto id = GetIndexList(a_type);
		if (id == -1)
			return;

		a_list.assign_range(arrList[id]);
	}

	if (a_sorted)
		std::qsort(a_list.data(), a_list.size(), sizeof(meItem), std::addressof(CompareItems));
}

void meModStorage::Sort() noexcept
{
	for (auto& arr : arrList)
		std::qsort(arr.data(), arr.size(), sizeof(meItem), std::addressof(CompareItems));
}

std::optional<meItem> meModStorage::GetItem(meItemType a_type, uint32_t a_formId) const noexcept
{
	RE::BSAutoLock guard(const_cast<meModStorage*>(this)->locker);

	if (a_type == meItemType::kMax)
	{
		for (auto& list : arrList)
		{
			auto it = std::lower_bound(list.begin(), list.end(), a_formId, [](const meItem& a_item, uint32_t a_value) {
				return a_item.formId < a_value;
				});
			if (it != list.end())
				return *it;
		}
	}
	else
	{
		auto id = GetIndexList(a_type);
		if (id == -1)
			return std::nullopt;

		auto& list = arrList[id];
		auto it = std::lower_bound(list.begin(), list.end(), a_formId, [](const meItem& a_item, uint32_t a_value) {
			return a_item.formId < a_value;
			});
		if (it != list.end())
			return *it;
	}

	return std::nullopt;
}

std::optional<uint32_t> meModStorage::GetIndex() const noexcept
{
	if (!file) return std::nullopt;
	return file->IsLight() ? (file->smallFileCompileIndex + 0xFE000) : file->compileIndex;
}

void meDataStorage::AddItem(meItemType a_type, const RE::TESForm* a_form) noexcept
{
	if (meModStorage::IsValidForm(a_form))
	{
		auto idx = a_form->formID >> 24;
		if (idx == 0xFE)
			idx = a_form->formID >> 12;

		for (auto& mod : mods)
		{
			if (mod.second->GetIndex() == idx)
			{
				mod.second->AddItem(a_type, a_form);
				break;
			}
		}
	}
}

void meDataStorage::AddRange(meItemType a_type, const RE::BSTArray<RE::TESForm*>& a_arr) noexcept
{
	auto size = a_arr.size();
	if (size < 100)	// Single-thread mode
	{
		for (auto form : a_arr)
			AddItem(a_type, form);
	}
	else			// Multithreads mode
	{
		std::atomic_bool a[4]{};

		auto count = size / 4;
		auto worker = [&](uint32_t begin, uint32_t end, uint32_t a_idx) {

			for (uint32_t i = begin; i < end; i++)
				AddItem(a_type, a_arr[i]);

			a[a_idx].store(true);
			};

		std::thread t1(worker, 0u, count, 0u);
		std::thread t2(worker, count, count << 1, 1u);
		std::thread t3(worker, count << 1, count * 3, 2u);
		std::thread t4(worker, count * 3, size, 3u);

		t1.detach();
		t2.detach();
		t3.detach();
		t4.detach();

		while (!a[0].load() || !a[1].load() || !a[2].load() || !a[3].load())
			std::this_thread::sleep_for(5ms);
	}
}

uint32_t meDataStorage::GetModCount() const noexcept
{
	return static_cast<uint32_t>(sortedMods.size());
}

const std::shared_ptr<meModStorage> meDataStorage::GetMod(const std::string& a_filename) const noexcept
{
	RE::BSAutoLock guard(const_cast<meDataStorage*>(this)->locker);

	std::string fname = a_filename;
	auto it = mods.find(_strlwr(fname.data()));
	return (it == mods.end()) ? it->second : nullptr;
}

const std::shared_ptr<meModStorage> meDataStorage::GetModByIndex(uint32_t a_idx) const noexcept
{
	if (a_idx >= GetModCount())
		return nullptr;

	RE::BSAutoLock guard(const_cast<meDataStorage*>(this)->locker);
	return sortedMods[a_idx];
}

uint32_t meDataStorage::GetItemCount(meItemType a_type) const noexcept
{
	RE::BSAutoLock guard(const_cast<meDataStorage*>(this)->locker);

	uint32_t num = 0;
	for (auto& it : mods)
		num += it.second->GetItemCount(a_type);
	return num;
}

void meDataStorage::GetAllItems(meItemType a_type, meItemList& a_list, bool a_sorted) const noexcept
{
	RE::BSAutoLock guard(const_cast<meDataStorage*>(this)->locker);

	a_list.clear();

	for (auto& it : mods)
		it.second->GetAllItems(a_type, a_list, false);

	if (a_sorted)
		std::qsort(a_list.data(), a_list.size(), sizeof(meItem), std::addressof(meModStorage::CompareItems));
}

void meDataStorage::InitSDM() noexcept
{
	RE::BSAutoLock guard(locker);

	mods.clear();
	lmodNum = dmodNum = 0;

	auto dataHandler = RE::TESDataHandler::GetSingleton();
	if (!dataHandler || dataHandler->loadingFiles) return;

	for (auto file : dataHandler->files)
		if (file && file->IsActive())
			sortedMods.emplace_back(std::make_shared<meModStorage>(file));

	// Sort by order
	std::sort(sortedMods.begin(), sortedMods.end(), 
		[](const std::shared_ptr<meModStorage>& a1, const std::shared_ptr<meModStorage>& a2) {
		uint32_t i1 = a1->GetIndex().value();
		uint32_t i2 = a2->GetIndex().value();
		return i1 < i2;
		});

	for (auto& mod : sortedMods)
		if (mod && mod->file)
		{
			std::string fname = mod->file->GetFilename().data();
			mods.emplace(_strlwr(fname.data()), mod);

			if (mod->file->IsLight())
				lmodNum++;
			else
				dmodNum++;
		}

	AddRange(meItemType::kArmorItem,	dataHandler->formArrays[std::to_underlying(meItemType::kArmorItem)]);
	AddRange(meItemType::kBookItem,		dataHandler->formArrays[std::to_underlying(meItemType::kBookItem)]);
	AddRange(meItemType::kMiscItem,		dataHandler->formArrays[std::to_underlying(meItemType::kMiscItem)]);
	AddRange(meItemType::kWeaponItem,	dataHandler->formArrays[std::to_underlying(meItemType::kWeaponItem)]);
	AddRange(meItemType::kAmmoItem,		dataHandler->formArrays[std::to_underlying(meItemType::kAmmoItem)]);
	AddRange(meItemType::kKeyItem,		dataHandler->formArrays[std::to_underlying(meItemType::kKeyItem)]);
	AddRange(meItemType::kAlchemyItem,	dataHandler->formArrays[std::to_underlying(meItemType::kAlchemyItem)]);
	AddRange(meItemType::kNoteItem,		dataHandler->formArrays[std::to_underlying(meItemType::kNoteItem)]);

	REX::DEBUG("ArmorItem: {}",		GetItemCount(meItemType::kArmorItem));
	REX::DEBUG("BookItem: {}",		GetItemCount(meItemType::kBookItem));
	REX::DEBUG("MiscItem: {}",		GetItemCount(meItemType::kMiscItem));
	REX::DEBUG("WeaponItem: {}",	GetItemCount(meItemType::kWeaponItem));
	REX::DEBUG("AmmoItem: {}",		GetItemCount(meItemType::kAmmoItem));
	REX::DEBUG("KeyItem: {}",		GetItemCount(meItemType::kKeyItem));
	REX::DEBUG("AlchemyItem: {}",	GetItemCount(meItemType::kAlchemyItem));
	REX::DEBUG("NoteItem: {}",		GetItemCount(meItemType::kNoteItem));

	for (auto& it : mods)
		it.second->Sort();
}

void meDataStorage::KillSDM() noexcept
{
	RE::BSAutoLock guard(locker);

	mods.clear();
}
