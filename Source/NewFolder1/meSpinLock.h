#pragma once

class meSpinLock
{
	std::atomic_flag lock = ATOMIC_FLAG_INIT;
	std::atomic_int32_t ref{ 0 };
	uint32_t ownerId{ (uint32_t)-1 };
public:
	meSpinLock() noexcept = default;
	~meSpinLock() noexcept = default;

	void Lock() noexcept;
	void Unlock() noexcept;
};

class meAutoSpinLock
{
	meSpinLock* spinlock{ nullptr };

	meAutoSpinLock(const meAutoSpinLock&) = delete;
	meAutoSpinLock(meAutoSpinLock&&) = delete;
	meAutoSpinLock& operator=(const meAutoSpinLock&) = delete;
	meAutoSpinLock& operator=(meAutoSpinLock&&) = delete;
public:
	meAutoSpinLock(const meSpinLock& a_spinlock) noexcept;
	meAutoSpinLock(const meSpinLock* a_spinlock) noexcept;
	~meAutoSpinLock() noexcept;
};