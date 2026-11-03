#include "meBasket.h"
#include "..\Resources\resource.h"

#include <RE/RTTI.h>
#include <RE/IDs_RTTI.h>
#include <RE/T/TESDataHandler.h>
#include <RE/T/TESWeightForm.h>
#include <RE/T/TESValueForm.h>

#include <RE/A/ActorValue.h>
#include <RE/A/ActorValueInfo.h>
#include <RE/A/ActorValueStorage.h>
#include <RE/B/BGSKeywordForm.h>
#include <RE/B/BGSInventoryItem.h>
#include <RE/B/BGSInventoryList.h>
#include <RE/P/PlayerCharacter.h>
#include <RE/T/TESBoundObject.h>
#include <RE/S/Script.h>
#include <RE/C/COMPILER_NAME.h>
#include <RE/M/MemoryManager.h>

namespace Script
{
	// Получение предметов через инвентарь в clib работает некорректно
	// Делаем псевдоскрипт

	class Invoker :
		public REX::TSingleton<Invoker>
	{
		RE::Script* script{ nullptr };
	public:
		Invoker()
		{
			script = reinterpret_cast<RE::Script*>(RE::calloc(sizeof(RE::Script) + 0x100, 1));
			if (script)
			{
				script->formFlags = 8;
				script->formID = std::numeric_limits<std::uint32_t>::max();
				script->formType = RE::ENUM_FORM_ID::kSCPT;
			}
			else
				REX::CRITICAL(L"Invoker::script no init");
		}

		~Invoker()
		{
			if (script)
			{
				RE::free(script);
				script = nullptr;
			}
		}

		bool Invoke(const std::string_view& a_text)
		{
			if (!script || a_text.empty() || !a_text.length())
				return false;

			__try
			{
				// Восстановить адрес vtable Script так как сбрасывается на BaseFormComponent
				(*(std::uintptr_t*)script) = RE::Script::VTABLE[0].address();

				script->SetText(a_text);
				script->CompileAndRun((RE::ScriptCompiler*)"bDecal", RE::COMPILER_NAME::kSystemWindow, nullptr);

				return true;
			}
			__except (1)
			{
				return false;
			}
		}

		bool InvokeFmt(const std::string_view& a_format, ...)
		{
			if (a_format.empty())
				return false;

			bool res = false;

			va_list ap;
			va_start(ap, &a_format);

			auto len = vsnprintf(nullptr, 0, a_format.data(), ap);
			if (len > 0)
			{
				auto buffer = std::make_unique<char[]>(static_cast<std::size_t>(len) + 1);
				if (buffer)
				{
					vsnprintf(buffer.get(), len, a_format.data(), ap);
					res = Invoke(buffer.get());
				}
			}

			va_end(ap);

			return res;
		}
	};
}

class ScopeCriticalLock
{
	REX::W32::CRITICAL_SECTION* critical{ nullptr };

	ScopeCriticalLock(const ScopeCriticalLock&) = delete;
	ScopeCriticalLock(ScopeCriticalLock&&) = delete;
	ScopeCriticalLock& operator=(const ScopeCriticalLock&) = delete;
	ScopeCriticalLock& operator=(ScopeCriticalLock&&) = delete;
public:
	ScopeCriticalLock(const REX::W32::CRITICAL_SECTION& a_lock) noexcept : critical(std::addressof(const_cast<REX::W32::CRITICAL_SECTION&>(a_lock)))
	{
		if (critical)
			REX::W32::EnterCriticalSection(critical);
	};
	~ScopeCriticalLock() noexcept
	{
		if (critical)
			REX::W32::LeaveCriticalSection(critical);
	}
};

meBasket::meBasket()
{
	REX::W32::InitializeCriticalSection(&locker);

	(void)takeFX.LoadFromResource("WAVE", IDB_WAVE_TAKE);
	(void)buyFX.LoadFromResource("WAVE", IDB_WAVE_BUY);
}

meBasket::~meBasket()
{
	REX::W32::DeleteCriticalSection(&locker);
}

void meBasket::Lock() const noexcept
{
	REX::W32::EnterCriticalSection(&const_cast<meBasket*>(this)->locker);
}

void meBasket::Unlock() const noexcept
{
	REX::W32::LeaveCriticalSection(&const_cast<meBasket*>(this)->locker);
}

void meBasket::Append(const meItem& a_item, uint32_t a_count) noexcept
{
	if (a_count < 1)
		return;

	if (IsSellMode())
	{
		Clear();
		mode.store(TMode::kBuy);
	}

	ScopeCriticalLock guard{ locker };

	a_count = std::min(a_count, MAX_NUM);
	auto it = list.find(a_item);
	if (it == list.end())
	{
		list.emplace(a_item, a_count);
		price.fetch_add(a_item.price * a_count);
		weight.fetch_add(a_item.weight * a_count);
	}
	else
	{

	}

//	if (list.try_emplace(a_item, a_count).second)
//	{
//#if 0
//		auto it = list.find(a_item);
//		if (it == list.end() || (a_count >= MAX_NUM)) return;
//		auto oldNum = it->second;
//		it->second = std::min(it->second + a_count, MAX_NUM);
//		a_count = it->second - oldNum;
//#else
//		return;
//#endif
//	}
//
//	if (a_count)
//	{
//		
//	}
}

