#include "meUtils.h"
#include "meError.h"

#include <detours/Detours.h>

#include <RE/C/COMPILER_NAME.h>
#include <RE/C/ConcreteFormFactory.h>
#include <RE/S/Script.h>
#include <RE/S/ScriptCompiler.h>

// Causes crashes if put above RE/S/Script.h
#include <RE/C/ConsoleLog.h>

std::string meUtils::GetRuntimePath() noexcept
{
    static char	appPath[4096] = { 0 };

    if (appPath[0])
        return appPath;

    meAssert(REX::W32::GetModuleFileNameA(REX::W32::GetModuleHandleA(nullptr), appPath, sizeof(appPath)));
    return appPath;
}

std::string meUtils::GetRuntimeDirectory() noexcept
{
    std::string runtimeDirectory;

    if (runtimeDirectory.empty())
    {
        std::string	runtimePath = GetRuntimePath();
        // truncate at last slash
        std::string::size_type	lastSlash = runtimePath.rfind('\\');
        if (lastSlash != std::string::npos)
            // if we don't find a slash something is VERY WRONG
            runtimeDirectory = runtimePath.substr(0, lastSlash + 1);
    }

    return runtimeDirectory;
}

bool meUtils::ExecuteCommand(std::string_view a_command, RE::TESObjectREFR* a_targetRef, bool a_silent) noexcept
{
	// Skip empty commands
	if (a_command.empty())
		return false;

	// Get Console Log Buffer before executing commands
	RE::ConsoleLog* log = RE::ConsoleLog::GetSingleton();
	RE::BSString buffer = log->buffer;

	// Create Compiler and Script Objects
	RE::ScriptCompiler compiler{};
	RE::ConcreteFormFactory<RE::Script>* scriptFactory = RE::ConcreteFormFactory<RE::Script>::GetFormFactory();
	RE::Script* script = scriptFactory->Create();

	// Set Script and Compile
	script->SetText(a_command);
	script->CompileAndRun(&compiler, RE::COMPILER_NAME::kSystemWindow, a_targetRef);

	// Warn on Failure
	if (!script->header.isCompiled)
	{
		REX::INFO("ExecuteCommand: Failed to compile command: {}"sv, a_command);
		return false;
	}

	// Restore Console Buffer
	if (a_silent)
		log->buffer = std::move(buffer);

	// Cleanup
	delete script;
	return true;
}

static uintptr_t DetourCheckRel32AndReturnDestAddress(uintptr_t a_target) noexcept
{
	auto target = reinterpret_cast<const uint8_t*>(a_target);
	if ((target[0] == 0xE8) || (target[0] == 0xE9))
	{
		auto rel32 = reinterpret_cast<const int32_t*>(a_target + 1);
		return a_target + *rel32 + 5;
	}
	else if ((target[0] == 0xFF) && ((target[1] == 0x15) || (target[1] == 0x25)))
	{
		auto rel32 = reinterpret_cast<const int32_t*>(a_target + 2);
		return *reinterpret_cast<uintptr_t*>(a_target + *rel32 + 6);
	}
	return 0;
}

[[nodiscard]] bool meUtils::Validate(uintptr_t a_target, const std::initializer_list<uint8_t>& a_validate) noexcept
{
	return !memcmp(reinterpret_cast<void*>(a_target), reinterpret_cast<const void*>(a_validate.begin()),
		a_validate.size());
}

[[nodiscard]] uintptr_t meUtils::DetourCall(uintptr_t a_target, uintptr_t a_function, uintptr_t* a_handle) noexcept
{
	auto destAddress = DetourCheckRel32AndReturnDestAddress(a_target);
	auto detourAddress = Detours::X64::DetourFunction(a_target, a_function, Detours::X64Option::USE_REL32_CALL);
	if (a_handle) *a_handle = detourAddress;
	return destAddress ? destAddress : detourAddress;
}

[[nodiscard]] uintptr_t meUtils::DetourJump(uintptr_t a_target, uintptr_t a_function, uintptr_t* a_handle) noexcept
{
	auto destAddress = DetourCheckRel32AndReturnDestAddress(a_target);
	auto detourAddress = Detours::X64::DetourFunction(a_target, a_function, Detours::X64Option::USE_REL32_JUMP);
	if (a_handle) *a_handle = detourAddress;
	return destAddress ? destAddress : detourAddress;
}

uintptr_t meUtils::DetourCall(const REL::Relocation<>& a_target, uintptr_t a_function, uintptr_t* a_handle) noexcept
{
	return DetourCall(a_target.address(), a_function, a_handle);
}

uintptr_t meUtils::DetourJump(const REL::Relocation<>& a_target, uintptr_t a_function, uintptr_t* a_handle) noexcept
{
	return DetourJump(a_target.address(), a_function, a_handle);
}

void meUtils::DetourRemove(uintptr_t a_handle) noexcept
{
	Detours::X64::DetourRemove(a_handle);
}