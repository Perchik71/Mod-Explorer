#pragma once

#include "meDataStorage.h"

#include <atomic>

struct meBasketItemHasher
{
	[[nodiscard]] size_t operator()(const meItem& p) const noexcept
	{
		return std::hash<uint32_t>{}(p.formId) ^ (std::hash<uint32_t>{}(std::to_underlying(p.type)) << 1);
	}
};

struct meBasketItemEqualTo
{
	[[nodiscard]] bool operator()(const meItem& lhs, const meItem& rhs) const noexcept
	{
		return lhs.form == rhs.form;
	}
};

using meBasketItemList = std::unordered_map<const meItem, uint32_t, meBasketItemHasher, meBasketItemEqualTo>;
class meBasket :
	public REX::TSingleton<meBasket>
{
	enum class TMode : uint8_t
	{
		kBuy = 0,
		kSell
	};

	meBasketItemList list{};
	RE::BSReadWriteLock locker{};
	std::atomic<TMode> mode{ TMode::kBuy };
	std::atomic<float> weight{ 0.f };
	std::atomic<uint32_t> price{ 0 };

	meBasket(const meBasket&) = delete;
	meBasket(meBasket&&) = delete;
	meBasket& operator=(const meBasket&) = delete;
	meBasket& operator=(meBasket&&) = delete;
public:
	constexpr inline static auto MAX_NUM = static_cast<uint32_t>(std::numeric_limits<int16_t>::max());

	meBasket() = default;
	~meBasket() = default;

	enum class TOperations : uint8_t
	{
		kTake = 0,
		kBuy,
		kSell
	};

	inline uint32_t GetCount() const noexcept { return static_cast<uint32_t>(list.size()); }

	inline meBasketItemList::iterator begin() noexcept { return list.begin(); }
	inline meBasketItemList::iterator end() noexcept { return list.end(); }
	inline meBasketItemList::const_iterator cbegin() noexcept { return list.cbegin(); }
	inline meBasketItemList::const_iterator cend() noexcept { return list.cend(); }

	void Lock() const noexcept { const_cast<meBasket*>(this)->locker.lock_read(); }
	void Unlock() const noexcept { const_cast<meBasket*>(this)->locker.unlock_read(); }

	void Append(const meItem& a_item, uint32_t a_count = 1) noexcept;
	void Append(const meItemList& a_list, uint32_t a_count = 1) noexcept;
	void AppendFromInventoryPlayer() noexcept;
	void Remove(size_t a_idx) noexcept;
	void Clear() noexcept;

	bool RunOp(TOperations a_op) noexcept;

	[[nodiscard]] inline bool IsBuyMode() const noexcept { return mode.load() == TMode::kBuy; }
	[[nodiscard]] inline bool IsSellMode() const noexcept { return mode.load() == TMode::kSell; }
	[[nodiscard]] bool IsGoodState() const noexcept;

	[[nodiscard]] float GetWeight() const noexcept;
	[[nodiscard]] uint32_t GetPrice() const noexcept;

	[[nodiscard]] std::array<float, 2> GetPlayerWeight() const noexcept;
	[[nodiscard]] uint32_t GetPlayerCaps() const noexcept;
};