void meBasket::Append(const meItemList& a_list, uint32_t a_count) noexcept
{
	if (a_count < 1)
		return;

	if (IsSellMode())
	{
		Clear();
		mode.store(TMode::kBuy);
	}

	ScopeCriticalLock guard{ locker };

	auto num = std::min(a_count, MAX_NUM);
	for (auto& item : a_list)
	{
		auto it = list.find(item);
		if (it == list.end())
		{
			list.emplace(item, a_count);
			price.fetch_add(item.price * a_count);
			weight.fetch_add(item.weight * a_count);
		}
		else
		{

		}

//
//		
//		if (list.try_emplace(item, num).second)
//		{
//#if 0
//			auto it = list.find(item);
//			if (it == list.end()) continue;
//			auto oldNum = it->second;
//			it->second = std::min(it->second + num, MAX_NUM);
//			num = it->second - oldNum;
//#else
//			continue;
//#endif
//		}
//		
//		if (num)
//		{
//			price.fetch_add(item.price * num);
//			weight.fetch_add(item.weight * num);
//		}
	}
}

void meBasket::AppendFromInventoryPlayer() noexcept
{
	if (IsBuyMode())
	{
		Clear();
		mode.store(TMode::kSell);
	}

	ScopeCriticalLock guard{ locker };

	auto player = RE::PlayerCharacter::GetSingleton();
	if (!player)
	{
		REX::ERROR("meBusket::AppendFromInventoryPlayer -- PlayerCharacter::GetSingleton() return nullptr");
		return;
	}

	if (!player->inventoryList)
	{
		REX::ERROR("meBusket::AppendFromInventoryPlayer -- PlayerCharacter haven't init inventoryList");
		return;
	}

	auto playerInvList = player->inventoryList;
	RE::BSAutoReadLock guardInv{ playerInvList->rwLock };

	for (auto& item : playerInvList->data)
	{
		if (!item.stackData || item.stackData->IsEquipped() || !item.stackData->count || !item.object)
			continue;

		auto pKeyword = (RE::BGSKeywordForm*)RE::RTDynamicCast((void*)item.object, 0, (void*)RE::RTTI::TESForm.address(),
			(void*)RE::RTTI::BGSKeywordForm.address(), 0);

		if (pKeyword)
		{
			for (uint32_t j = 0; j < pKeyword->numKeywords; j++)
			{
				// FeaturedItem [KYWD:001B3FAC], quest item
				if ((pKeyword->keywords[j]->formID == 0x1B3FAC))
					continue;

				// VendorItemKey [KYWD:00022CE3], this a key to buy or enter a house.
				if ((item.object->formType.get() == RE::ENUM_FORM_ID::kKEYM) && (pKeyword->keywords[j]->formID == 0x22CE3))
					continue;
			}
		}

		meItem appItem{};
		appItem.formId = item.object->formID;
		appItem.form = item.object;
		appItem.type = static_cast<meItemType>(std::to_underlying(item.object->formType.get()));

		auto name = (RE::TESFullName*)RE::RTDynamicCast((void*)item.object, 0, (void*)RE::RTTI::TESForm.address(),
			(void*)RE::RTTI::TESFullName.address(), 0);
		auto weight = (RE::TESWeightForm*)RE::RTDynamicCast((void*)item.object, 0, (void*)RE::RTTI::TESForm.address(),
			(void*)RE::RTTI::TESWeightForm.address(), 0);
		auto value = (RE::TESValueForm*)RE::RTDynamicCast((void*)item.object, 0, (void*)RE::RTTI::TESForm.address(),
			(void*)RE::RTTI::TESValueForm.address(), 0);

		if (name) appItem.name = name->fullName.QString();
		if (weight) appItem.weight = weight->weight;
		if (value) appItem.price = value->value;

		switch (appItem.type)
		{
		case meItemType::kArmorItem:
		{
			auto& data = reinterpret_cast<const RE::TESObjectARMO*>(item.object)->armorData;
			appItem.weight = data.weight;
			appItem.price = data.value;
			break;
		}
		case meItemType::kWeaponItem:
		{
			auto& data = reinterpret_cast<const RE::TESObjectWEAP*>(item.object)->weaponData;
			appItem.weight = data.weight;
			appItem.price = data.value;
			break;
		}
		}

		auto num = std::min(item.stackData->count, MAX_NUM);
		if (list.try_emplace(appItem, num).second)
			continue;

		if (num)
		{
			this->price.fetch_add(static_cast<uint32_t>((appItem.price * num) * 0.1f));		// 10% by price
			this->weight.fetch_add(appItem.weight * num);
		}
	}
}

