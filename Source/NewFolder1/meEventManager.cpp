#include "meEventManager.h"
#include "meExplorer.h"
#include "meUtils.h"

#include <Windows.h>

void meEventManager::InitSDM() noexcept
{
	auto eventMgr = meEventManager::GetSingleton();
	if (eventMgr->QInitialized())
		return;

	std::thread([]() {
		auto eventMgr = meEventManager::GetSingleton();
		if (!eventMgr) return;

		while (!eventMgr->QReleased())
		{
			auto event = eventMgr->PopEvent();
			if (!event)
			{
				std::this_thread::yield();
				continue;
			}

			event->Execute();

			std::this_thread::sleep_for(10ms);
		}

		}).detach();

	eventMgr->state.set(true, State::kInit);
}

void meEventManager::KillSDM() noexcept
{
	auto eventMgr = meEventManager::GetSingleton();
	eventMgr->state.set(true, State::kShutdown);
}

bool meEventManager::QInitialized() const noexcept
{
	return state.any(State::kInit);
}

bool meEventManager::QReleased() const noexcept
{
	return state.any(State::kShutdown);
}

meEvent* meEventManager::PopEvent() noexcept
{
	meAutoSpinLock scopeLock(lock);

	if (stack.empty())
		return nullptr;

	auto event = stack.front();
	stack.pop();

	return event;
}

void meEventManager::PushEvent(meEvent* a_event) noexcept
{
	meAutoSpinLock scopeLock(lock);

	if (!a_event && (stack.size() >= 0x10000))
		return;

	stack.push(a_event);
}

void meInactiveGameEvent::Execute() noexcept
{
	auto explorer = meExplorer::GetSingleton();
	if (explorer) explorer->Close();
}

void meActiveGameEvent::Execute() noexcept
{
	// nope
}