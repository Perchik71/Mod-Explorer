#pragma once

#include <RE/T/TESObjectREFR.h>

namespace meUtils
{
	[[nodiscard]] std::string GetRuntimePath() noexcept;
	[[nodiscard]] std::string GetRuntimeDirectory() noexcept;
	bool ExecuteCommand(std::string_view a_command, RE::TESObjectREFR* a_targetRef, bool a_silent) noexcept;

	[[nodiscard]] bool Validate(uintptr_t a_target, const std::initializer_list<uint8_t>& a_validate) noexcept;

	[[nodiscard]] uintptr_t DetourCall(uintptr_t a_target, uintptr_t a_function, uintptr_t* a_handle = nullptr) noexcept;
	[[nodiscard]] uintptr_t DetourJump(uintptr_t a_target, uintptr_t a_function, uintptr_t* a_handle = nullptr) noexcept;
	[[nodiscard]] uintptr_t DetourCall(const REL::Relocation<>& a_target, uintptr_t a_function, uintptr_t* a_handle = nullptr) noexcept;
	[[nodiscard]] uintptr_t DetourJump(const REL::Relocation<>& a_target, uintptr_t a_function, uintptr_t* a_handle = nullptr) noexcept;

	template<class T>
	inline static uintptr_t DetourClassJump(uintptr_t a_target, T a_function, uintptr_t* a_handle = nullptr) noexcept
	{
		return DetourJump(a_target, *(uintptr_t*)&a_function, a_handle);
	}

	template<class T>
	inline static uintptr_t DetourClassCall(uintptr_t a_target, T a_function, uintptr_t* a_handle = nullptr) noexcept
	{
		return DetourCall(a_target, *(uintptr_t*)&a_function, a_handle);
	}

	template<class T>
	inline static uintptr_t DetourClassJump(const REL::Relocation<>& a_target, T a_function, uintptr_t* a_handle = nullptr) noexcept
	{
		return DetourJump(a_target, *(uintptr_t*)&a_function, a_handle);
	}

	template<class T>
	inline static uintptr_t DetourClassCall(const REL::Relocation<>& a_target, T a_function, uintptr_t* a_handle = nullptr) noexcept
	{
		return DetourCall(a_target, *(uintptr_t*)&a_function, a_handle);
	}

	void DetourRemove(uintptr_t a_handle) noexcept;
};

using namespace std::literals;