#include "meSpinLock.h"
#include <Windows.h>

void meSpinLock::Lock() noexcept
{
	auto id = static_cast<uint32_t>(GetCurrentThreadId());
	if (id != ownerId)
	{
		while (lock.test_and_set(std::memory_order_acquire))
			std::this_thread::yield();

		ownerId = id;
	}

	ref.fetch_add(1, std::memory_order_relaxed);
}

void meSpinLock::Unlock() noexcept
{
	if (ref.fetch_sub(1, std::memory_order_release) == 1)
		lock.clear(std::memory_order_release);
}

meAutoSpinLock::meAutoSpinLock(const meSpinLock& a_spinlock) noexcept :
	spinlock(const_cast<meSpinLock*>(&a_spinlock))
{
	spinlock->Lock();
}

meAutoSpinLock::meAutoSpinLock(const meSpinLock* a_spinlock) noexcept :
	spinlock(const_cast<meSpinLock*>(a_spinlock))
{
	if (spinlock)
		spinlock->Lock();
}

meAutoSpinLock::~meAutoSpinLock() noexcept
{
	if (spinlock)
		spinlock->Unlock();
}