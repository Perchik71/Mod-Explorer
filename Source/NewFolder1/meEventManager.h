#pragma once

#include "meSpinLock.h"

class meEvent
{
	meEvent(const meEvent&) = delete;
	meEvent(meEvent&&) = delete;
	meEvent& operator=(const meEvent&) = delete;
	meEvent& operator=(meEvent&&) = delete;
public:
	meEvent() = default;
	virtual ~meEvent() = default;

	virtual void Execute() noexcept = 0;
};

class meEventManager :
	public REX::TSingleton<meEventManager>
{
	enum class State
	{
		kInit = 1 << 0,
		kShutdown = 1 << 1,
	};

	REX::TEnumSet<State> state{};
	std::queue<meEvent*> stack;
	meSpinLock lock{};

	meEventManager(const meEventManager&) = delete;
	meEventManager(meEventManager&&) = delete;
	meEventManager& operator=(const meEventManager&) = delete;
	meEventManager& operator=(meEventManager&&) = delete;
public:
	meEventManager() = default;
	virtual ~meEventManager() = default;

	static void InitSDM() noexcept;
	static void KillSDM() noexcept;

	virtual bool QInitialized() const noexcept;
	virtual bool QReleased() const noexcept;

	virtual meEvent* PopEvent() noexcept;
	virtual void PushEvent(meEvent* a_event) noexcept;
};

class meInactiveGameEvent :
	public meEvent
{
public:
	meInactiveGameEvent() = default;
	virtual ~meInactiveGameEvent() = default;

	void Execute() noexcept override;
};

class meActiveGameEvent :
	public meEvent
{
public:
	meActiveGameEvent() = default;
	virtual ~meActiveGameEvent() = default;

	void Execute() noexcept override;
};