void meBasket::Remove(size_t a_idx) noexcept
{
	ScopeCriticalLock guard{ locker };

	if (a_idx >= list.size())
		return;

	auto it = list.begin();
	std::advance(it, a_idx);

	auto money = it->first.price;
	if (IsSellMode())
		money = static_cast<uint32_t>(money * 0.1f);

	price.fetch_sub(money);
	weight.fetch_sub(it->first.weight);
	list.erase(it);

	if (!list.size())
	{
		mode.store(TMode::kBuy);
		price.store(0);
		weight.store(.0f);
	}
}

void meBasket::Clear() noexcept
{
	ScopeCriticalLock guard{ locker };

	list.clear();
	mode.store(TMode::kBuy);
	price.store(0);
	weight.store(.0f);
}

bool meBasket::RunOp(TOperations a_op) noexcept
{
	using namespace std::literals;

	ScopeCriticalLock guard{ locker };

	auto player = RE::PlayerCharacter::GetSingleton();
	if (!player)
	{
		REX::ERROR("meBusket::RunOp -- PlayerCharacter::GetSingleton() return nullptr");
		return false;
	}

	if (!player->inventoryList)
	{
		REX::ERROR("meBusket::RunOp -- PlayerCharacter haven't init inventoryList");
		return false;
	}

	auto playerInvList = player->inventoryList;
	RE::BSAutoWriteLock guardInv{ playerInvList->rwLock };

	if (a_op == TOperations::kTake)
	{
		for (auto& item : list)
		{
			//REX::INFO("Debug: take {} in count {}", item.first.name, item.second);
			/*playerInvList->AddItem2(
				const_cast<RE::TESBoundObject*>(reinterpret_cast<const RE::TESBoundObject*>(item.first.form)),
				item.second);*/
			if (Script::Invoker::GetSingleton()->InvokeFmt("player.additem %X %d 1"sv, item.first.form->formID, item.second))
				REX::ERROR("Failed additem {:X}"sv, item.first.form->formID);
		}

		if (list.size() > 0)
			takeFX.Play();
	}
	else if (a_op == TOperations::kBuy)
	{
		auto needMoney = this->price.load();

		RE::BGSInventoryItem* moneyPlayerInv = nullptr;
		uint32_t moneyPlayer = 0;
		for (auto& item : playerInvList->data)
			if (item.object->formID == 0xF)
			{
				moneyPlayer = item.GetCount();
				moneyPlayerInv = std::addressof(item);
				break;
			}

		if ((needMoney > moneyPlayer) || !moneyPlayerInv || !moneyPlayerInv->stackData)
			return false;

		moneyPlayerInv->stackData->count -= needMoney;

		for (auto& item : list)
		{
			/*playerInvList->AddItem2(
				const_cast<RE::TESBoundObject*>(reinterpret_cast<const RE::TESBoundObject*>(item.first.form)),
				item.second);*/
			if (Script::Invoker::GetSingleton()->InvokeFmt("player.additem %X %d 1"sv, item.first.form->formID, item.second))
				REX::ERROR("Failed additem {:X}"sv, item.first.form->formID);
		}

		if (list.size() > 0)
			buyFX.Play();
	}
	else if (a_op == TOperations::kSell)
	{
		auto shouldMoney = this->price.load();

		auto moneyForm = reinterpret_cast<RE::TESBoundObject*>(RE::TESForm::GetFormByNumericID(0xF));
		if (!moneyForm)
			return false;

		for (auto& item : list)
			playerInvList->RemoveItem1(
				const_cast<RE::TESBoundObject*>(reinterpret_cast<const RE::TESBoundObject*>(item.first.form)),
				item.second);
		
		playerInvList->AddItem2(moneyForm, shouldMoney);

		if (list.size() > 0)
			buyFX.Play();
	}
	else
		return false;

	Clear();

	return true;
}

bool meBasket::IsGoodState() const noexcept
{
	auto player = RE::PlayerCharacter::GetSingleton();
	if (!player || !player->inventoryList) return false;
	return true;
}

float meBasket::GetWeight() const noexcept
{
	return weight.load();
}

uint32_t meBasket::GetPrice() const noexcept
{
	return price.load();
}

std::array<float, 2> meBasket::GetPlayerWeight() const noexcept
{
	if (IsGoodState())
	{
		auto player = RE::PlayerCharacter::GetSingleton();
		auto actorValue = RE::ActorValue::GetSingleton();
		auto carryWeight = actorValue ? actorValue->carryWeight : nullptr;
		return { player->GetWeightInContainer(),
			carryWeight ? player->GetActorValue(*carryWeight) : .0f };
	}
	return { .0f, .0f };
}

uint32_t meBasket::GetPlayerCaps() const noexcept
{
	if (IsGoodState())
	{
		auto player = RE::PlayerCharacter::GetSingleton();
		auto playerInvList = player->inventoryList;
		RE::BSAutoReadLock guardInv{ playerInvList->rwLock };

		uint32_t moneyPlayer = 0;
		for (auto& item : playerInvList->data)
			if (item.object->formID == 0xF)
			{
				moneyPlayer = item.GetCount();
				break;
			}

		return moneyPlayer;
	}
	return 0;
}
