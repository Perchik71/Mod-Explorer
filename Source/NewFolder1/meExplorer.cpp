#include "meExplorer.h"

#include <RE/S/SCRIPT_PARAMETER.h>
#include <RE/S/SCRIPT_FUNCTION.h>
#include <RE/P/PlayerCharacter.h>
#include <RE/U/UI.h>

static void meToggleGamePause() noexcept
{
	float returnValueDimmy;
	uint32_t offsetDimmy;
	// pause/resume game
	auto cmd = RE::SCRIPT_FUNCTION::GetConsoleFunctions();
	cmd[169].executeFunction(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, returnValueDimmy, offsetDimmy);
}

void meExplorerEventOpen::Execute() noexcept
{
	auto ui = RE::UI::GetSingleton();
	auto player = RE::PlayerCharacter::GetSingleton();
	auto explorer = meExplorer::GetSingleton();
	if (ui && player && !player->IsInCombat() && explorer && !explorer->IsOpen() && 
		(ui->GetMenuOpen("HUDMenu") || ui->GetMenuOpen("PowerArmorHUDMenu")) && 
		!ui->GetMenuOpen("PipboyMenu") && !ui->GetMenuOpen("CursorMenu"))
	{
		explorer->state.set(true, meExplorer::State::kOpen);
		meToggleGamePause();
	}
}

void meExplorerEventClose::Execute() noexcept
{
	auto explorer = meExplorer::GetSingleton();
	if (explorer && explorer->IsOpen())
	{
		explorer->state.set(false, meExplorer::State::kOpen);
		meToggleGamePause();
	}
}

void meExplorerEventTakeItemsFromBasket::Execute() noexcept
{
	//MessageBoxA(0, "event", "", 0);
}

void meExplorerEventClearBasket::Execute() noexcept
{
	//MessageBoxA(0, "event", "", 0);
}

void meExplorer::Open() noexcept
{
	if (IsOpen())
		return;

	meEventManager::GetSingleton()->PushEvent(new (std::nothrow) meExplorerEventOpen);
}

void meExplorer::Close() noexcept
{
	if (!IsOpen())
		return;

	meEventManager::GetSingleton()->PushEvent(new (std::nothrow) meExplorerEventClose);
}

bool meExplorer::IsOpen() const noexcept
{
	return state.any(State::kOpen);
}

void meExplorer::GiveItemsToPlayer() const noexcept
{

}

void meExplorer::ClearSelectedItems() const noexcept
{
}

bool meExplorer::QInitialized() const noexcept
{
	return state.any(State::kInit);
}

bool meExplorer::QReleased() const noexcept
{
	return state.any(State::kShutdown);
}