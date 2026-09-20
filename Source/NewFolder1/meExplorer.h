#pragma once

#include "meEventManager.h"
#include "meStorageItems.h"

class meExplorerEventOpen :
	public meEvent
{
public:
	meExplorerEventOpen() = default;
	virtual ~meExplorerEventOpen() = default;

	void Execute() noexcept override;
};

class meExplorerEventClose :
	public meEvent
{
public:
	meExplorerEventClose() = default;
	virtual ~meExplorerEventClose() = default;

	void Execute() noexcept override;
};

class meExplorerEventTakeItemsFromBasket :
	public meEvent
{
public:
	meExplorerEventTakeItemsFromBasket() = default;
	virtual ~meExplorerEventTakeItemsFromBasket() = default;

	void Execute() noexcept override;
};

class meExplorerEventClearBasket :
	public meEvent
{
public:
	meExplorerEventClearBasket() = default;
	virtual ~meExplorerEventClearBasket() = default;

	void Execute() noexcept override;
};

class meExplorer :
	public REX::TSingleton<meExplorer>
{
	enum class State
	{
		kInit = 1 << 0,
		kShutdown = 1 << 1,
		kOpen = 1 << 2,
	};

	REX::TEnumSet<State> state{};
	std::vector<meItem> items{};
	meSpinLock lock{};

	meExplorer(const meExplorer&) = delete;
	meExplorer(meExplorer&&) = delete;
	meExplorer& operator=(const meExplorer&) = delete;
	meExplorer& operator=(meExplorer&&) = delete;
public:
	friend class meExplorerEventOpen;
	friend class meExplorerEventClose;

	meExplorer() = default;
	virtual ~meExplorer() = default;

	virtual void Open() noexcept;
	virtual void Close() noexcept;
	virtual bool IsOpen() const noexcept;

	virtual void GiveItemsToPlayer() const noexcept;
	virtual void ClearSelectedItems() const noexcept;

	virtual bool QInitialized() const noexcept;
	virtual bool QReleased() const noexcept;
};